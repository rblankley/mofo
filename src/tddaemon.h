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

    /// Check if credentials can be edited (i.e. dialog implemented).
    /**
     * @return  @c true if credentials can be edited, @c false otherwise
     */
    virtual bool canEditCredentials() const override {return true;}

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

    /// Edit credentials.
    virtual void editCredentials() override;

    /// Retrieve accounts.
    virtual void getAccounts() override;

    /// Retrieve symbol candle data.
    /**
     * Example: For a 2 day / 1 min chart, the values would be:
     *
     * period: 2
     * periodType: day
     * frequency: 1
     * frequencyType: min
     *
     * Valid periods by periodType (defaults marked with an asterisk):
     *
     * day: 1, 2, 3, 4, 5, 10*
     * month: 1*, 2, 3, 6
     * year: 1*, 2, 3, 5, 10, 15, 20
     * ytd: 1*
     *
     * Valid frequencies by frequencyType (defaults marked with an asterisk):
     *
     * minute: 1*, 5, 10, 15, 30
     * daily: 1*
     * weekly: 1*
     * monthly: 1*
     *
     * Valid frequencyTypes by periodType (defaults marked with an asterisk):
     *
     * day: minute*
     * month: daily, weekly*
     * year: daily, weekly, monthly*
     * ytd: daily, weekly*
     *
     * @param[in] symbol  symbol
     * @param[in] period  history period
     * @param[in] periodType  period type (day, month, year, or ytd)
     * @param[in] freq  frequency
     * @param[in] freqType  frequency type (minute, daily, weekly, or monthly)
     */
    virtual void getCandles( const QString& symbol, int period, const QString& periodType, int freq, const QString& freqType ) override;

    /// Retrieve option chain.
    /**
     * @param[in] symbol  symbol
     */
    virtual void getOptionChain( const QString& symbol ) override;

    /// Retrieve quote.
    /**
     * @param[in] symbol  symbol
     */
    virtual void getQuote( const QString& symbol ) override;

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

    /// TDA daemon state.
    enum state
    {
        INACTIVE,                                   ///< Not active.
        FETCH_TREAS_YIELDS,                         ///< Fetching treasury yield rates.
        WAIT_TREAS_YIELDS,                          ///< Waiting on treasury yields.
        FETCH_MARKET_HOURS,                         ///< Fetching market hours.
        WAIT_MARKET_HOURS,                          ///< Waiting on market hours.
        FETCH_ACCOUNTS,                             ///< Fetching accounts.
        WAIT_ACCOUNTS,                              ///< Waiting on accounts.
        ACTIVE,                                     ///< Active (online and idle).
        ACTIVE_BACKGROUND,                          ///< Fetching background data.

        // meta states
        STARTUP = FETCH_TREAS_YIELDS,               ///< Startup state.
    };

    QStringList equityQueue_;                       ///< Queue of equity requests.
    QStringList fundamentalsQueue_;                 ///< Queue of fundamental data requests.
    QStringList optionChainQueue_;                  ///< Queue of option chain requests.
    QStringList quoteHistoryQueue_;                 ///< Queue of quote history requests.

    QStringList equityBackgroundPending_;           ///< List of pending background equity requests.
    QStringList optionChainBackgroundPending_;      ///< List of pending background option chain requests.

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
    /**
     * @param[in] symbols  symbols to queue
     * @param[in] force  @c true to force queueing, @c false otherwise
     */
    virtual void queueEquityRequests( const QStringList& symbols, const bool force = false ) override;

    /// Queue option chain requests.
    /**
     * @param[in] symbols  symbols to queue
     * @param[in] force  @c true to force queueing, @c false otherwise
     */
    virtual void queueOptionChainRequests( const QStringList& symbols, const bool force = false ) override;

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

    /// Slot for instruments changed.
    void onInstrumentsChanged();

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

    static constexpr int REQUEST_TIMEOUT = 120;             // 120s

    static constexpr int DEQUEUE_TIME = 520;                // 520ms (do not go below 500ms, TDA throttles to 120 requests/min)

    static constexpr int EQUITY_DEQUEUE_SIZE = 8;

    static constexpr int MARKET_HOURS_HIST = 7;             // days
    static constexpr int QUOTE_HIST = 5;                    // years
    static constexpr int TREAS_YIELD_HIST = 5;              // years

    static constexpr int QUOTE_HIST_CHECK = 3;              // years

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

    /// Check if fundamentals are needed for symbol.
    bool needFundamentals( const QString& symbol ) const;

    /// Check if quote history is needed for symbol.
    bool needQuoteHistory( const QString& symbol ) const;

    /// Check for idle (ready) status.
    void checkIdleStatus();

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
