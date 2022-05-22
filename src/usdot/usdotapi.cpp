/**
 * @file usdotapi.cpp
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
#include "stringsxml.h"
#include "usdotapi.h"

#include <QDomDocument>
#include <QDomElement>
#include <QSettings>
#include <QUrl>
#include <QUrlQuery>

static const QString INI_FILE( SYS_CONF_DIR "endpoints.config" );

static const QString TREAS_BILL_RATE_DATA( "DailyTreasuryBillRateData" );
static const QString TREAS_YIELD_CURVE_DATA( "DailyTreasuryYieldCurveRateData" );

///////////////////////////////////////////////////////////////////////////////////////////////////
DeptOfTheTreasury::DeptOfTheTreasury( QObject *parent ) :
    _Mybase( parent )
{
    endpointNames_[GET_DAILY_TREASURY_BILL_RATES] = "getDailyTreasuryBillRates";
    endpointNames_[GET_DAILY_TREASURY_YIELD_CURVE_RATES] = "getDailyTreasuryYieldCurveRates";

    loadEndpoints();

    connect( this, &_Myt::processDocumentXml, this, &_Myt::onProcessDocumentXml, Qt::DirectConnection );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DeptOfTheTreasury::~DeptOfTheTreasury()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DeptOfTheTreasury::getDailyTreasuryBillRates( int year, int month )
{
    const QDate now( QDate::currentDate() );

    if ( year <= 0 )
        year = now.year();

    if ( month <= 0 )
        month = now.month();

    QString filter( QString( "%1%2" ).arg( year, 4, 10, QChar('0') ).arg( month, 2, 10, QChar('0') ) );

    QUrlQuery urlQuery;
    urlQuery.addQueryItem( "data", "daily_treasury_bill_rates" );
    urlQuery.addQueryItem( "field_tdr_date_value_month", filter );

    QUrl url( endpoints_[GET_DAILY_TREASURY_BILL_RATES] );
    url.setQuery( urlQuery );

    const QUuid uuid( QUuid::createUuid() );

    QMutexLocker guard( &m_ );

    pendingRequests_[uuid] = GET_DAILY_TREASURY_BILL_RATES;
    send( uuid, url, REQUEST_TIMEOUT, REQUEST_RETRIES );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DeptOfTheTreasury::getDailyTreasuryYieldCurveRates( int year, int month )
{
    const QDate now( QDate::currentDate() );

    if ( year <= 0 )
        year = now.year();

    if ( month <= 0 )
        month = now.month();

    QString filter( QString( "%1%2" ).arg( year, 4, 10, QChar('0') ).arg( month, 2, 10, QChar('0') ) );

    QUrlQuery urlQuery;
    urlQuery.addQueryItem( "data", "daily_treasury_yield_curve" );
    urlQuery.addQueryItem( "field_tdr_date_value_month", filter );

    QUrl url( endpoints_[GET_DAILY_TREASURY_YIELD_CURVE_RATES] );
    url.setQuery( urlQuery );

    const QUuid uuid( QUuid::createUuid() );

    QMutexLocker guard( &m_ );

    pendingRequests_[uuid] = GET_DAILY_TREASURY_YIELD_CURVE_RATES;
    send( uuid, url, REQUEST_TIMEOUT, REQUEST_RETRIES );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined( QT_DEBUG )
void DeptOfTheTreasury::simulateDailyTreasuryBillRates( const QDomDocument& doc )
{
    parseDailyTreasuryBillRatesDoc( doc );
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined( QT_DEBUG )
void DeptOfTheTreasury::simulateDailyTreasuryYieldCurveRates( const QDomDocument& doc )
{
    parseDailyTreasuryBillYieldCurveDoc( doc );
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
void DeptOfTheTreasury::onProcessDocumentXml( const QUuid& uuid, const QByteArray& request, const QString& requestType, int status, const QDomDocument& response )
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
    case GET_DAILY_TREASURY_BILL_RATES:
        parseDailyTreasuryBillRatesDoc( response );
        break;
    case GET_DAILY_TREASURY_YIELD_CURVE_RATES:
        parseDailyTreasuryBillYieldCurveDoc( response );
        break;
    default:
        LOG_WARN << "unhandled endpoint type " << type;
        break;
    }

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DeptOfTheTreasury::loadEndpoints()
{
    static const QString prefix( "DeptOfTheTreasury/" );

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
void DeptOfTheTreasury::parseDailyTreasuryBillRatesDoc( const QDomDocument& doc )
{
    if ( doc.isNull() )
    {
        LOG_WARN << "bad document";
        return;
    }

    const QDomElement feed = doc.documentElement();

    if ( XML_FEED != feed.tagName() )
        LOG_WARN << "bad or missing root";
    else
    {
        // validate document title
        const QDomElement title = feed.firstChildElement( XML_TITLE );

        if ( title.isNull() )
            LOG_WARN << "bad or missing xml title tag";
        else if ( TREAS_BILL_RATE_DATA != title.text() )
            LOG_WARN << "bad or missing title";
        else
        {
            // emit
            emit dailyTreasuryBillRatesReceived( doc );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DeptOfTheTreasury::parseDailyTreasuryBillYieldCurveDoc( const QDomDocument& doc )
{
    if ( doc.isNull() )
    {
        LOG_WARN << "bad document";
        return;
    }

    const QDomElement feed = doc.documentElement();

    if ( XML_FEED != feed.tagName() )
        LOG_WARN << "bad or missing root";
    else
    {
        // validate document title
        const QDomElement title = feed.firstChildElement( XML_TITLE );

        if ( title.isNull() )
            LOG_WARN << "bad or missing xml title tag";
        else if ( TREAS_YIELD_CURVE_DATA != title.text() )
            LOG_WARN << "bad or missing title";
        else
        {
            // emit
            emit dailyTreasuryYieldCurveRatesReceived( doc );
        }
    }
}
