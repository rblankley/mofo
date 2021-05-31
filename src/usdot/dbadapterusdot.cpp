/**
 * @file dbadapterusdot.cpp
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
#include "dbadapterusdot.h"
#include "stringsxml.h"

#include "../db/stringsdb.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>

// uncomment to debug content data
//#define DEBUG_JSON
//#define DEBUG_JSON_SAVE

///////////////////////////////////////////////////////////////////////////////////////////////////
DeptOfTheTreasuryDatabaseAdapter::DeptOfTheTreasuryDatabaseAdapter( QObject *parent ) :
    _Mybase( parent )
{
    // build rates table
    yieldCurveRates_[XML_BC_1MONTH] = 1;
    yieldCurveRates_[XML_BC_2MONTH] = 2;
    yieldCurveRates_[XML_BC_3MONTH] = 3;
    yieldCurveRates_[XML_BC_6MONTH] = 6;
    yieldCurveRates_[XML_BC_1YEAR] = 12 * 1;
    yieldCurveRates_[XML_BC_2YEAR] = 12 * 2;
    yieldCurveRates_[XML_BC_3YEAR] = 12 * 3;
    yieldCurveRates_[XML_BC_5YEAR] = 12 * 5;
    yieldCurveRates_[XML_BC_7YEAR] = 12 * 7;
    yieldCurveRates_[XML_BC_10YEAR] = 12 * 10;
    yieldCurveRates_[XML_BC_20YEAR] = 12 * 20;
    yieldCurveRates_[XML_BC_30YEAR] = 12 * 30;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DeptOfTheTreasuryDatabaseAdapter::~DeptOfTheTreasuryDatabaseAdapter()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DeptOfTheTreasuryDatabaseAdapter::transformDailyTreasuryBillRates( const QDomDocument& doc ) const
{
    const QDomElement feed = doc.documentElement();

    // parse entries
    QJsonArray data;

    for ( QDomElement entry = feed.firstChildElement( XML_ENTRY ); !entry.isNull(); entry = entry.nextSiblingElement( XML_ENTRY ) )
    {
        const QDomElement content = entry.firstChildElement( XML_CONTENT );

        if ( content.isNull() )
            continue;

        const QDomElement prop = content.firstChildElement( XML_PROPERTIES );

        if ( prop.isNull() )
            continue;

        const int dataId = prop.firstChildElement( XML_DATA_ID ).text().toInt();
        const QString indexDate = prop.firstChildElement( XML_INDEX_DATE ).text();
        const int week = prop.firstChildElement( XML_CF_WEEK ).text().toInt();

        const QString cusip4Wk = prop.firstChildElement( XML_CUSIP_4WK ).text();
        const QString cusip8Wk = prop.firstChildElement( XML_CUSIP_8WK ).text();
        const QString cusip13Wk = prop.firstChildElement( XML_CUSIP_13WK ).text();
        const QString cusip26Wk = prop.firstChildElement( XML_CUSIP_26WK ).text();
        const QString cusip52Wk = prop.firstChildElement( XML_CUSIP_52WK ).text();

        if ( cusip4Wk.length() )
        {
            QJsonObject obj;
            obj[DB_DATE] = indexDate;
            obj[DB_MATURITY_DATE] = prop.firstChildElement( XML_MATURITY_DATE_4WK ).text();
            obj[DB_CUSIP] = cusip4Wk;
            obj[DB_DATA_ID] = dataId;

            obj[DB_ROUND_CLOSE] = prop.firstChildElement( XML_ROUND_B1_CLOSE_4WK_2 ).text().toDouble();
            obj[DB_ROUND_YIELD] = prop.firstChildElement( XML_ROUND_B1_YIELD_4WK_2 ).text().toDouble();
            obj[DB_CLOSE_AVG] = prop.firstChildElement( XML_CS_4WK_CLOSE_AVG ).text().toDouble();
            obj[DB_YIELD_AVG] = prop.firstChildElement( XML_CS_4WK_YIELD_AVG ).text().toDouble();

            obj[DB_WEEK] = week;

            data.append( obj );
        }

        if ( cusip8Wk.length() )
        {
            QJsonObject obj;
            obj[DB_DATE] = indexDate;
            obj[DB_MATURITY_DATE] = prop.firstChildElement( XML_MATURITY_DATE_8WK ).text();
            obj[DB_CUSIP] = cusip8Wk;
            obj[DB_DATA_ID] = dataId;

            obj[DB_ROUND_CLOSE] = prop.firstChildElement( XML_ROUND_B1_CLOSE_8WK_2 ).text().toDouble();
            obj[DB_ROUND_YIELD] = prop.firstChildElement( XML_ROUND_B1_YIELD_8WK_2 ).text().toDouble();
            obj[DB_CLOSE_AVG] = prop.firstChildElement( XML_CS_8WK_CLOSE_AVG ).text().toDouble();
            obj[DB_YIELD_AVG] = prop.firstChildElement( XML_CS_8WK_YIELD_AVG ).text().toDouble();

            obj[DB_WEEK] = week;

            data.append( obj );
        }

        if ( cusip13Wk.length() )
        {
            QJsonObject obj;
            obj[DB_DATE] = indexDate;
            obj[DB_MATURITY_DATE] = prop.firstChildElement( XML_MATURITY_DATE_13WK ).text();
            obj[DB_CUSIP] = cusip13Wk;
            obj[DB_DATA_ID] = dataId;

            obj[DB_ROUND_CLOSE] = prop.firstChildElement( XML_ROUND_B1_CLOSE_13WK_2 ).text().toDouble();
            obj[DB_ROUND_YIELD] = prop.firstChildElement( XML_ROUND_B1_YIELD_13WK_2 ).text().toDouble();
            obj[DB_CLOSE_AVG] = prop.firstChildElement( XML_CS_13WK_CLOSE_AVG ).text().toDouble();
            obj[DB_YIELD_AVG] = prop.firstChildElement( XML_CS_13WK_YIELD_AVG ).text().toDouble();

            obj[DB_WEEK] = week;

            data.append( obj );
        }

        if ( cusip26Wk.length() )
        {
            QJsonObject obj;
            obj[DB_DATE] = indexDate;
            obj[DB_MATURITY_DATE] = prop.firstChildElement( XML_MATURITY_DATE_26WK ).text();
            obj[DB_CUSIP] = cusip26Wk;
            obj[DB_DATA_ID] = dataId;

            obj[DB_ROUND_CLOSE] = prop.firstChildElement( XML_ROUND_B1_CLOSE_26WK_2 ).text().toDouble();
            obj[DB_ROUND_YIELD] = prop.firstChildElement( XML_ROUND_B1_YIELD_26WK_2 ).text().toDouble();
            obj[DB_CLOSE_AVG] = prop.firstChildElement( XML_CS_26WK_CLOSE_AVG ).text().toDouble();
            obj[DB_YIELD_AVG] = prop.firstChildElement( XML_CS_26WK_YIELD_AVG ).text().toDouble();

            obj[DB_WEEK] = week;

            data.append( obj );
        }

        if ( cusip52Wk.length() )
        {
            QJsonObject obj;
            obj[DB_DATE] = indexDate;
            obj[DB_MATURITY_DATE] = prop.firstChildElement( XML_MATURITY_DATE_52WK ).text();
            obj[DB_CUSIP] = cusip52Wk;
            obj[DB_DATA_ID] = dataId;

            obj[DB_ROUND_CLOSE] = prop.firstChildElement( XML_ROUND_B1_CLOSE_52WK_2 ).text().toDouble();
            obj[DB_ROUND_YIELD] = prop.firstChildElement( XML_ROUND_B1_YIELD_52WK_2 ).text().toDouble();
            obj[DB_CLOSE_AVG] = prop.firstChildElement( XML_CS_52WK_CLOSE_AVG ).text().toDouble();
            obj[DB_YIELD_AVG] = prop.firstChildElement( XML_CS_52WK_YIELD_AVG ).text().toDouble();

            obj[DB_WEEK] = week;

            data.append( obj );
        }
    }

    QJsonObject rates;
    rates[DB_DATA] = data;

    // updated
    const QDomElement updated = feed.firstChildElement( XML_UPDATED );

    if ( !updated.isNull() )
        rates[DB_UPDATED] = updated.text();

    QJsonObject obj;
    obj[DB_TREAS_BILL_RATES] = rates;

    complete( obj );

    LOG_TRACE << "done";
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DeptOfTheTreasuryDatabaseAdapter::transformDailyTreasuryYieldCurveRates( const QDomDocument& doc ) const
{
    const QDomElement feed = doc.documentElement();

    // parse entries
    QJsonArray data;

    for ( QDomElement entry = feed.firstChildElement( XML_ENTRY ); !entry.isNull(); entry = entry.nextSiblingElement( XML_ENTRY ) )
    {
        const QDomElement content = entry.firstChildElement( XML_CONTENT );

        if ( content.isNull() )
            continue;

        const QDomElement prop = content.firstChildElement( XML_PROPERTIES );

        if ( prop.isNull() )
            continue;

        const int dataId = prop.firstChildElement( XML_ID ).text().toInt();
        const QString newDate = prop.firstChildElement( XML_NEW_DATE ).text();

        for ( QMap<QString, int>::const_iterator i = yieldCurveRates_.constBegin(); i != yieldCurveRates_.constEnd(); ++i )
        {
            const QString rate = prop.firstChildElement( i.key() ).text();

            if ( !rate.length() )
                continue;

            QJsonObject obj;
            obj[DB_DATE] = newDate;
            obj[DB_MONTHS] = i.value();
            obj[DB_DATA_ID] = dataId;

            obj[DB_RATE] = rate.toDouble();

            data.append( obj );
        }
    }

    QJsonObject rates;
    rates[DB_DATA] = data;

    // updated
    const QDomElement updated = feed.firstChildElement( XML_UPDATED );

    if ( !updated.isNull() )
        rates[DB_UPDATED] = updated.text();

    QJsonObject obj;
    obj[DB_TREAS_YIELD_CURVE_RATES] = rates;

    complete( obj );

    LOG_TRACE << "done";
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DeptOfTheTreasuryDatabaseAdapter::complete( const QJsonObject& obj ) const
{
#ifdef DEBUG_JSON
    saveObject( obj, "transform.json" );
#endif

    emit transformComplete( obj );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DeptOfTheTreasuryDatabaseAdapter::saveObject( const QJsonObject& obj, const QString& filename )
{
#if !defined( DEBUG_JSON )
    Q_UNUSED( obj )
    Q_UNUSED( filename )
#elif !defined( DEBUG_JSON_SAVE )
    Q_UNUSED( filename )
#endif

#ifdef DEBUG_JSON
    const QJsonDocument doc( obj );
    const QByteArray a( doc.toJson() );

#if defined(HAVE_CLIO_H)
    LOG_TRACE << HEX_DUMP( a.constData(), a.length() );
#else
    LOG_TRACE << a;
#endif

#ifdef DEBUG_JSON_SAVE
    QFile f( filename );

    if ( f.open( QFile::WriteOnly ) )
    {
        f.write( a );
        f.close();
    }
#endif
#endif
}

