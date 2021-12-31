/**
 * @file abstractdaemon.cpp
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

#include "db/appdb.h"

#include <QTimer>

// uncomment to queue requests when market closed
//#define QUEUE_WHEN_CLOSED

static const QString EQUITY_REFRESH_RATE( "equityRefreshRate" );
static const QString EQUITY_WATCH_LISTS( "equityWatchLists" );

static const QString OPTION_CHAIN_EXPIRY_END_DATE( "optionChainExpiryEndDate" );
static const QString OPTION_CHAIN_REFRESH_RATE( "optionChainRefreshRate" );
static const QString OPTION_CHAIN_WATCH_LISTS( "optionChainWatchLists" );

QMutex AbstractDaemon::instanceMutex_;
AbstractDaemon *AbstractDaemon::instance_( nullptr );

///////////////////////////////////////////////////////////////////////////////////////////////////
AbstractDaemon::AbstractDaemon( QObject *parent ) :
    _Mybase( parent ),
    db_( AppDatabase::instance() ),
    queueWhenClosed_( false ),
    paused_( false )
{
#if defined( QUEUE_WHEN_CLOSED )
    queueWhenClosed_ = true;
#endif

    // dequeue timer
    dequeue_ = new QTimer( this );
    dequeue_->setSingleShot( false );

    connect( dequeue_, &QTimer::timeout, this, &_Myt::onTimeout );

    // equity timer
    equity_ = new QTimer( this );
    equity_->setSingleShot( false );

    connect( equity_, &QTimer::timeout, this, &_Myt::onTimeout );

    // option chain timer
    optionChain_ = new QTimer( this );
    optionChain_->setSingleShot( false );

    connect( optionChain_, &QTimer::timeout, this, &_Myt::onTimeout );

    // configs
    onConfigurationChanged();

    connect( db_, &AppDatabase::configurationChanged, this, &_Myt::onConfigurationChanged );

    // set instance
    QMutexLocker guard( &instanceMutex_ );
    instance_ = this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
AbstractDaemon::~AbstractDaemon()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AbstractDaemon::canEditCredentials() const
{
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
AbstractDaemon::ConnectedState AbstractDaemon::connectedState() const
{
    return Online;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AbstractDaemon::isActive() const
{
    return dequeue_->isActive();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString AbstractDaemon::name() const
{
    return tr( "Market &Daemon" );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AbstractDaemon::editCredentials()
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AbstractDaemon::getAccounts()
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AbstractDaemon::getCandles( const QString& symbol, int period, const QString& periodType, int freq, const QString& freqType )
{
    Q_UNUSED( symbol )
    Q_UNUSED( period )
    Q_UNUSED( periodType )
    Q_UNUSED( freq )
    Q_UNUSED( freqType )
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AbstractDaemon::getOptionChain( const QString& symbol )
{
    Q_UNUSED( symbol )
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AbstractDaemon::waitForConnected( int timeout ) const
{
    Q_UNUSED( timeout )

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
AbstractDaemon *AbstractDaemon::instance()
{
    QMutexLocker guard( &instanceMutex_ );
    return instance_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AbstractDaemon::setActive( bool value )
{
    // nothing to do
    if ( isActive() == value )
        return;

    // start timers
    if ( value )
    {
        if ( Online == connectedState() )
        {
            if ( 0 < equity_->interval() )
                equity_->start();

            if ( 0 < optionChain_->interval() )
                optionChain_->start();

            dequeue_->start( dequeueTime() );

            // notify
            emit activeChanged( true );

            // make sure not paused
            setPaused( false );

            LOG_INFO << "=== DAEMON RUNNING ===";
        }
    }

    // stop timers
    else
    {
        equity_->stop();
        optionChain_->stop();

        dequeue_->stop();

        // notify
        emit activeChanged( false );

        // make sure not paused
        setPaused( false );

        LOG_INFO << "=== DAEMON STOPPED ===";
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AbstractDaemon::setPaused( bool value )
{
    // nothing to do
    if ( paused_ == value )
        return;

    // notify
    emit pausedChanged( paused_ = value );

    LOG_INFO << "=== DAEMON " << (paused_ ? "" : "UN") << "PAUSED ===";
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AbstractDaemon::authorize()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AbstractDaemon::scan( const QString& watchlists )
{
    if (( isActive() ) && ( !isPaused() ))
    {
        const QStringList symbols( watchlistSymbols( watchlists ) );

        // force scan
        if ( symbols.size() )
        {
            LOG_DEBUG << "force scan of " << symbols.size() << " symbols";
            queueOptionChainRequests( symbols, true );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QStringList AbstractDaemon::equityWatchlist() const
{
    return watchlistSymbols( configs_[EQUITY_WATCH_LISTS].toString() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int AbstractDaemon::optionChainExpiryEndDate() const
{
    return configs_[OPTION_CHAIN_EXPIRY_END_DATE].toString().toInt();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QStringList AbstractDaemon::optionChainWatchlist() const
{
    return watchlistSymbols( configs_[OPTION_CHAIN_WATCH_LISTS].toString() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AbstractDaemon::onConfigurationChanged()
{
    static const int MIN_TO_MS = 60 * 1000;

    // read config
    configs_ = db_->configs();

    // update timer intervals
    updateTimerInterval( equity_, configs_[EQUITY_REFRESH_RATE].toString().toInt() * MIN_TO_MS );
    updateTimerInterval( optionChain_, configs_[OPTION_CHAIN_REFRESH_RATE].toString().toInt() * MIN_TO_MS );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AbstractDaemon::onTimeout()
{
    // dequeue
    if ( dequeue_ == sender() )
    {
        // nothing to do
        if ( paused_ )
            return;
        else if ( Online != connectedState() )
            return;

        dequeue();
    }

    // equity
    else if ( equity_ == sender() )
        queueEquityRequests( equityWatchlist() );

    // option chain
    else if ( optionChain_ == sender() )
        queueOptionChainRequests( optionChainWatchlist() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AbstractDaemon::updateTimerInterval( QTimer *t, int interval ) const
{
    t->setInterval( interval );

    if (( t->isActive() ) && ( !interval ))
        t->stop();
    else if (( !t->isActive() ) && ( isActive() ))
        t->start();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QStringList AbstractDaemon::watchlistSymbols( const QString& lists ) const
{
    QStringList result;

    foreach ( const QString& list, lists.split( "," ) )
        result.append( db_->watchlist( list.trimmed() ) );

    result.removeDuplicates();
    result.sort();

    return result;
}
