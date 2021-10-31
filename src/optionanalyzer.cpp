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

#include "db/optiontradingitemmodel.h"

#include <QApplication>

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionAnalyzer::OptionAnalyzer( model_type *model, QObject *parent ) :
    _Mybase( parent ),
    active_( false ),
    analysis_( model ),
    workers_( 0 ),
    maxWorkers_( 2 * std::thread::hardware_concurrency() )
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
bool OptionAnalyzer::isActive() const
{
    return (( active_ ) || ( numThreadsComplete_ < numThreads_ ));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionAnalyzer::onOptionChainBackgroundProcess( bool active, const QStringList& symbols )
{
    // nothing to do
    if ( active_ == active )
        return;

    active_ = active;

    if ( active_ )
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

    emit activeChanged( active_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionAnalyzer::onOptionChainUpdated( const QString& symbol, const QList<QDate>& expiryDates, bool background )
{
    if ( !background )
        return;
    else if ( !symbols_.contains( symbol ) )
        return;

    // throttle cpu to max worker limit
    while (( THROTTLE ) && ( maxWorkers_ <= workers_ ))
    {
        const QList<OptionAnalyzerThread*> threads( findChildren<OptionAnalyzerThread*>() );

        int finished( 0 );

        foreach ( OptionAnalyzerThread *t, threads )
            if ( t->isFinished() )
                ++finished;

        if ( (workers_ - finished) < maxWorkers_ )
            break;

        LOG_TRACE << "throttle workers...";
        QThread::msleep( THROTTLE_YIELD_TIME );
    }

    symbols_.removeOne( symbol );

    // create thread(s) for analysis
    LOG_DEBUG << "processing " << qPrintable( symbol ) << " " << expiryDates.size() << " chains...";

    foreach ( const QDate& d, expiryDates )
    {
        // create worker thread
        OptionAnalyzerThread *worker( new OptionAnalyzerThread( symbol, d, analysis_, this ) );

        if ( customFilter_.length() )
            worker->setCustomFilter( customFilter_ );

        connect( worker, &QThread::finished, this, &_Myt::onWorkerFinished );

        // start work!
        worker->start();

        ++numThreads_;
        ++workers_;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionAnalyzer::onWorkerFinished()
{
    static const double MIN_PROGRESS( 2.0 );

    sender()->deleteLater();
    --workers_;

    ++numThreadsComplete_;

    // analysis complete
    if ( !isActive() )
    {
        const QString message( tr( "Options analysis complete! %1 options scanned in %2 minutes." ) );

        LOG_INFO << "analysis complete!";
        QApplication::beep();

        // reset custom filter
        resetCustomFilter();

        // record stop time
        stop_ = QDateTime::currentDateTime();

        const double totalTime( start_.secsTo( stop_ ) / 60.0 );

        LOG_INFO << "scanned " << symbolsTotal_ << " symbols with " << numThreadsComplete_ << " total expirations " << totalTime << " in minutes (" << THROTTLE << ")";
        LOG_DEBUG << "average time per expiration " << (double) numThreadsComplete_ / start_.secsTo( stop_ ) << " sec (" << THROTTLE << ")";

        emit statusMessageChanged( message.arg( symbolsTotal_ ).arg( totalTime, 0, 'f', 2 ) );
        emit complete();
    }

    // analysis in progress
    else
    {
        double currentProgress( 100.0 );
        currentProgress = qMin( currentProgress, (100.0 * numThreadsComplete_) / (double) numThreads_ );
        currentProgress = qMin( currentProgress, (100.0 * (symbolsTotal_ - symbols_.size())) / (double) symbolsTotal_ );

        // update message
        if ( MIN_PROGRESS <= (currentProgress - progress_) )
        {
            const QString message( tr( "Options analysis in progress... %1% complete..." ) );

            progress_ = currentProgress;

            emit statusMessageChanged( message.arg( progress_, 0, 'f', 1 ) );
        }
    }
}
