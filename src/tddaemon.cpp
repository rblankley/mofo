/**
 * @file tddaemon.cpp
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

#include "common.h"
#include "tddaemon.h"

#include "db/appdb.h"

#include "usdot/usdotapi.h"

static const QString EQUITY_MARKET( "EQUITY" );
static const QString OPTION_MARKET( "OPTION" );

///////////////////////////////////////////////////////////////////////////////////////////////////
TDAmeritradeDaemon::TDAmeritradeDaemon( TDAmeritrade *api, DeptOfTheTreasury *usdot, QObject *parent ) :
    _Mybase( parent ),
    api_( api ),
    usdot_( usdot ),
    state_( INACTIVE ),
    apiPending_( 0 ),
    usdotPending_( 0 )
{
    // map connected states
    connectedStates_[TDAmeritrade::Offline] = Offline;
    connectedStates_[TDAmeritrade::Authorizing] = Authorizing;
    connectedStates_[TDAmeritrade::Online] = Online;

    connect( api_, &TDAmeritrade::connectedStateChanged, this, &_Myt::onConnectedStateChanged );

    connect( api_, &TDAmeritrade::requestsPendingChanged, this, &_Myt::onRequestsPendingChanged );
    connect( usdot_, &TDAmeritrade::requestsPendingChanged, this, &_Myt::onRequestsPendingChanged );

    connect( db_, &AppDatabase::accountsChanged, this, &_Myt::onAccountsChanged );
    connect( db_, &AppDatabase::marketHoursChanged, this, &_Myt::onMarketHoursChanged );
    connect( db_, &AppDatabase::optionChainChanged, this, &_Myt::onOptionChainChanged );
    connect( db_, &AppDatabase::quotesChanged, this, &_Myt::onQuotesChanged );
    connect( db_, &AppDatabase::treasuryYieldCurveRatesChanged, this, &_Myt::onTreasuryYieldCurveRatesChanged );

    connect( this, &_Mybase::activeChanged, this, &_Myt::onActiveChanged );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TDAmeritradeDaemon::~TDAmeritradeDaemon()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
AbstractDaemon::ConnectedState TDAmeritradeDaemon::connectedState() const
{
    const TDAmeritrade::ConnectedState state( api_->connectedState() );

    return connectedStates_[state];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::getAccounts()
{
    api_->getAccounts();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::getOptionChain( const QString& symbol )
{
    api_->getOptionChain( symbol );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool TDAmeritradeDaemon::waitForConnected( int timeout ) const
{
    return api_->waitForConnected( timeout );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::authorize()
{
    api_->authorize();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::setCurrentState( state value )
{
    // nothing to do
    if ( currentState() == value )
        return;

    LOG_DEBUG << "new state " << value;

    // set message
    switch ( value )
    {
    case INACTIVE:
        emit statusMessageChanged( tr( "Offline." ) );
        break;
    case FETCH_TREAS_YIELDS:
    case WAIT_TREAS_YIELDS:
        emit statusMessageChanged( tr( "Fetching treasury yields..." ) );
        break;
    case FETCH_MARKET_HOURS:
    case WAIT_MARKET_HOURS:
        emit statusMessageChanged( tr( "Fetching market hours..." ) );
        break;
    case FETCH_ACCOUNTS:
    case WAIT_ACCOUNTS:
        emit statusMessageChanged( tr( "Fetching account and balance information..." ) );
        break;
    case ACTIVE:
        emit statusMessageChanged( tr( "Ready." ) );
        break;
    case ACTIVE_BACKGROUND:
        emit statusMessageChanged( tr( "Processing watchlists..." ) );
        break;
    default:
        break;
    }

    state_ = value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::dequeue()
{
    if ( Online != connectedState() )
        return;
    else if ( INACTIVE == currentState() )
        return;

    const QDateTime now( db_->currentDateTime() );

    // check for date change... refresh data on date change
    if ( today_ != now.date() )
    {
        setCurrentState( STARTUP );
        today_ = now.date();

        LOG_INFO << "entering startup due to new date " << qPrintable( today_.toString() );
    }

    // ------------------------------------------------------------------------
    // Startup / Init
    // ------------------------------------------------------------------------

    // fetch treasury yield curve
    if ( FETCH_TREAS_YIELDS == currentState() )
    {
        QDate start;
        QDate end;

        db_->treasuryYieldCurveDateRange( start, end );

        while ( fetchTreas_ <= now.date() )
        {
            if (( start.isValid() ) && ( end.isValid() ))
            {
                // force fetch this month and last month
                if ( now.date().addMonths( -1 ) <= fetchTreas_ )
                    ;
                // fetch missing months
                else if (( start <= fetchTreas_ ) && ( fetchTreas_ <= end ))
                {
                    // proceed to next month
                    fetchTreas_ = fetchTreas_.addMonths( 1 );
                    continue;
                }
            }

            LOG_DEBUG << "feching treasury yield curve rates for " << qPrintable( fetchTreas_.toString() );
            usdot_->getDailyTreasuryYieldCurveRates( fetchTreas_.year(), fetchTreas_.month() );

            fetchTreasStamp_ = QDateTime::currentDateTime();
            setCurrentState( WAIT_TREAS_YIELDS );

            return;
        }

        setCurrentState( FETCH_MARKET_HOURS );
    }

    // wait for treasury yield curve
    if ( WAIT_TREAS_YIELDS == currentState() )
    {
        if ( fetchTreasStamp_.addSecs( REQUEST_TIMEOUT ) <= QDateTime::currentDateTime() )
        {
            LOG_WARN << "timeout waiting for treasury yield curve data";
            setActive( false );

            emit statusMessageChanged( tr( "ERROR: Timeout waiting for treasury yield curve data." ) );
        }

        return;
    }

    // fetch market hours
    if ( FETCH_MARKET_HOURS == currentState() )
    {
        const QStringList marketTypes( db_->marketTypes() );

        while ( fetchMarketHours_ <= now.date().addDays( MARKET_HOURS_HIST ) )
        {
            bool fetch( false );

            // check we have hours for every market type
            foreach ( const QString& marketType, marketTypes )
                if ( !db_->marketHoursExist( fetchMarketHours_, marketType ) )
                {
                    fetch = true;
                    break;
                }

            if ( !fetch )
            {
                fetchMarketHours_ = fetchMarketHours_.addDays( 1 );
                continue;
            }

            LOG_DEBUG << "feching market hours for " << qPrintable( fetchMarketHours_.toString() );
            api_->getMarketHours( fetchMarketHours_, marketTypes );

            fetchMarketHoursStamp_ = QDateTime::currentDateTime();
            setCurrentState( WAIT_MARKET_HOURS );

            return;
        }

        setCurrentState( FETCH_ACCOUNTS );
    }

    // wait for market hours
    if ( WAIT_MARKET_HOURS == currentState() )
    {
        if ( fetchMarketHoursStamp_.addSecs( REQUEST_TIMEOUT ) <= QDateTime::currentDateTime() )
        {
            LOG_WARN << "timeout waiting for market hours";
            setActive( false );

            emit statusMessageChanged( tr( "ERROR: Timeout waiting for market hours." ) );
        }

        return;
    }

    // fetch accounts
    if ( FETCH_ACCOUNTS == currentState() )
    {
        LOG_DEBUG << "feching accounts";
        api_->getAccounts();

        fetchAccountsStamp_ = QDateTime::currentDateTime();
        setCurrentState( WAIT_ACCOUNTS );

        return;
    }

    // wait for accounts
    if ( WAIT_ACCOUNTS == currentState() )
    {
        if ( fetchAccountsStamp_.addSecs( REQUEST_TIMEOUT ) <= QDateTime::currentDateTime() )
        {
            LOG_WARN << "timeout waiting for accounts";
            setActive( false );

            emit statusMessageChanged( tr( "ERROR: Timeout waiting for account and balance information." ) );
        }

        return;
    }

    // ------------------------------------------------------------------------
    // Active
    // ------------------------------------------------------------------------

    // request equity quotes
    if ( equityQueue_.size() )
    {
        QStringList symbols;

        while (( equityQueue_.size() ) && ( symbols.size() < EQUITY_DEQUEUE_SIZE ))
        {
            symbols.push_back( equityQueue_.front() );
            equityQueue_.pop_front();
        }

        LOG_DEBUG << "requesting " << symbols.size() << " equity quotes";
        api_->getQuotes( symbols );

        setCurrentState( ACTIVE_BACKGROUND );
        return;
    }
    else if ( requestsPending() )
    {
        LOG_TRACE << "waiting for equity";
        return;
    }

    // request quote history
    if ( quoteHistoryQueue_.size() )
    {
        const QString symbol( quoteHistoryQueue_.front() );
        quoteHistoryQueue_.pop_front();

        QDate start;
        QDate end;

        db_->quoteHistoryDateRange( symbol, start, end );

        // no history
        //   -or-
        // not enough history
        if (( !start.isValid() ) || ( !end.isValid() ) || ( now.addYears( -QUOTE_HIST ).date() < start ))
        {
            LOG_DEBUG << "request " << QUOTE_HIST << " year history for " << qPrintable( symbol );
            api_->getPriceHistory( symbol, QUOTE_HIST, "year", 1, "daily", QDateTime(), now );
        }

        // retrieve missing data
        else
        {
            // how much history do we need
            int numMonths( 0 );

            do
            {
                end = end.addMonths( 1 );
                ++numMonths;

            } while ( end < now.date() );

            LOG_DEBUG << "request " << numMonths << " months daily history for " << qPrintable( symbol );
            api_->getPriceHistory( symbol, numMonths, "month", 1, "daily", QDateTime(), now );
        }

        setCurrentState( ACTIVE_BACKGROUND );
        return;
    }
    else if ( requestsPending() )
    {
        LOG_TRACE << "waiting for quote history";
        return;
    }

    // request option chain
    if ( optionChainQueue_.size() )
    {
        const QString symbol( optionChainQueue_.front() );
        optionChainQueue_.pop_front();

        const int numExpiryDays( optionChainExpiryEndDate() );

        LOG_DEBUG << "request " << numExpiryDays << " days of option contracts for " << qPrintable( symbol );
        api_->getOptionChain( symbol, "SINGLE", "ALL", true, now.date(), now.addDays( numExpiryDays ).date() );

        setCurrentState( ACTIVE_BACKGROUND );
        return;
    }
    else if ( requestsPending() )
    {
        LOG_TRACE << "waiting for option chain";
        return;
    }

    // clear background processing flag
    setCurrentState( ACTIVE );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::queueEquityRequests()
{
    // prevent queue of back to back requests
    if (( fetchEquityStamp_.isValid() ) && ( fetchEquityStamp_.addSecs( REQUEST_TIMEOUT ) <= QDateTime::currentDateTime() ))
    {
        LOG_TRACE << "not fetching equity since within timeout period";
        return;
    }

    // check for markets closed
    if ( queueWhenClosed_ )
        LOG_TRACE << "queue when markets closed set";
    else if ( !db_->isMarketOpen( db_->currentDateTime(), EQUITY_MARKET ) )
    {
        LOG_TRACE << "markets are closed";
        return;
    }

    // retrieve list
    equityQueue_ = equityWatchlist();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::queueOptionChainRequests()
{
    // prevent queue of back to back requests
    if (( fetchOptionChainStamp_.isValid() ) && ( fetchOptionChainStamp_.addSecs( REQUEST_TIMEOUT ) <= QDateTime::currentDateTime() ))
    {
        LOG_TRACE << "not fetching option chains since within timeout period";
        return;
    }

    // check for markets closed
    if ( queueWhenClosed_ )
        LOG_TRACE << "queue when markets closed set";
    else if ( !db_->isMarketOpen( db_->currentDateTime(), OPTION_MARKET ) )
    {
        LOG_TRACE << "markets are closed";
        return;
    }

    // retrieve list
    optionChainQueue_ = optionChainWatchlist();

    // determine if quote history needed (fetch once per day)
    foreach ( const QString& symbol, optionChainQueue_ )
    {
        const QDateTime stamp( db_->lastQuoteHistoryProcessed( symbol ) );

        if (( !stamp.isValid() ) || ( stamp.date() < db_->currentDateTime().date() ))
            quoteHistoryQueue_.append( symbol );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::onAccountsChanged()
{
    if ( WAIT_ACCOUNTS != currentState() )
        return;

    LOG_DEBUG << "have accounts";
    setCurrentState( ACTIVE );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::onActiveChanged( bool newValue )
{
    if ( newValue )
    {
        const QDateTime now( db_->currentDateTime() );

        setCurrentState( STARTUP );
        today_ = now.date();

        // fetch TREAS_TIME years worth of historical data
        fetchTreas_ = now.date().addYears( -TREAS_YIELD_HIST );

        // fetch MARKET_HOURS_TIME days worth of market hours
        fetchMarketHours_ = now.date();

        // queue
        queueEquityRequests();
        queueOptionChainRequests();
    }
    else
    {
        setCurrentState( INACTIVE );

        // clear queues
        equityQueue_.clear();
        optionChainQueue_.clear();
        quoteHistoryQueue_.clear();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::onConnectedStateChanged( TDAmeritrade::ConnectedState newState )
{
    emit connectedStateChanged( connectedStates_[newState] );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::onMarketHoursChanged()
{
    if ( WAIT_MARKET_HOURS != currentState() )
        return;

    LOG_DEBUG << "have market hours";

    // fetch next market hours
    fetchMarketHours_ = fetchMarketHours_.addDays( 1 );
    setCurrentState( FETCH_MARKET_HOURS );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::onOptionChainChanged( const QString& symbol, const QList<QDate>& expiryDates )
{
    emit optionChainUpdated( symbol, expiryDates, (ACTIVE_BACKGROUND == currentState()) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::onQuotesChanged( const QStringList& symbols )
{
    emit quotesUpdated( symbols, (ACTIVE_BACKGROUND == currentState()) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::onRequestsPendingChanged( int pending )
{
    if ( api_ == sender() )
        apiPending_ = pending;
    else if ( usdot_ == sender() )
        usdotPending_ = pending;

    emit requestsPendingChanged( requestsPending() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::onTreasuryYieldCurveRatesChanged()
{
    if ( WAIT_TREAS_YIELDS != currentState() )
        return;

    LOG_DEBUG << "have treas yields";

    // fetch next set of treasury yield curve rates
    fetchTreas_ = fetchTreas_.addMonths( 1 );
    setCurrentState( FETCH_TREAS_YIELDS );
}
