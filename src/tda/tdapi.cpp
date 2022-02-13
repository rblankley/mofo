/**
 * @file tdapi.cpp
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
#include "stringsjson.h"
#include "tdapi.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QUrl>
#include <QUrlQuery>

static const QString INI_FILE( SYS_CONF_DIR "endpoints.config" );

///////////////////////////////////////////////////////////////////////////////////////////////////
TDAmeritrade::TDAmeritrade( QObject *parent ) :
    _Mybase( parent )
{
    endpointNames_[GET_ACCOUNT] = "getAccount";
    endpointNames_[GET_ACCOUNTS] = "getAccounts";
    endpointNames_[GET_INSTRUMENT] = "getInstrument";
    endpointNames_[GET_INSTRUMENTS] = "getInstruments";
    endpointNames_[GET_MARKET_HOURS] = "getMarketHours";
    endpointNames_[GET_MARKET_HOURS_SINGLE] = "getMarketHoursSingle";
    endpointNames_[GET_OPTION_CHAIN] = "getOptionChain";
    endpointNames_[GET_PRICE_HISTORY] = "getPriceHistory";
    endpointNames_[GET_QUOTE] = "getQuote";
    endpointNames_[GET_QUOTES] = "getQuotes";

    loadEndpoints();

    connect( this, &_Myt::processDocumentJson, this, &_Myt::onProcessDocumentJson, Qt::DirectConnection );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TDAmeritrade::~TDAmeritrade()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritrade::getAccount( const QString& id )
{
    if ( id.isEmpty() )
        return;

    QString s( endpoints_[GET_ACCOUNT] );
    s.replace( "{accountId}", id );

    const QUuid uuid( QUuid::createUuid() );

    QMutexLocker guard( &m_ );

    pendingRequests_[uuid] = GET_ACCOUNT;
    send( uuid, s, REQUEST_TIMEOUT, REQUEST_RETRIES );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritrade::getAccounts()
{
    const QUrl url( endpoints_[GET_ACCOUNTS] );

    const QUuid uuid( QUuid::createUuid() );

    QMutexLocker guard( &m_ );

    pendingRequests_[uuid] = GET_ACCOUNTS;
    send( uuid, url, REQUEST_TIMEOUT, REQUEST_RETRIES );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritrade::getFundamentalData( const QString& symbol )
{
    if ( symbol.isEmpty() )
        return;

    QUrlQuery urlQuery;
    urlQuery.addQueryItem( "symbol", symbol );
    urlQuery.addQueryItem( "projection", "fundamental" );

    QUrl url( endpoints_[GET_INSTRUMENTS] );
    url.setQuery( urlQuery );

    const QUuid uuid( QUuid::createUuid() );

    QMutexLocker guard( &m_ );

    pendingRequests_[uuid] = GET_INSTRUMENTS;
    send( uuid, url, REQUEST_TIMEOUT, REQUEST_RETRIES );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritrade::getInstrument( const QString& cusip )
{
    if ( cusip.isEmpty() )
        return;

    QString s( endpoints_[GET_INSTRUMENT] );
    s.replace( "{cusip}", cusip );

    const QUuid uuid( QUuid::createUuid() );

    QMutexLocker guard( &m_ );

    pendingRequests_[uuid] = GET_INSTRUMENT;
    send( uuid, s, REQUEST_TIMEOUT, REQUEST_RETRIES );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritrade::getMarketHours( const QDate& date, const QStringList& markets )
{
    if (( !date.isValid() ) || ( markets.isEmpty() ))
        return;

    QString s;

    foreach ( const QString& market, markets )
    {
        if ( s.length() )
            s.append( "," );

        s.append( market );
    }

    QUrlQuery urlQuery;
    urlQuery.addQueryItem( "markets", s );
    urlQuery.addQueryItem( "date", date.toString( Qt::ISODate ) );

    QUrl url( endpoints_[GET_MARKET_HOURS] );
    url.setQuery( urlQuery );

    const QUuid uuid( QUuid::createUuid() );

    QMutexLocker guard( &m_ );

    pendingRequests_[uuid] = GET_MARKET_HOURS;
    send( uuid, url, REQUEST_TIMEOUT, REQUEST_RETRIES );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritrade::getMarketHoursSingle( const QDate& date, const QString& market )
{
    if (( !date.isValid() ) || ( market.isEmpty() ))
        return;

    QString s( endpoints_[GET_MARKET_HOURS_SINGLE] );
    s.replace( "{market}", market );

    QUrlQuery urlQuery;
    urlQuery.addQueryItem( "date", date.toString( Qt::ISODate ) );

    QUrl url( s );
    url.setQuery( urlQuery );

    const QUuid uuid( QUuid::createUuid() );

    QMutexLocker guard( &m_ );

    pendingRequests_[uuid] = GET_MARKET_HOURS_SINGLE;
    send( uuid, url, REQUEST_TIMEOUT, REQUEST_RETRIES );
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritrade::getOptionChain( const QString& symbol, const QString& strategy, const QString& contractType, bool includeQuotes, const QDate& fromDate, const QDate& toDate )
{
    QUrlQuery urlQuery;
    urlQuery.addQueryItem( "symbol", symbol );
    urlQuery.addQueryItem( "strategy", strategy );
    urlQuery.addQueryItem( "contractType", contractType );
    urlQuery.addQueryItem( "includeQuotes", includeQuotes ? "TRUE" : "FALSE" );

    if ( fromDate.isValid() )
        urlQuery.addQueryItem( "fromDate", fromDate.toString( Qt::ISODate ) );

    if ( toDate.isValid() )
        urlQuery.addQueryItem( "toDate", toDate.toString( Qt::ISODate ) );

    QUrl url( endpoints_[GET_OPTION_CHAIN] );
    url.setQuery( urlQuery );

    const QUuid uuid( QUuid::createUuid() );

    QMutexLocker guard( &m_ );

    pendingRequests_[uuid] = GET_OPTION_CHAIN;
    send( uuid, url, REQUEST_TIMEOUT, REQUEST_RETRIES );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritrade::getPriceHistory( const QString& symbol, int period, const QString& periodType, int freq, const QString& freqType, const QDateTime& fromDate, const QDateTime& toDate )
{
    QString s( endpoints_[GET_PRICE_HISTORY] );
    s.replace( "{symbol}", symbol );

    QUrlQuery urlQuery;
    urlQuery.addQueryItem( "frequency", QString::number( freq ) );
    urlQuery.addQueryItem( "frequencyType", freqType );
    urlQuery.addQueryItem( "periodType", periodType );

    if (( fromDate.isValid() ) && ( toDate.isValid() ))
    {
        urlQuery.addQueryItem( "startDate", QString::number( fromDate.toMSecsSinceEpoch() ) );
        urlQuery.addQueryItem( "endDate", QString::number( toDate.toMSecsSinceEpoch() ) );
    }
    else
    {
        urlQuery.addQueryItem( "period", QString::number( period ) );

        if ( fromDate.isValid() )
            urlQuery.addQueryItem( "startDate", QString::number( fromDate.toMSecsSinceEpoch() ) );
        else if ( toDate.isValid() )
            urlQuery.addQueryItem( "endDate", QString::number( toDate.toMSecsSinceEpoch() ) );
    }

    QUrl url( s );
    url.setQuery( urlQuery );

    const QUuid uuid( QUuid::createUuid() );

    // save off request information
    PriceHistoryRequest request;
    request.period = period;
    request.periodType = periodType;
    request.freq = freq;
    request.freqType = freqType;
    request.fromDate = fromDate;
    request.toDate = toDate;

    priceHistoryRequests_[uuid] = request;

    // request!
    QMutexLocker guard( &m_ );

    pendingRequests_[uuid] = GET_PRICE_HISTORY;
    send( uuid, url, REQUEST_TIMEOUT, REQUEST_RETRIES );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritrade::getQuote( const QString& symbol )
{
    if ( symbol.isEmpty() )
        return;

    QString s( endpoints_[GET_QUOTE] );
    s.replace( "{symbol}", symbol );

    const QUuid uuid( QUuid::createUuid() );

    QMutexLocker guard( &m_ );

    pendingRequests_[uuid] = GET_QUOTE;
    send( uuid, s, REQUEST_TIMEOUT, REQUEST_RETRIES );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritrade::getQuotes( const QStringList& symbols )
{
    if ( symbols.isEmpty() )
        return;

    QString s;

    foreach ( const QString& symbol, symbols )
    {
        if ( s.length() )
            s.append( "," );

        s.append( symbol );
    }

    QUrlQuery urlQuery;
    urlQuery.addQueryItem( "symbol", QUrl::toPercentEncoding( s ) );

    QUrl url( endpoints_[GET_QUOTES] );
    url.setQuery( urlQuery );

    const QUuid uuid( QUuid::createUuid() );

    QMutexLocker guard( &m_ );

    pendingRequests_[uuid] = GET_QUOTES;
    send( uuid, url, REQUEST_TIMEOUT, REQUEST_RETRIES );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined( QT_DEBUG )
void TDAmeritrade::simulateAccounts( const QJsonDocument& doc )
{
    parseAccountsDoc( doc );
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined( QT_DEBUG )
void TDAmeritrade::simulateMarketHours( const QJsonDocument& doc )
{
    parseMarketHoursDoc( doc );
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined( QT_DEBUG )
void TDAmeritrade::simulateOptionChain( const QJsonDocument& doc )
{
    parseOptionChainDoc( doc );
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined( QT_DEBUG )
void TDAmeritrade::simulatePriceHistory( const QJsonDocument& doc, int period, const QString& periodType, int freq, const QString& freqType, const QDateTime& fromDate, const QDateTime& toDate )
{
    PriceHistoryRequest request;
    request.period = period;
    request.periodType = periodType;
    request.freq = freq;
    request.freqType = freqType;
    request.fromDate = fromDate;
    request.toDate = toDate;

    parsePriceHistoryDoc( request, doc );
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined( QT_DEBUG )
void TDAmeritrade::simulateQuotes( const QJsonDocument& doc )
{
    parseQuotesDoc( doc );
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritrade::onProcessDocumentJson( const QUuid& uuid, const QByteArray& request, const QString& requestType, int status, const QJsonDocument& response )
{
    Q_UNUSED( request )
    Q_UNUSED( requestType )

    Endpoint type;

    {
        QMutexLocker guard( &m_ );

        if ( !pendingRequests_.contains( uuid ) )
            return;

        type = pendingRequests_[uuid];

        pendingRequests_.remove( uuid );
    }

    if ( 200 != status )
    {
        LOG_WARN << "bad response " << qPrintable( uuid.toString() ) << " " << status;
        return;
    }

    switch ( type )
    {
    case GET_ACCOUNT:
    case GET_ACCOUNTS:
        parseAccountsDoc( response );
        break;
    case GET_INSTRUMENT:
    case GET_INSTRUMENTS:
        parseInstrumentsDoc( response );
        break;
    case GET_MARKET_HOURS:
    case GET_MARKET_HOURS_SINGLE:
        parseMarketHoursDoc( response );
        break;
    case GET_OPTION_CHAIN:
        parseOptionChainDoc( response );
        break;
    case GET_PRICE_HISTORY:
        parsePriceHistoryDoc( priceHistoryRequests_[uuid], response );
        priceHistoryRequests_.remove( uuid );
        break;
    case GET_QUOTE:
    case GET_QUOTES:
        parseQuotesDoc( response );
        break;
    default:
        LOG_WARN << "unhandled endpoint type " << type;
        break;
    }

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritrade::loadEndpoints()
{
    static const QString prefix( "TDAmeritrade/" );

    QSettings settings( INI_FILE, QSettings::IniFormat );

    for ( EndpointMap::const_iterator i( endpointNames_.constBegin() ); i != endpointNames_.constEnd(); ++i )
    {
        const QVariant value( settings.value( prefix + i.value() ) );

        if ( !value.isValid() )
            LOG_WARN << "bad endpoint " << qPrintable( i.value() );
        else
        {
            const QString v( value.toString() );

            LOG_DEBUG << "endpoint " << qPrintable( i.value() ) << " " << qPrintable( v );
            endpoints_[i.key()] = v;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritrade::parseAccountsDoc( const QJsonDocument& doc )
{
    if ( doc.isObject() )
    {
        QJsonArray a;
        a.append( doc.object() );

        emit accountsReceived( a );
    }
    else if ( doc.isArray() )
    {
        emit accountsReceived( doc.array() );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritrade::parseInstrumentsDoc( const QJsonDocument& doc )
{
    if ( !doc.isObject() )
    {
        LOG_WARN << "not an object";
        return;
    }

    emit instrumentReceived( doc.object() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritrade::parseMarketHoursDoc( const QJsonDocument& doc )
{
    if ( !doc.isObject() )
    {
        LOG_WARN << "not an object";
        return;
    }

    emit marketHoursReceived( doc.object() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritrade::parseOptionChainDoc( const QJsonDocument& doc )
{
    if ( !doc.isObject() )
    {
        LOG_WARN << "not an object";
        return;
    }

    const QJsonObject obj( doc.object() );

    const QString symbol( obj[JSON_SYMBOL].toString() );

    // validate
    if ( symbol.isEmpty() )
    {
        LOG_WARN << "missing symbol";
        return;
    }

    // emit
    emit optionChainReceived( obj );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritrade::parsePriceHistoryDoc( const PriceHistoryRequest& request, const QJsonDocument& doc )
{
    if ( !doc.isObject() )
    {
        LOG_WARN << "not an object";
        return;
    }

    QJsonObject obj( doc.object() );

    if (( obj.contains( JSON_EMPTY ) ) && ( obj[JSON_EMPTY].isBool() ))
        if ( !obj[JSON_EMPTY].toBool() )
        {
            obj[JSON_PERIOD] = request.period;
            obj[JSON_PERIOD_TYPE] = request.periodType;
            obj[JSON_FREQUENCY] = request.freq;
            obj[JSON_FREQUENCY_TYPE] = request.freqType;

            if ( request.fromDate.isValid() )
                obj[JSON_START_DATE] = request.fromDate.toString( Qt::ISODateWithMs );
            else
                obj[JSON_START_DATE] = QJsonValue::Null;

            if ( request.toDate.isValid() )
                obj[JSON_END_DATE] = request.toDate.toString( Qt::ISODateWithMs );
            else
                obj[JSON_END_DATE] = QJsonValue::Null;

            // emit
            emit priceHistoryReceived( obj );
        }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritrade::parseQuotesDoc( const QJsonDocument& doc )
{
    if ( !doc.isObject() )
    {
        LOG_WARN << "not an object";
        return;
    }

    // emit!
    emit quotesReceived( doc.object() );
}
