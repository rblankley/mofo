/**
 * @file tddaemon.h
 * TD Ameritrade API Daemon.
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

#ifndef TDDAEMON_H
#define TDDAEMON_H

#include "abstractdaemon.h"

#include "tda/tdapi.h"

#include <QMap>

class DeptOfTheTreasury;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// TD Ameritrade API Daemon.
class TDAmeritradeDaemon : public AbstractDaemon
{
    Q_OBJECT

    using _Myt = TDAmeritradeDaemon;
    using _Mybase = AbstractDaemon;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] api  api
     * @param[in] usdot  dept of the treasury api
     * @param[in,out] parent  parent object
     */
    TDAmeritradeDaemon( TDAmeritrade *api, DeptOfTheTreasury *usdot, QObject *parent = nullptr );

    /// Destructor.
    ~TDAmeritradeDaemon();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve connected state.
    /**
     * @return  connected state
     */
    virtual ConnectedState connectedState() const override;

    /// Retrieve daemon name.
    /**
     * @return  name
     */
    virtual QString name() const override;

    /// Retrieve number of pending requests.
    /**
     * @return  number of pending requests
     */
    virtual int requestsPending() const {return apiPending_ + usdotPending_;}

    // ========================================================================
    // Methods
    // ========================================================================

    /// Retrieve accounts.
    virtual void getAccounts() override;

    /// Retrieve option chain.
    /**
     * @param[in] symbol  symbol
     */
    virtual void getOptionChain( const QString& symbol ) override;

    /// Wait for connected.
    /**
     * @param[in] timeout  time to wait (ms)
     * @return  @c true if connected, @c false otherwise
     */
    virtual bool waitForConnected( int timeout = 240 * 1000 ) const override;

public slots:

    // ========================================================================
    // Methods
    // ========================================================================

    /// Daemon API Authorization.
    virtual void authorize() override;

protected:

    enum state
    {
        INACTIVE,
        FETCH_TREAS_YIELDS,
        WAIT_TREAS_YIELDS,
        FETCH_MARKET_HOURS,
        WAIT_MARKET_HOURS,
        FETCH_ACCOUNTS,
        WAIT_ACCOUNTS,
        ACTIVE,
        ACTIVE_BACKGROUND,

        // meta states
        STARTUP = FETCH_TREAS_YIELDS,
    };

    QStringList equityQueue_;                       ///< Queue of equity requests.
    QStringList optionChainQueue_;                  ///< Queue of option chain requests.
    QStringList quoteHistoryQueue_;                 ///< Queue of quote history requests.

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve current state.
    /**
     * @return  state
     */
    virtual state currentState() const {return state_;}

    /// Retrieve dequeue time.
    /**
     * @return  dequeue time (ms)
     */
    virtual int dequeueTime() const override {return DEQUEUE_TIME;}

    /// Set current state.
    /**
     * @param[in] value  state
     */
    virtual void setCurrentState( state value );

    // ========================================================================
    // Methods
    // ========================================================================

    /// Dequeue
    virtual void dequeue() override;

    /// Queue equity requests.
    virtual void queueEquityRequests() override;

    /// Queue option chain requests.
    virtual void queueOptionChainRequests() override;

    /// Fetch option chain.
    /**
     * @param[in] symbol  symbol to fetch
     * @param[in] fromDate  date to fetch from
     * @param[in] numExpiryDays  how many days to fetch
     */
    virtual void retrieveOptionChain( const QString& symbol, const QDateTime& fromDate, int numExpiryDays ) const;

    /// Fetch price history.
    /**
     * @param[in] symbol  symbol to fetch
     * @param[in] toDate  date to fetch to
     */
    virtual void retrievePriceHistory( const QString& symbol, const QDateTime& toDate ) const;

private slots:

    /// Slot for accounts changed.
    void onAccountsChanged();

    /// Slot for when active changes.
    void onActiveChanged( bool newValue );

    /// Slot for when connected state changes.
    void onConnectedStateChanged( TDAmeritrade::ConnectedState newState );

    /// Slot for market hours changed.
    void onMarketHoursChanged();

    /// Slot for when option chains have changed.
    void onOptionChainChanged( const QString& symbol, const QList<QDate>& expiryDates );

    /// Slot for when quotes have changed.
    void onQuotesChanged( const QStringList& symbols );

    /// Slot for requests pending changed.
    void onRequestsPendingChanged( int pending );

    /// Slot for treasury yield curve rates changed.
    void onTreasuryYieldCurveRatesChanged();

private:

    enum {REQUEST_TIMEOUT = 120}; // 120s

    enum {DEQUEUE_TIME = 600}; // 600ms

    enum {EQUITY_DEQUEUE_SIZE = 8};

    enum {MARKET_HOURS_HIST = 7}; // days
    enum {QUOTE_HIST = 5}; // years
    enum {TREAS_YIELD_HIST = 5}; // years

    enum {QUOTE_HIST_CHECK = 3}; // years

    using ConnectedStateMap = QMap<TDAmeritrade::ConnectedState, ConnectedState>;

    TDAmeritrade *api_;
    DeptOfTheTreasury *usdot_;

    bool init_;

    ConnectedStateMap connectedStates_;

    state state_;

    int apiPending_;
    int usdotPending_;

    QDate today_;

    QDate fetchTreas_;
    QDateTime fetchTreasStamp_;

    QDate fetchMarketHours_;
    QDateTime fetchMarketHoursStamp_;

    QDateTime fetchAccountsStamp_;
    QDateTime fetchEquityStamp_;
    QDateTime fetchOptionChainStamp_;

    /// Process treasury yields state.
    bool processTreasYieldsState( const QDateTime& now );

    /// Process market hours state.
    bool processMarketHoursState( const QDateTime& now );

    /// Process accounts state.
    bool processAccountsState();

    /// Process active state.
    bool processActiveState( const QDateTime& now );

    // not implemented
    TDAmeritradeDaemon( const _Myt& ) = delete;

    // not implemented
    TDAmeritradeDaemon( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // TDDAEMON_H
