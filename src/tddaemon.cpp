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

#include "tda/tdcredentialsdialog.h"

#include "usdot/usdotapi.h"

#include <QApplication>

static const QString EQUITY_MARKET( "EQUITY" );
static const QString OPTION_MARKET( "OPTION" );

///////////////////////////////////////////////////////////////////////////////////////////////////
TDAmeritradeDaemon::TDAmeritradeDaemon( TDAmeritrade *api, DeptOfTheTreasury *usdot, QObject *parent ) :
    _Mybase( parent ),
    api_( api ),
    usdot_( usdot ),
    init_( false ),
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
    connect( db_, &AppDatabase::instrumentsChanged, this, &_Myt::onInstrumentsChanged );
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
QString TDAmeritradeDaemon::name() const
{
    return tr( "T&DA API" );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::editCredentials()
{
    TDCredentialsDialog d( QApplication::activeWindow() );
    d.setConsumerId( api_->clientId() );
    d.setCallbackUrl( api_->redirectUrl().toString() );

    // prompt new credentials
    if ( QDialog::Accepted == d.exec() )
    {
        api_->setClientId( d.consumerId() );
        api_->setRedirectUrl( d.callbackUrl() );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::getAccounts()
{
    api_->getAccounts();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::getOptionChain( const QString& symbol )
{
    const QString MESSAGE( tr( "Fetching option chain information for %1..." ) );

    // fetch fundamental data
    if ( needFundamentals( symbol ) )
        api_->getFundamentalData( symbol );

    // fetch price history
    if ( needQuoteHistory( symbol ) )
        retrievePriceHistory( symbol, db_->currentDateTime() );

    // fetch chain
    api_->getOptionChain( symbol );

    emit statusMessageChanged( MESSAGE.arg( symbol ) );
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
    state_ = value;

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
        checkIdleStatus();
        break;
    case ACTIVE_BACKGROUND:
        emit statusMessageChanged( tr( "Processing watchlists..." ) );
        break;
    default:
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::dequeue()
{
    if ( Online != connectedState() )
        return;
    else if (( !isActive() ) && ( init_ ))
        return;

    const QDateTime now( db_->currentDateTime() );

    // check for date change... refresh data on date change
    if ( today_ != now.date() )
    {
        LOG_INFO << "entering startup due to new date " << qPrintable( today_.toString() );

        setCurrentState( STARTUP );
        today_ = now.date();

        // fetch TREAS_TIME years worth of historical data
        fetchTreas_ = now.date().addYears( -TREAS_YIELD_HIST );

        // fetch MARKET_HOURS_TIME days worth of market hours
        fetchMarketHours_ = now.date();
    }

    // ------------------------------------------------------------------------
    // Startup / Init
    // ------------------------------------------------------------------------

    if ( !processTreasYieldsState( now ) )
        return;

    if ( !processMarketHoursState( now ) )
        return;

    if ( !processAccountsState() )
        return;

    init_ = true;

    if ( !isActive() )
        return;

    // ------------------------------------------------------------------------
    // Active
    // ------------------------------------------------------------------------

    if ( !processActiveState( now ) )
        return;

    // no longer processing chains or equities
    if ( ACTIVE_BACKGROUND == currentState() )
    {
        quotesBackgroundProcess( false );
        optionChainBackgroundProcess( false );
    }

    // clear background processing flag
    setCurrentState( ACTIVE );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::queueEquityRequests( const QStringList& symbols, bool force )
{
    // prevent queue of back to back requests
    if (( fetchEquityStamp_.isValid() ) && ( fetchEquityStamp_.addSecs( REQUEST_TIMEOUT ) <= QDateTime::currentDateTime() ))
    {
        LOG_TRACE << "not fetching equity since within timeout period";
        return;
    }

    // check for markets closed
    if ( force )
        LOG_TRACE << "forcing queue";
    else if ( queueWhenClosed_ )
        LOG_TRACE << "queue when markets closed set";
    else if ( !db_->isMarketOpen( db_->currentDateTime(), EQUITY_MARKET ) )
    {
        LOG_TRACE << "markets are closed";
        return;
    }

    // retrieve list
    equityQueue_ = symbols;

    // active
    if ( equityQueue_.size() )
        emit quotesBackgroundProcess( true, equityQueue_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::retrieveOptionChain( const QString& symbol, const QDateTime& fromDate, int numExpiryDays ) const
{
    LOG_DEBUG << "request " << numExpiryDays << " days of option contracts for " << qPrintable( symbol );
    api_->getOptionChain( symbol, "SINGLE", "ALL", true, fromDate.date(), fromDate.addDays( numExpiryDays ).date() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::retrievePriceHistory( const QString& symbol, const QDateTime& toDate ) const
{
    QDate start;
    QDate end;

    db_->quoteHistoryDateRange( symbol, start, end );

    // no history
    //   -or-
    // not enough history
    if (( !start.isValid() ) || ( !end.isValid() ) || ( toDate.addYears( -QUOTE_HIST_CHECK ).date() < start ))
    {
        // yuck... get around stupid Qt cannot emit from const methods...
        emit const_cast<_Myt*>( this )->statusMessageChanged( tr( "Fetching historical prices for " ) + symbol + "..." );

        LOG_DEBUG << "request " << QUOTE_HIST << " year history for " << qPrintable( symbol );
        api_->getPriceHistory( symbol, QUOTE_HIST, "year", 1, "daily", QDateTime(), toDate );
    }

    // retrieve missing data
    else
    {
        // yuck... get around stupid Qt cannot emit from const methods...
        emit const_cast<_Myt*>( this )->statusMessageChanged( tr( "Updating historical prices for " ) + symbol + "..." );

        // how much history do we need
        int numMonths( 0 );

        do
        {
            end = end.addMonths( 1 );
            ++numMonths;

        } while ( end < toDate.date() );

        LOG_DEBUG << "request " << numMonths << " months daily history for " << qPrintable( symbol );
        api_->getPriceHistory( symbol, numMonths, "month", 1, "daily", QDateTime(), toDate );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::queueOptionChainRequests( const QStringList& symbols, const bool force )
{
    // prevent queue of back to back requests
    if (( fetchOptionChainStamp_.isValid() ) && ( fetchOptionChainStamp_.addSecs( REQUEST_TIMEOUT ) <= QDateTime::currentDateTime() ))
    {
        LOG_TRACE << "not fetching option chains since within timeout period";
        return;
    }

    // check for markets closed
    if ( force )
        LOG_TRACE << "forcing queue";
    else if ( queueWhenClosed_ )
        LOG_TRACE << "queue when markets closed set";
    else if ( !db_->isMarketOpen( db_->currentDateTime(), OPTION_MARKET ) )
    {
        LOG_TRACE << "markets are closed";
        return;
    }

    // retrieve list
    optionChainQueue_ = symbols;
    optionChainQueue_.removeDuplicates();

    // determine if fundamental data needed
    // determine if quote history needed
    foreach ( const QString& symbol, optionChainQueue_ )
    {
        if ( needFundamentals( symbol ) )
            fundamentalsQueue_.append( symbol );

        if ( needQuoteHistory( symbol ) )
            quoteHistoryQueue_.append( symbol );
    }

    fundamentalsQueue_.removeDuplicates();
    quoteHistoryQueue_.removeDuplicates();

    // active
    if ( optionChainQueue_.size() )
        emit optionChainBackgroundProcess( true, optionChainQueue_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::onAccountsChanged()
{
    if ( WAIT_ACCOUNTS != currentState() )
        return;

    LOG_DEBUG << "have accounts";
    setCurrentState( ACTIVE );

    // process next state manually when not initialized
    if ( !init_ )
        dequeue();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::onInstrumentsChanged()
{
    checkIdleStatus();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::onActiveChanged( bool newValue )
{
    if ( newValue )
    {
        setCurrentState( STARTUP );

        // queue
        queueEquityRequests( equityWatchlist() );
        queueOptionChainRequests( optionChainWatchlist() );
    }
    else
    {
        setCurrentState( INACTIVE );

        // clear queues
        equityQueue_.clear();
        fundamentalsQueue_.clear();
        optionChainQueue_.clear();
        quoteHistoryQueue_.clear();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::onConnectedStateChanged( TDAmeritrade::ConnectedState newState )
{
    emit connectedStateChanged( connectedStates_[newState] );

    // process next state manually when not initialized
    if ( !init_ )
        dequeue();
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

    // process next state manually when not initialized
    if ( !init_ )
        dequeue();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::onOptionChainChanged( const QString& symbol, const QList<QDate>& expiryDates )
{
    emit optionChainUpdated( symbol, expiryDates, (ACTIVE_BACKGROUND == currentState()) );

    checkIdleStatus();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::onQuotesChanged( const QStringList& symbols )
{
    emit quotesUpdated( symbols, (ACTIVE_BACKGROUND == currentState()) );

    checkIdleStatus();
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

    // process next state manually when not initialized
    if ( !init_ )
        dequeue();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool TDAmeritradeDaemon::needFundamentals( const QString& symbol ) const
{
    const QDateTime stamp( db_->lastFundamentalProcessed( symbol ) );

    // once per day
    if (( !stamp.isValid() ) || ( stamp.date() < db_->currentDateTime().date() ))
        return true;

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool TDAmeritradeDaemon::needQuoteHistory( const QString& symbol ) const
{
    const QDateTime stamp( db_->lastQuoteHistoryProcessed( symbol ) );

    // once per day
    if (( !stamp.isValid() ) || ( stamp.date() < db_->currentDateTime().date() ))
        return true;

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDaemon::checkIdleStatus()
{
    if ( ACTIVE != currentState() )
        return;

    // all queues empty and nothing pending
    if (( equityQueue_.empty() ) &&
        ( fundamentalsQueue_.empty() ) &&
        ( optionChainQueue_.empty() ) &&
        ( quoteHistoryQueue_.empty() ) &&
        ( !requestsPending() ))
    {
        emit statusMessageChanged( tr( "Ready." ) );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool TDAmeritradeDaemon::processTreasYieldsState( const QDateTime& now )
{
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

            return false;
        }

        setCurrentState( FETCH_MARKET_HOURS );
    }

    // wait for treasury yield curve
    if ( WAIT_TREAS_YIELDS == currentState() )
    {
        if (( fetchTreasStamp_.addSecs( REQUEST_TIMEOUT ) <= QDateTime::currentDateTime() ) || ( !requestsPending() ))
        {
            LOG_WARN << "timeout waiting for treasury yield curve data (or bad response)";
            setActive( false );

            emit statusMessageChanged( tr( "ERROR: Timeout waiting for treasury yield curve data." ) );
        }

        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool TDAmeritradeDaemon::processMarketHoursState( const QDateTime& now )
{
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

            return false;
        }

        setCurrentState( FETCH_ACCOUNTS );
    }

    // wait for market hours
    if ( WAIT_MARKET_HOURS == currentState() )
    {
        if (( fetchMarketHoursStamp_.addSecs( REQUEST_TIMEOUT ) <= QDateTime::currentDateTime() ) || ( !requestsPending() ))
        {
            bool okay( true );

            LOG_WARN << "timeout waiting for market hours (or bad response)";

            // as long as we have market hours for today and the past this error is okay... for
            // now... not sure about when we fetch market hours tomorrow...
            if ( fetchMarketHours_ <= now.date() )
                okay = false;
            else
            {
                const QStringList marketTypes( db_->marketTypes() );

                foreach ( const QString& marketType, marketTypes )
                    if ( !db_->marketHoursExist( now.date(), marketType ) )
                    {
                        okay = false;
                        break;
                    }
            }

            if ( okay )
            {
                LOG_INFO << "market hours exist for today";

                emit statusMessageChanged( tr( "WARNING: Timeout waiting for market hours" ) );

                // move to next state
                setCurrentState( FETCH_ACCOUNTS );
                return true;
            }

            // we do not have market hours for today and/or the past
            setActive( false );

            emit statusMessageChanged( tr( "ERROR: Timeout waiting for market hours." ) );
        }

        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool TDAmeritradeDaemon::processAccountsState()
{
    // fetch accounts
    if ( FETCH_ACCOUNTS == currentState() )
    {
        LOG_DEBUG << "feching accounts";
        api_->getAccounts();

        fetchAccountsStamp_ = QDateTime::currentDateTime();
        setCurrentState( WAIT_ACCOUNTS );

        return false;
    }

    // wait for accounts
    if ( WAIT_ACCOUNTS == currentState() )
    {
        if (( fetchAccountsStamp_.addSecs( REQUEST_TIMEOUT ) <= QDateTime::currentDateTime() ) || ( !requestsPending() ))
        {
            LOG_WARN << "timeout waiting for accounts (or bad response)";
            setActive( false );

            emit statusMessageChanged( tr( "ERROR: Timeout waiting for account and balance information." ) );
        }

        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool TDAmeritradeDaemon::processActiveState( const QDateTime& now )
{
    // request equity quotes
    if ( equityQueue_.size() )
    {
        QStringList symbols;

        while (( equityQueue_.size() ) && ( symbols.size() < EQUITY_DEQUEUE_SIZE ))
        {
            symbols.push_back( equityQueue_.front() );
            equityQueue_.pop_front();
        }

        setCurrentState( ACTIVE_BACKGROUND );

        LOG_DEBUG << "requesting " << symbols.size() << " equity quotes";
        api_->getQuotes( symbols );

        emit statusMessageChanged( tr( "Fetching quotes..." ) );
        return false;
    }
    else if ( requestsPending() )
    {
        LOG_TRACE << "waiting for equity";
        return false;
    }

    // request fundamental data
    if ( fundamentalsQueue_.size() )
    {
        const QString MESSAGE( tr( "Fetching fundamental data for %1..." ) );

        const QString symbol( fundamentalsQueue_.front() );
        fundamentalsQueue_.pop_front();

        setCurrentState( ACTIVE_BACKGROUND );

        // fetch fundamental data
        api_->getFundamentalData( symbol );

        emit statusMessageChanged( MESSAGE.arg( symbol ) );
        return false;
    }
    else if ( requestsPending() )
    {
        LOG_TRACE << "waiting for fundamental data";
        return false;
    }

    // request quote history
    if ( quoteHistoryQueue_.size() )
    {
        const QString MESSAGE( tr( "Fetching price history for %1..." ) );

        const QString symbol( quoteHistoryQueue_.front() );
        quoteHistoryQueue_.pop_front();

        setCurrentState( ACTIVE_BACKGROUND );

        // fetch price history
        retrievePriceHistory( symbol, now );

        emit statusMessageChanged( MESSAGE.arg( symbol ) );
        return false;
    }
    else if ( requestsPending() )
    {
        LOG_TRACE << "waiting for quote history";
        return false;
    }

    // request option chain
    if ( optionChainQueue_.size() )
    {
        const QString symbol( optionChainQueue_.front() );
        optionChainQueue_.pop_front();

        setCurrentState( ACTIVE_BACKGROUND );

        // fetch option chain
        retrieveOptionChain( symbol, now, optionChainExpiryEndDate() );

        return false;
    }
    else if ( requestsPending() )
    {
        LOG_TRACE << "waiting for option chain";
        return false;
    }

    return true;
}
