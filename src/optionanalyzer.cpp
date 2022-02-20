/**
 * @file optionanalyzer.cpp
 *
 * @copyright Copyright (C) 2021 Randy Blankley. All rights reserved.
 *
 * @section LICENSE
 *
 * This file is part of mofo.
 *
 * Money4Options is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If
 * not, see <http://www.gnu.org/licenses/>.
 */

#include "abstractdaemon.h"
#include "common.h"
#include "optionanalyzer.h"
#include "optionanalyzerthread.h"

#include "db/appdb.h"
#include "db/optiontradingitemmodel.h"

#include <QApplication>
#include <QThread>

#ifdef Q_OS_WINDOWS
#include <Windows.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionAnalyzer::OptionAnalyzer( model_type *model, QObject *parent ) :
    _Mybase( parent ),
    active_( false ),
    analysis_( model ),
    halt_( false ),
    throttle_( false ),
#ifdef Q_OS_WINDOWS
    prevIdleTime_( 0 ),
    prevKernelTime_( 0 ),
    prevUserTime_( 0 ),
#endif
    workers_( 0 ),
    maxWorkers_( 4 * QThread::idealThreadCount() )
{
    // connect signals/slots
    connect( AbstractDaemon::instance(), &AbstractDaemon::optionChainBackgroundProcess, this, &_Myt::onOptionChainBackgroundProcess );
    connect( AbstractDaemon::instance(), &AbstractDaemon::optionChainUpdated, this, &_Myt::onOptionChainUpdated );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionAnalyzer::~OptionAnalyzer()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString OptionAnalyzer::filter() const
{
    if ( customFilter_.isEmpty() )
        return AppDatabase::instance()->optionAnalysisFilter();

    return customFilter_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool OptionAnalyzer::isActive() const
{
    return (( active_ ) || ( numThreadsComplete_ < numThreads_ ) || ( symbols_.length() ));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionAnalyzer::halt()
{
    // set halt flag
    halt_ = true;

    // halt each thread
    const QList<OptionAnalyzerThread*> workers( findChildren<OptionAnalyzerThread*>() );

    foreach ( OptionAnalyzerThread *worker, workers )
        worker->halt();

    // wait for analysis threads to complete
    while (( active_ ) || ( numThreadsComplete_ < numThreads_ ))
    {
        static const QEventLoop::ProcessEventsFlags flags( QEventLoop::AllEvents | QEventLoop::WaitForMoreEvents );

        QApplication::processEvents( flags, WAIT_TIME );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionAnalyzer::onOptionChainBackgroundProcess( bool active, const QStringList& symbols )
{
    const bool prevActive( isActive() );

    // new state
    active_ = active;

    // moving from inactive -> active
    if (( active_ ) && ( !prevActive ))
    {
        // remove all previous rows
        analysis_->removeAllRows();

        // save off total number of symbols to analyze
        symbols_ = symbols;
        symbolsTotal_ = symbols_.size();

        // reset progress
        numThreads_ = numThreadsComplete_ = 0;
        progress_ = 0.0;

        // record start time
        start_ = QDateTime::currentDateTime();
    }

    // moving from active -> active
    // additional symbols probably added to list
    else if (( active_ ) && ( prevActive ))
    {
        symbolsTotal_ -= symbols_.size();

        // save off total number of symbols to analyze
        symbols_ = symbols;
        symbolsTotal_ += symbols_.size();
    }

    // state changed!
    if ( active_ != prevActive )
        emit activeChanged( active_ );

    // refresh status
    if (( active_ ) || ( prevActive ))
        updateStatus( true );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionAnalyzer::onOptionChainUpdated( const QString& symbol, const QList<QDate>& expiryDates, bool background )
{
    if ( halt_ )
        return;
    else if ( !background )
        return;
    else if ( !symbols_.contains( symbol ) )
        return;

    // throttle cpu
    if (( THROTTLE ) && ( needToThrottle() ))
    {
        LOG_TRACE << "throttle workers...";
        AbstractDaemon::instance()->setPaused( (throttle_ = true) );
    }

    symbols_.removeOne( symbol );

    // create thread(s) for analysis
    LOG_INFO << "processing " << qPrintable( symbol ) << " " << expiryDates.size() << " chains...";
    LOG_DEBUG << symbols_.size() << " symbols remaining...";

    // force refresh status when no option data
    if ( expiryDates.isEmpty() )
        updateStatus( false );
    else
    {
        // create worker thread
        OptionAnalyzerThread *worker( new OptionAnalyzerThread( symbol, expiryDates, analysis_, this ) );
        worker->setFilter( filter() );

        connect( worker, &QThread::finished, this, &_Myt::onWorkerFinished );

        // start work!
        worker->start();

        ++numThreads_;
        ++workers_;
    }

    LOG_DEBUG << "workers started";
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionAnalyzer::onWorkerFinished()
{
    sender()->deleteLater();
    --workers_;

    ++numThreadsComplete_;

    // unthrottle cpu
    if (( THROTTLE ) && ( throttle_ ))
        if (( workers_ < QThread::idealThreadCount() ) || ( !needToThrottle() ))
        {
            LOG_TRACE << "restore workers...";
            AbstractDaemon::instance()->setPaused( (throttle_ = false) );
        }

    // refresh status
    updateStatus( false );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionAnalyzer::updateStatus( bool force )
{
    static const double MIN_PROGRESS( 2.0 );

    // analysis complete
    if ( !isActive() )
    {
        const QString message( tr( "Options analysis complete %1 using filter '%2'. %3 symbols scanned in %4 minutes." ) );

        LOG_INFO << "analysis complete!";
        QApplication::beep();

        // retrieve filter name
        QString f( filter() );

        if ( f.isEmpty() )
            f = tr( "NONE" );

        // reset custom filter
        resetCustomFilter();

        // record stop time
        stop_ = QDateTime::currentDateTime();

        const double totalTime( start_.secsTo( stop_ ) / 60.0 );

        LOG_INFO << "scanned " << symbolsTotal_ << " symbols with " << numThreadsComplete_ << " total expirations in " << totalTime << " minutes (" << THROTTLE << ")";
        LOG_DEBUG << "average time per expiration " << (double) numThreadsComplete_ / start_.secsTo( stop_ ) << " sec (" << THROTTLE << ")";

        emit statusMessageChanged( message.arg( stop_.toString() ).arg( f ).arg( symbolsTotal_ ).arg( totalTime, 0, 'f', 2 ) );
        emit complete();
    }

    // analysis in progress
    else
    {
        double currentProgress( 100.0 );
        currentProgress = qMin( currentProgress, (100.0 * numThreadsComplete_) / (double) numThreads_ );
        currentProgress = qMin( currentProgress, (100.0 * (symbolsTotal_ - symbols_.size())) / (double) symbolsTotal_ );

        // update message
        if (( force ) || ( MIN_PROGRESS <= (currentProgress - progress_) ))
        {
            const QString message( tr( "Options analysis in progress... %1% complete..." ) );

            progress_ = currentProgress;

            emit statusMessageChanged( message.arg( progress_, 0, 'f', 1 ) );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool OptionAnalyzer::needToThrottle() const
{
    if ( workers_ < QThread::idealThreadCount() )
        return false;

#ifdef Q_OS_WINDOWS
    if ( THROTTLE_CPU_THRESHOLD < cpuUsage() )
        return true;
#endif

    return ( maxWorkers_ <= workers_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double OptionAnalyzer::cpuUsage() const
{
    double result( 0.0 );

#ifdef Q_OS_WINDOWS
    FILETIME idleTime;
    FILETIME kernelTime;
    FILETIME userTime;

    // lookup system times
    if ( GetSystemTimes( &idleTime, &kernelTime, &userTime ) )
    {
        ULARGE_INTEGER idle;
        idle.HighPart = idleTime.dwHighDateTime;
        idle.LowPart = idleTime.dwLowDateTime;

        ULARGE_INTEGER kernel;
        kernel.HighPart = kernelTime.dwHighDateTime;
        kernel.LowPart = kernelTime.dwLowDateTime;

        ULARGE_INTEGER user;
        user.HighPart = userTime.dwHighDateTime;
        user.LowPart = userTime.dwLowDateTime;

        // check we have data to compare against
        if ( 0 < prevIdleTime_ )
        {
            const quint64 idleTime( idle.QuadPart - prevIdleTime_ );
            const quint64 sysTime( kernel.QuadPart - prevKernelTime_ + user.QuadPart - prevUserTime_ );

            // compute total cpu usage
            result = sysTime;
            result /= sysTime + idleTime;

            LOG_TRACE << "cpu usage " << result;
        }

        // store values for next time
        prevIdleTime_ = idle.QuadPart;
        prevKernelTime_ = kernel.QuadPart;
        prevUserTime_ = user.QuadPart;
    }
#endif

    return result;
}
