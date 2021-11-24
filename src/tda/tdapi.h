/**
 * @file tdapi.h
 * TD Ameritrade API implementation.
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

#ifndef TDAPI_H
#define TDAPI_H

#include "tdoauthapi.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

/// TD Ameritrade API implementation.
/**
 * @note
 * See TDA API documentation at:
 * https://developer.tdameritrade.com/
 */
class TDAmeritrade : public TDOpenAuthInterface
{
    Q_OBJECT

    using _Myt = TDAmeritrade;
    using _Mybase = TDOpenAuthInterface;

signals:

    /// Signal for accounts received.
    /**
     * @param[in] a  data objects
     */
    void accountsReceived( const QJsonArray& a );

    /// Signal for instrument received.
    /**
     * @param[in] obj  data
     */
    void instrumentReceived( const QJsonObject& obj );

    /// Signal for market hours received.
    /**
     * @param[in] obj  data
     */
    void marketHoursReceived( const QJsonObject& obj );

    /// Signal for option chain received.
    /**
     * @param[in] obj  data
     */
    void optionChainReceived( const QJsonObject& obj );

    /// Signal for price history received.
    /**
     * @param[in] obj  data
     */
    void priceHistoryReceived( const QJsonObject& obj );

    /// Signal for quotes received.
    /**
     * @param[in] obj  data
     */
    void quotesReceived( const QJsonObject& obj );

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in,out] parent  parent object
     */
    TDAmeritrade( QObject *parent = nullptr );

    /// Destructor.
    ~TDAmeritrade();

    // ========================================================================
    // Methods
    // ========================================================================

    /// Retrieve account.
    /**
     * @param[in] id  account id
     */
    virtual void getAccount( const QString& id );

    /// Retrieve accounts.
    virtual void getAccounts();

    /// Retrieve fundamental data.
    /**
     * @param[in] symbol  symbol
     */
    virtual void getFundamentalData( const QString& symbol );

    /// Retrieve instrument.
    /**
     * @param[in] cusip  cusip
     */
    virtual void getInstrument( const QString& cusip );

    /// Retrieve market hours.
    /**
     * @param[in] date  date to get hours for
     * @param[in] markets  markets to get hours for EQUITY, OPTION, FUTURE, BOND, or FOREX
     */
    virtual void getMarketHours( const QDate& date, const QStringList& markets );

    /// Retrieve market hours for single market.
    /**
     * @param[in] date  date to get hours for
     * @param[in] market  market to get hours for EQUITY, OPTION, FUTURE, BOND, or FOREX
     */
    virtual void getMarketHoursSingle( const QDate& date, const QString& market );

    /// Retrieve option chain.
    /**
     * @param[in] symbol  symbol
     * @param[in] strategy  strategy type SINGLE, ANALYTICAL (allows use of the volatility, underlyingPrice, interestRate, and daysToExpiration params to calculate theoretical values), COVERED, VERTICAL, CALENDAR, STRANGLE, STRADDLE, BUTTERFLY, CONDOR, DIAGONAL, COLLAR, or ROLL
     * @param[in] contractType  contract type CALL, PUT, or ALL
     * @param[in] includeQuotes  @c true to include quotes, @c false otherwise
     * @param[in] fromDate  from expiration date
     * @param[in] toDate  to expiration date
     */
    virtual void getOptionChain( const QString& symbol, const QString& strategy = "SINGLE", const QString& contractType = "ALL", bool includeQuotes = true, const QDate& fromDate = QDate(), const QDate& toDate = QDate() );

    /// Retrieve price history.
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
     * @param[in] fromDate  start date
     * @param[in] toDate  end date
     */
    virtual void getPriceHistory( const QString& symbol, int period, const QString& periodType, int freq, const QString& freqType, const QDateTime& fromDate = QDateTime(), const QDateTime& toDate = QDateTime() );

    /// Retrieve quote.
    /**
     * @param[in] symbol  symbol
     */
    virtual void getQuote( const QString& symbol );

    /// Retrieve quotes.
    /**
     * @param[in] symbols  list of symbols
     */
    virtual void getQuotes( const QStringList& symbols );

    // ========================================================================
    // Methods
    // ========================================================================

#ifdef QT_DEBUG
    /// Simulate accounts.
    /**
     * @param[in] doc  document
     */
    virtual void simulateAccounts( const QJsonDocument& doc );

    /// Simulate market hours.
    /**
     * @param[in] doc  document
     */
    virtual void simulateMarketHours( const QJsonDocument& doc );

    /// Simulate option chain.
    /**
     * @param[in] doc  document
     */
    virtual void simulateOptionChain( const QJsonDocument& doc );

    /// Simulate price history.
    /**
     * @param[in] doc  document
     * @param[in] period  history period
     * @param[in] periodType  period type (day, month, year, or ytd)
     * @param[in] freq  frequency
     * @param[in] freqType  frequency type (minute, daily, weekly, or monthly)
     * @param[in] fromDate  start date
     * @param[in] toDate  end date
     */
    virtual void simulatePriceHistory( const QJsonDocument& doc, int period, const QString& periodType, int freq, const QString& freqType, const QDateTime& fromDate = QDateTime(), const QDateTime& toDate = QDateTime() );

    /// Simulate quotes.
    /**
     * @param[in] doc  document
     */
    virtual void simulateQuotes( const QJsonDocument& doc );
#endif

private slots:

    /// Slot to process document.
    void onProcessDocumentJson( const QUuid& uuid, const QByteArray& request, const QString& requestType, int status, const QJsonDocument& response );

private:

    static constexpr int REQUEST_TIMEOUT = 30 * 1000;       // 30s
    static constexpr int REQUEST_RETRIES = 3;

    enum Endpoint
    {
        GET_ACCOUNT,
        GET_ACCOUNTS,
        GET_INSTRUMENT,
        GET_INSTRUMENTS,
        GET_MARKET_HOURS,
        GET_MARKET_HOURS_SINGLE,
        GET_OPTION_CHAIN,
        GET_PRICE_HISTORY,
        GET_QUOTE,
        GET_QUOTES,
    };

    using EndpointMap = QMap<Endpoint, QString>;

    EndpointMap endpointNames_;
    EndpointMap endpoints_;

    using PendingRequestsMap = QMap<QUuid, Endpoint>;

    PendingRequestsMap pendingRequests_;

    struct PriceHistoryRequest
    {
        int period;
        QString periodType;
        int freq;
        QString freqType;
        QDateTime fromDate;
        QDateTime toDate;
    };

    using PriceHistoryRequestMap = QMap<QUuid, PriceHistoryRequest>;

    PriceHistoryRequestMap priceHistoryRequests_;

    /// Load endpoints.
    void loadEndpoints();

    /// Parse accounts.
    void parseAccountsDoc( const QJsonDocument& doc );

    /// Parse instruments.
    void parseInstrumentsDoc( const QJsonDocument& doc );

    /// Parse market hours.
    void parseMarketHoursDoc( const QJsonDocument& doc );

    /// Parse option chain.
    void parseOptionChainDoc( const QJsonDocument& doc );

    /// Parse price history.
    void parsePriceHistoryDoc( const PriceHistoryRequest& request, const QJsonDocument& doc );

    /// Parse quotes.
    void parseQuotesDoc( const QJsonDocument& doc );

    // not implemented
    TDAmeritrade( const _Myt& ) = delete;

    // not implemented
    TDAmeritrade( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // TDAPI_H
