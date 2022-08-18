/**
 * @file appdb.cpp
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

#include "appdb.h"
#include "common.h"
#include "stringsdb.h"

#include <cmath>

#include <QApplication>
#include <QJsonArray>
#include <QJsonObject>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QThread>

static const QString DB_NAME( "appdb.db" );
static const QString DB_VERSION( "12" );

QMutex AppDatabase::instanceMutex_;
AppDatabase *AppDatabase::instance_( nullptr );

///////////////////////////////////////////////////////////////////////////////////////////////////
AppDatabase::AppDatabase() :
    _Mybase( DB_NAME, DB_VERSION )
{
    // open database
    if ( open() )
        readSettings();

    // make list of configuration values
    configs_.append( "equityRefreshRate" );
    configs_.append( "equityTradeCost" );
    configs_.append( "equityTradeCostNonExchange" );
    configs_.append( "equityWatchLists" );

    configs_.append( "history" );
    configs_.append( "marketTypes" );
    configs_.append( "numDays" );
    configs_.append( "numTradingDays" );
    configs_.append( "palette" );
    configs_.append( "paletteHighlight" );

    configs_.append( "optionChainRefreshRate" );
    configs_.append( "optionChainExpiryEndDate" );
    configs_.append( "optionChainWatchLists" );
    configs_.append( "optionTradeCost" );
    configs_.append( "optionCalcMethod" );

    configs_.append( "optionAnalysisFilter" );

    // create mapping of table names
    tableNames_[HeaderView] = "headerStates";
    tableNames_[Splitter] = "splitterStates";
    tableNames_[PriceHistory] = "priceHistoryStates";
    tableNames_[Dialog] = "dialogStates";

    // list of good friday dates
    goodFriday_.append( QDate( 2000,  4, 21 ) );
    goodFriday_.append( QDate( 2001,  4, 13 ) );
    goodFriday_.append( QDate( 2002,  3, 29 ) );
    goodFriday_.append( QDate( 2003,  4, 18 ) );
    goodFriday_.append( QDate( 2004,  4,  9 ) );
    goodFriday_.append( QDate( 2005,  3, 25 ) );
    goodFriday_.append( QDate( 2006,  4, 14 ) );
    goodFriday_.append( QDate( 2007,  4,  6 ) );
    goodFriday_.append( QDate( 2008,  3, 21 ) );
    goodFriday_.append( QDate( 2009,  4, 10 ) );
    goodFriday_.append( QDate( 2010,  4,  2 ) );
    goodFriday_.append( QDate( 2011,  4, 22 ) );
    goodFriday_.append( QDate( 2012,  4,  6 ) );
    goodFriday_.append( QDate( 2013,  3, 29 ) );
    goodFriday_.append( QDate( 2014,  4, 18 ) );
    goodFriday_.append( QDate( 2015,  4,  3 ) );
    goodFriday_.append( QDate( 2016,  3, 25 ) );
    goodFriday_.append( QDate( 2017,  4, 14 ) );
    goodFriday_.append( QDate( 2018,  3, 30 ) );
    goodFriday_.append( QDate( 2019,  4, 19 ) );
    goodFriday_.append( QDate( 2020,  4, 10 ) );
    goodFriday_.append( QDate( 2021,  4,  2 ) );
    goodFriday_.append( QDate( 2022,  4, 15 ) );
    goodFriday_.append( QDate( 2023,  4,  7 ) );
    goodFriday_.append( QDate( 2024,  3, 29 ) );
    goodFriday_.append( QDate( 2025,  4, 18 ) );
    goodFriday_.append( QDate( 2026,  4,  3 ) );
    goodFriday_.append( QDate( 2027,  3, 26 ) );
    goodFriday_.append( QDate( 2028,  4, 14 ) );
    goodFriday_.append( QDate( 2029,  3, 30 ) );

#if defined( QT_DEBUG )
    foreach ( const QDate& d, goodFriday_ )
        assert( 5 == d.dayOfWeek() );

    QMap<int, int> tradingDays;
    tradingDays[2000] = 252;
    tradingDays[2001] = 252; // 248 (closed 4 days extra due to 9/11)
    tradingDays[2002] = 252;
    tradingDays[2003] = 252;
    tradingDays[2004] = 252;
    tradingDays[2005] = 252;
    tradingDays[2006] = 251;
    tradingDays[2007] = 252; // 251 Tribute to former US President Gerald Ford January 2, 2007
    tradingDays[2008] = 253;
    tradingDays[2009] = 252;
    tradingDays[2010] = 251; // 252 The 2011 New Years Eve falls in 2010
    tradingDays[2011] = 252;
    tradingDays[2012] = 252; // 250 (closed 2 days extra due to Hurricane Sandy)
    tradingDays[2013] = 252;
    tradingDays[2014] = 252;
    tradingDays[2015] = 252;
    tradingDays[2016] = 252;
    tradingDays[2017] = 251;
    tradingDays[2018] = 252;
    tradingDays[2019] = 252;
    tradingDays[2020] = 253;
    tradingDays[2021] = 251; // 252 The 2022 New Years Eve falls in 2021
    tradingDays[2022] = 251;
    tradingDays[2023] = 250;
    tradingDays[2024] = 252;

    QDate d( 2000, 1, 1 );

    do
    {
        const int y( d.year() );

        setCurrentDateTime( QDateTime( d, QTime( 8, 0 ) ) );
        d = d.addYears( 1 );

        if ( tradingDays.contains( y ) )
            assert( tradingDays[y] == numTradingDaysUntil( d ) );

    } while ( d.year() < 2030 );

    setCurrentDateTime( QDateTime() );
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
AppDatabase::~AppDatabase()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QStringList AppDatabase::accounts() const
{
    static const QString sql( "SELECT * FROM accounts" );

    QStringList result;

    QSqlQuery query( connection() );
    query.setForwardOnly( true );

    // exec sql
    if ( !query.exec( sql ) )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
    }
    else
    {
        while ( query.next() )
        {
            const QSqlRecord rec( query.record() );

            const QString accountId( rec.value( "accountId" ).toString() );

            QString nickname( rec.value( "nickname" ).toString() );

            // mask account id
            if ( nickname.isEmpty() )
            {
                nickname = accountId;

                int i( nickname.length() - 4 );

                while ( 0 < i-- )
                    nickname[i] = '*';
            }

            result.append(
                QString( "%1;%2;%3;%4" ).arg(
                    accountId,
                    rec.value( "type" ).toString(),
                    nickname,
                    rec.value( "isDefault" ).toString() ) );
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QJsonObject AppDatabase::configs() const
{
    QJsonObject obj;
    QVariant v;

    foreach ( const QString& c, configs_ )
        if ( readSetting( c, v ) )
            obj[c] = v.toString();

    return obj;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QDateTime AppDatabase::currentDateTime() const
{
    if ( now_.isValid() )
        return now_;

    return QDateTime::currentDateTime();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QByteArray AppDatabase::filter( const QString& name ) const
{
    static const QString sql( "SELECT * FROM filters WHERE name=:name" );

    QByteArray result;

    QSqlQuery query( connection() );
    query.setForwardOnly( true );
    query.prepare( sql );

    query.bindValue( ":name", name );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
    }
    else
    {
        while ( query.next() )
        {
            const QSqlRecord rec( query.record() );

            result.append( rec.value( "value" ).toByteArray() );
        }

        if ( result.isEmpty() )
             LOG_WARN << "no row(s) found";
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QStringList AppDatabase::filters() const
{
    static const QString sql( "SELECT DISTINCT name FROM filters" );

    QStringList result;

    QSqlQuery query( connection() );
    query.setForwardOnly( true );

    // exec sql
    if ( !query.exec( sql ) )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
    }
    else
    {
        while ( query.next() )
        {
            const QSqlRecord rec( query.record() );

            result.append( rec.value( "name" ).toString() );
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AppDatabase::isMarketOpen( const QDateTime& dt, const QString& marketType, const QString& product, bool *isExtended ) const
{
    QString sql( "SELECT isOpen, product FROM marketHours WHERE DATE(date)=DATE(:date) AND marketType=:marketType" );

    if ( product.length() )
        sql += " AND product=:product";

    QSqlQuery query( connection() );
    query.setForwardOnly( true );
    query.prepare( sql );

    query.bindValue( ":date", dt.date().toString( Qt::ISODate ) );
    query.bindValue( ":marketType", marketType );

    if ( product.length() )
        query.bindValue( ":product", product );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    if ( !query.next() )
        return false;

    // ---- //

    if ( isExtended )
        (*isExtended) = false;

    do
    {
        const QSqlRecord rec( query.record() );

        if ( !rec.value( "isOpen" ).toBool() )
            return false;
        else if ( !checkSessionHours( dt, marketType, rec.value( "product" ).toString(), isExtended ) )
            return false;

    } while ( query.next() );

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AppDatabase::marketHoursExist( const QDate& date, const QString& marketType ) const
{
    QString sql( "SELECT isOpen FROM marketHours WHERE DATE(date)=DATE(:date)" );

    if ( marketType.length() )
        sql += " AND marketType=:marketType";

    QSqlQuery query( connection() );
    query.setForwardOnly( true );
    query.prepare( sql );

    query.bindValue( ":date", date.toString( Qt::ISODate ) );

    if ( marketType.length() )
        query.bindValue( ":marketType", marketType );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    return query.next();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QMap<QString, MarketProductHours> AppDatabase::marketHours( const QDate& date, const QString& marketType, const QString& product )
{
    static const QString sql( "SELECT * FROM sessionHours WHERE DATE(date)=DATE(:date) AND marketType=:marketType" );

    QMap<QString, MarketProductHours> result;

    QSqlQuery query( connection() );
    query.setForwardOnly( true );
    query.prepare( sql );

    query.bindValue( ":date", date.toString( Qt::ISODate ) );
    query.bindValue( ":marketType", marketType );

    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
    }
    else
    {
        while ( query.next() )
        {
            const QSqlRecord rec( query.record() );
            const QString p( rec.value( "product" ).toString() );

            if (( product.isEmpty() ) || ( product == p ))
            {
                const QString t( rec.value( "sessionHoursType" ).toString() );
                const QDateTime start( QDateTime::fromString( rec.value( "start" ).toString(), Qt::ISODateWithMs ) );
                const QDateTime end( QDateTime::fromString( rec.value( "end" ).toString(), Qt::ISODateWithMs ) );

                // add product to results if not there already
                if ( !result.contains( p ) )
                    result[p] = MarketProductHours();

                // handle session type
                if ( DB_PRE_MARKET == t )
                {
                    result[p].preMarketStart = start;
                    result[p].preMarketEnd = end;
                }
                else if ( DB_REGULAR_MARKET == t )
                {
                    result[p].regularMarketStart = start;
                    result[p].regularMarketEnd = end;
                }
                else if ( DB_POST_MARKET == t )
                {
                    result[p].postMarketStart = start;
                    result[p].postMarketEnd = end;
                }

            } // product
        } // for each row
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QStringList AppDatabase::marketTypes( bool hasHours ) const
{
    QStringList types;
    QVariant v;

    if ( readSetting( "marketTypes", v ) )
        types = v.toString().split( ',' );

    // ---- //

    QStringList result;

    QString sql( "SELECT type FROM marketType" );

    if ( hasHours )
        sql += " WHERE 1=hasMarketHours";

    QSqlQuery query( connection() );
    query.setForwardOnly( true );

    if ( !query.exec( sql ) )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
    }
    else
    {
        while ( query.next() )
        {
            const QSqlRecord rec( query.record() );

            const QString type( rec.value( "type" ).toString() );

            if ( types.contains( type ) )
                result.append( type );
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AppDatabase::removeFilter( const QString& name )
{
    static const QString sql( "DELETE FROM filters WHERE name=:name" );

    QMutexLocker guard( &writer_ );

    QSqlQuery query( connection() );
    query.prepare( sql );

    query.bindValue( ":name", name );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during delete " << e.type() << " " << qPrintable( e.text() );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AppDatabase::removeWatchlist( const QString& name )
{
    static const QString sql( "DELETE FROM %1 WHERE name=:name" );

    QMutexLocker guard( &writer_ );

    QStringList tables;
    tables.append( "watchlist" );
    tables.append( "indices" );

    foreach ( const QString& table, tables )
    {
        QSqlQuery query( connection() );
        query.prepare( sql.arg( table ) );

        query.bindValue( ":name", name );

        // exec sql
        if ( !query.exec() )
        {
            const QSqlError e( query.lastError() );

            LOG_ERROR << "error during delete " << e.type() << " " << qPrintable( e.text() );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AppDatabase::removeWidgetState( WidgetType type, const QString& groupName, const QString& name )
{
    static const QString sql( "DELETE FROM %1 WHERE groupName=:groupName AND name=:name" );

    QMutexLocker guard( &writer_ );

    // determine table
    const QString table( tableNames_[type] );

    if ( table.length() )
    {
        // create new filter
        QSqlQuery query( connection() );
        query.prepare( sql.arg( table ) );

        query.bindValue( ":groupName", groupName );
        query.bindValue( ":name", name );

        // exec sql
        if ( !query.exec() )
        {
            const QSqlError e( query.lastError() );

            LOG_ERROR << "error during delete " << e.type() << " " << qPrintable( e.text() );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double AppDatabase::riskFreeRate( double term ) const
{
    const QString sql( "SELECT * FROM riskFreeInterestRates WHERE "
        "DATE(:dateMin)<=DATE(date) AND "
        "DATE(date)<= DATE(:dateMax) AND "
        "source='" + DB_TREAS_YIELD_CURVE + "' "
            "ORDER BY DATE(date) DESC, term ASC" );

    const QDate dateMax( currentDateTime().date() );
    const QDate dateMin( dateMax.addDays( -7 ) );

    double rate( 0.0 );

    double lowerTerm( 0.0 );
    double lowerRate( 0.0 );

    QSqlQuery query( connection() );
    query.setForwardOnly( true );
    query.prepare( sql );

    query.bindValue( ":dateMin", dateMin.toString( Qt::ISODate ) );
    query.bindValue( ":dateMax", dateMax.toString( Qt::ISODate ) );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
    }
    else
    {
        while ( query.next() )
        {
            const QSqlRecord rec( query.record() );

            const double upperTerm( rec.value( "term" ).toDouble() );
            const double upperRate( rec.value( "rate" ).toDouble() );

            if ( term <= upperTerm )
            {
                const double ratio = (term - lowerTerm) / (upperTerm - lowerTerm);

                rate = lowerRate + ratio * (upperRate - lowerRate);
                break;
            }

            lowerTerm = upperTerm;
            lowerRate = upperRate;
        }
    }

    return rate;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AppDatabase::setAccountNicknames( const QStringList& accounts )
{
    const QString sql( "UPDATE accounts "
        "SET nickname=:nickname,isDefault=:isDefault "
            "WHERE accountId=:accountId" );

    QMutexLocker guard( &writer_ );

    // start transaction
    QSqlDatabase conn( connection() );

    if ( !conn.transaction() )
    {
        const QSqlError e( conn.lastError() );

        LOG_ERROR << "failed to start transaction " << e.type() << " " << qPrintable( e.text() );
        return;
    }

    // prepare sql
    QSqlQuery query( conn );
    query.prepare( sql );

    foreach ( const QString& account, accounts )
    {
        const QStringList parts( account.split( ';' ) );

        if ( 3 <= parts.size() )
        {
            query.bindValue( ":accountId", parts[0] );
            query.bindValue( ":nickname", parts[1] );
            query.bindValue( ":isDefault", parts[2] );

            // exec sql
            if ( !query.exec() )
            {
                const QSqlError e( query.lastError() );

                LOG_WARN << "error during update " << e.type() << " " << qPrintable( e.text() );
            }
        }
    }

    // commit to database
    if ( !conn.commit() )
    {
        const QSqlError e( conn.lastError() );

        LOG_ERROR << "commit failed " << e.type() << " " << qPrintable( e.text() );

        if ( !conn.rollback() )
            LOG_FATAL << "rollback failed";

        return;
    }

    // emit!
    emit accountsChanged();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AppDatabase::setConfigs( const QJsonObject& value )
{
    QMutexLocker guard( &writer_ );

    // start transaction
    QSqlDatabase conn( connection() );

    if ( !conn.transaction() )
    {
        const QSqlError e( conn.lastError() );

        LOG_ERROR << "failed to start transaction " << e.type() << " " << qPrintable( e.text() );
        return;
    }

    // write each value
    for ( QJsonObject::const_iterator i( value.constBegin() ); i != value.constEnd(); ++i )
    {
        const QString v( i->toString() );

        if ( !writeSetting( i.key(), v ) )
            LOG_ERROR << "failed to write setting " << qPrintable( i.key() ) << " '" << qPrintable( v ) << "'";
    }

    // commit to database
    if ( !conn.commit() )
    {
        const QSqlError e( conn.lastError() );

        LOG_ERROR << "commit failed " << e.type() << " " << qPrintable( e.text() );

        if ( !conn.rollback() )
            LOG_FATAL << "rollback failed";

        return;
    }

    // refresh settings
    readSettings();

    // emit!
    emit configurationChanged();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AppDatabase::setFilter( const QString& name, const QByteArray& value )
{
    static const QString sql( "INSERT INTO filters (name,value) "
            "VALUES (:name,:value)" );

    QMutexLocker guard( &writer_ );

    // remove old filter
    removeFilter( name );

    // create new filter
    QSqlQuery query( connection() );
    query.prepare( sql );

    query.bindValue( ":name", name );
    query.bindValue( ":value", value );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during insert " << e.type() << " " << qPrintable( e.text() );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AppDatabase::setWatchlist( const QString& name, const QStringList& symbols )
{
    static const QString sql( "INSERT INTO watchlist (name,symbol) "
            "VALUES (:name,:symbol)" );

    QMutexLocker guard( &writer_ );

    // remove old list
    removeWatchlist( name );

    // start transaction
    QSqlDatabase conn( connection() );

    if ( !conn.transaction() )
    {
        const QSqlError e( conn.lastError() );

        LOG_ERROR << "failed to start transaction " << e.type() << " " << qPrintable( e.text() );
    }
    else
    {
        // create new list
        QSqlQuery query( conn );
        query.prepare( sql );

        foreach ( const QString& symbol, symbols )
        {
            query.bindValue( ":name", name );
            query.bindValue( ":symbol", symbol );

            // exec sql
            if ( !query.exec() )
            {
                const QSqlError e( query.lastError() );

                LOG_ERROR << "error during insert " << e.type() << " " << qPrintable( e.text() );
            }
        }

        // commit to database
        if ( !conn.commit() )
        {
            const QSqlError e( conn.lastError() );

            LOG_ERROR << "commit failed " << e.type() << " " << qPrintable( e.text() );

            if ( !conn.rollback() )
                LOG_FATAL << "rollback failed";
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AppDatabase::setWidgetState( WidgetType type, const QString& groupName, const QString& name, const QByteArray& state )
{
    static const QString sql( "REPLACE INTO %1 (groupName,name,state) "
            "VALUES (:groupName,:name,:state)" );

    QMutexLocker guard( &writer_ );

    // determine table
    const QString table( tableNames_[type] );

    if ( table.length() )
    {
        // create new filter
        QSqlQuery query( connection() );
        query.prepare( sql.arg( table ) );

        query.bindValue( ":groupName", groupName );
        query.bindValue( ":name", name );
        query.bindValue( ":state", state );

        // exec sql
        if ( !query.exec() )
        {
            const QSqlError e( query.lastError() );

            LOG_ERROR << "error during replace " << e.type() << " " << qPrintable( e.text() );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AppDatabase::treasuryYieldCurveDateRange( QDate& start, QDate& end ) const
{
    const QString sql( "SELECT date FROM riskFreeInterestRates WHERE "
        "source='" + DB_TREAS_YIELD_CURVE + "' ORDER BY DATE(date) %1 LIMIT 5" );

    // end date
    QSqlQuery queryEnd( connection() );
    queryEnd.setForwardOnly( true );

    if ( !queryEnd.exec( sql.arg( "DESC" ) ) )
    {
        const QSqlError e( queryEnd.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
    }
    else if ( queryEnd.next() )
    {
        const QSqlRecord rec( queryEnd.record() );

        end = QDate::fromString( rec.value( "date" ).toString(), Qt::ISODate );
    }

    // start date
    QSqlQuery queryStart( connection() );
    queryStart.setForwardOnly( true );

    if ( !queryStart.exec( sql.arg( "ASC" ) ) )
    {
        const QSqlError e( queryStart.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
    }
    else if ( queryStart.next() )
    {
        const QSqlRecord rec( queryStart.record() );

        start = QDate::fromString( rec.value( "date" ).toString(), Qt::ISODate );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QStringList AppDatabase::watchlist( const QString& name ) const
{
    static const QString sql( "SELECT * FROM watchlist WHERE name=:name" );

    QStringList result;

    QSqlQuery query( connection() );
    query.setForwardOnly( true );
    query.prepare( sql );

    query.bindValue( ":name", name );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
    }
    else
    {
        while ( query.next() )
        {
            const QSqlRecord rec( query.record() );

            result.append( rec.value( "symbol" ).toString() );
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QStringList AppDatabase::watchlists( bool includeIndices ) const
{
    static const QString sql( "SELECT DISTINCT name FROM %1" );

    QStringList result;

    QSqlQuery query( connection() );
    query.setForwardOnly( true );

    // exec sql
    if ( !query.exec( sql.arg( "watchlist" ) ) )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
    }
    else
    {
        while ( query.next() )
        {
            const QSqlRecord rec( query.record() );

            result.append( rec.value( "name" ).toString() );
        }
    }

    // remove each index from list
    if ( !includeIndices )
    {
        QSqlQuery indicesQuery( connection() );
        indicesQuery.setForwardOnly( true );

        // exec sql
        if ( !indicesQuery.exec( sql.arg( "indices" ) ) )
        {
            const QSqlError e( indicesQuery.lastError() );

            LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        }
        else
        {
            while ( indicesQuery.next() )
            {
                const QSqlRecord rec( indicesQuery.record() );
                const QString name( rec.value( "name" ).toString() );

                if ( result.contains( name ) )
                    result.removeAll( name );
            }
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QStringList AppDatabase::widgetGroupNames( WidgetType type ) const
{
    static const QString sql( "SELECT DISTINCT groupName FROM %1" );

    QStringList result;

    // determine table
    const QString table( tableNames_[type] );

    if ( table.length() )
    {
        QSqlQuery query( connection() );
        query.setForwardOnly( true );

        // exec sql
        if ( !query.exec( sql.arg( table ) ) )
        {
            const QSqlError e( query.lastError() );

            LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        }
        else
        {
            while ( query.next() )
            {
                const QSqlRecord rec( query.record() );

                result.append( rec.value( "groupName" ).toString() );
            }
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QByteArray AppDatabase::widgetState( WidgetType type, const QString& groupName, const QString& name ) const
{
    static const QString sql( "SELECT state FROM %1 WHERE groupName=:groupName AND name=:name" );

    QByteArray result;

    // determine table
    const QString table( tableNames_[type] );

    if ( table.length() )
    {
        QSqlQuery query( connection() );
        query.setForwardOnly( true );
        query.prepare( sql.arg( table ) );

        query.bindValue( ":groupName", groupName );
        query.bindValue( ":name", name );

        // exec sql
        if ( !query.exec() )
        {
            const QSqlError e( query.lastError() );

            LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        }
        else if ( !query.next() )
             LOG_WARN << "no row(s) found";
        else
        {
            const QSqlRecord rec( query.record() );

            result = rec.value( "state" ).toByteArray();
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QStringList AppDatabase::widgetStates( WidgetType type, const QString& groupName ) const
{
    static const QString sql( "SELECT name FROM %1 WHERE groupName=:groupName AND name NOT LIKE '[[%]]' ORDER BY name ASC" );

    QStringList result;

    // determine table
    const QString table( tableNames_[type] );

    if ( table.length() )
    {
        QSqlQuery query( connection() );
        query.setForwardOnly( true );
        query.prepare( sql.arg( table ) );

        query.bindValue( ":groupName", groupName );

        // exec sql
        if ( !query.exec() )
        {
            const QSqlError e( query.lastError() );

            LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        }
        else
        {
            while ( query.next() )
            {
                const QSqlRecord rec( query.record() );

                result.append( rec.value( "name" ).toString() );
            }
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
AppDatabase *AppDatabase::instance()
{
    if ( !instance_ )
    {
        QMutexLocker guard( &instanceMutex_ );

        if ( !instance_ )
            instance_ = new _Myt();
    }

    return instance_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int AppDatabase::numTradingDaysBetween( const QDate& d0, const QDate& d ) const
{
    const double days( numTradingDaysBetween( QDateTime( d0, QTime( 1, 0 ) ), QDateTime( d, QTime( 1, 0 ) ) ) );

    // floor result to give num days only
    // add in a slight adjustment to ensure rounding is not an issue
    // for example, 2.9999999999999999 should be 3
    return std::floor( days + 0.000000001 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double AppDatabase::numTradingDaysBetween( const QDateTime& dt0, const QDateTime& dt ) const
{
    // make sure dates are ordered correctly
    assert( dt0 <= dt );

    QDate now( dt0.date() );

    double days( 0 );

    do
    {
        // check day of week
        // 6 = sat
        // 7 = sun
        if ( now.dayOfWeek() < 6 )
        {
            const bool isMonday( 1 == now.dayOfWeek() );
            const bool isFriday( 5 == now.dayOfWeek() );
            const int nthDayOfWeek( std::ceil( now.day() / 7.0 ) );

            const int m( now.month() );
            const int d( now.day() );

            bool valid( true );

            // new years day
            if ((  1 == m &&  1 == d ) ||
                ( 12 == m && 31 == d && isFriday ) ||
                (  1 == m &&  2 == d && isMonday ))
                valid = false;

            // martin luther king jr
            // third monday in january
            if ( 1 == m && isMonday && 3 == nthDayOfWeek )
                valid = false;

            // presidents day
            // third monday in february
            if ( 2 == m && isMonday && 3 == nthDayOfWeek )
                valid = false;

            // good friday
            if ( isFriday )
                if ( goodFriday_.contains( now ) )
                    valid = false;

            // memorial day
            // last monday in may
            if ( 5 == m && isMonday && 6 == now.addDays( 7 ).month() )
                valid = false;

            // juneteenth
            if (( 6 == m && 19 == d ) ||
                ( 6 == m && 18 == d && isFriday ) ||
                ( 6 == m && 20 == d && isMonday ))
            {
                if ( 2022 <= now.year() )
                    valid = false;
            }

            // independance day
            if (( 7 == m && 4 == d ) ||
                ( 7 == m && 3 == d && isFriday ) ||
                ( 7 == m && 5 == d && isMonday ))
                valid = false;

            // labor day
            // first monday in september
            if ( 9 == m && isMonday && 1 == nthDayOfWeek )
                valid = false;

            // thanksgiving day
            // fourth thursday in november
            if ( 11 == m && 4 == now.dayOfWeek() && 4 == nthDayOfWeek )
                valid = false;

            // christmas day
            if (( 12 == m && 25 == d ) ||
                ( 12 == m && 24 == d && isFriday ) ||
                ( 12 == m && 26 == d && isMonday ))
                valid = false;

            if ( valid )
            {
                // TODO: check market hours table for closure

                // check for partial day
                if ( now < dt.date() )
                    days += 1.0;
                else
                {
                    double hoursRemain( dt0.secsTo( dt ) );
                    hoursRemain /= 3600.0;

                    hoursRemain -= 24.0 * std::floor( hoursRemain / 24.0 );

                    // TODO: early closures?

                    // 6.5 hours in a trading day
                    if ( 6.5 <= hoursRemain )
                        days += 1.0;
                    else
                        days += (hoursRemain / 6.5);
                }
            }
        }

        // increment current day
        now = now.addDays( 1 );

    } while ( now <= dt.date() );

    return days;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AppDatabase::removeConnection()
{
    const QString cname( connectionNameThread() );

    // do not remove application thread connection
    if ( cname == connectionName() )
        return;

    LOG_TRACE << "remove database " << qPrintable( cname );
    QSqlDatabase::removeDatabase( cname );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AppDatabase::processData( const QJsonObject& obj )
{
    const QDateTime now( currentDateTime() );

    bool accountsProcessed( false );
    bool marketHoursProcessed( false );
    bool treasBillRatesProcessed( false );
    bool treasYieldCurveRatesProcessed( false );

    bool result( true );

    {
        QMutexLocker guard( &writer_ );

        // START DB TRANSACION
        QSqlDatabase conn( connection() );

        if ( !conn.transaction() )
        {
            const QSqlError e( conn.lastError() );

            LOG_ERROR << "failed to start transaction " << e.type() << " " << qPrintable( e.text() );
        }
        else
        {
            // iterate accounts
            const QJsonObject::const_iterator accounts( obj.constFind( DB_ACCOUNTS ) );

            if (( obj.constEnd() != accounts ) && ( accounts->isArray() ))
            {
                foreach ( const QJsonValue& accountVal, accounts->toArray() )
                   if ( accountVal.isObject() )
                      result &= addAccount( now, accountVal.toObject() );

                accountsProcessed = result;
            }

            // iterate market hours
            const QJsonObject::const_iterator marketHours( obj.constFind( DB_MARKET_HOURS ) );

            if (( obj.constEnd() != marketHours ) && ( marketHours->isArray() ))
            {
                foreach ( const QJsonValue& hoursVal, marketHours->toArray() )
                   if ( hoursVal.isObject() )
                      result &= addMarketHours( hoursVal.toObject() );

                marketHoursProcessed = result;
            }

            // process treasury bill rates
            const QJsonObject::const_iterator treasBillRatesIt( obj.constFind( DB_TREAS_BILL_RATES ) );

            if (( obj.constEnd() != treasBillRatesIt ) && ( treasBillRatesIt->isObject() ))
            {
                const QJsonObject treasBillRates( treasBillRatesIt->toObject() );

                const QJsonObject::const_iterator rates( treasBillRates.constFind( DB_DATA ) );
                const QString updated( treasBillRates[DB_UPDATED].toString() );

                if ( rates->isArray() )
                {
                    LOG_DEBUG << "process treasury bill rates";

                    foreach ( const QJsonValue& rateVal, rates->toArray() )
                       if ( rateVal.isObject() )
                          result &= addTreasuryBillRate( QDateTime::fromString( updated, Qt::ISODate ), rateVal.toObject() );

                    treasBillRatesProcessed = result;
                }
            }

            // process treasury yield curve rates
            const QJsonObject::const_iterator treasYieldCurveRatesIt( obj.constFind( DB_TREAS_YIELD_CURVE_RATES ) );

            if (( obj.constEnd() != treasYieldCurveRatesIt ) && ( treasYieldCurveRatesIt->isObject() ))
            {
                const QJsonObject treasYieldCurveRates( treasYieldCurveRatesIt->toObject() );

                const QJsonObject::const_iterator rates( treasYieldCurveRates.constFind( DB_DATA ) );
                const QString updated( treasYieldCurveRates[DB_UPDATED].toString() );

                if ( rates->isArray() )
                {
                    LOG_DEBUG << "process treasury yield curve rates";

                    foreach ( const QJsonValue& rateVal, rates->toArray() )
                       if ( rateVal.isObject() )
                          result &= addTreasuryYieldCurveRate( QDateTime::fromString( updated, Qt::ISODate ), rateVal.toObject() );

                    treasYieldCurveRatesProcessed = result;
                }
            }

            // COMMIT DB TRANSACION

            // commit to database
            if (( result ) && ( !(result = conn.commit()) ))
            {
                const QSqlError e( conn.lastError() );

                LOG_ERROR << "commit failed " << e.type() << " " << qPrintable( e.text() );
            }

            if (( !result ) && ( !conn.rollback() ))
                LOG_FATAL << "rollback failed";
        }
    }

    // EMIT SIGNALS

    if ( accountsProcessed )
        emit accountsChanged();

    if ( marketHoursProcessed )
        emit marketHoursChanged();

    if ( treasBillRatesProcessed )
        emit treasuryBillRatesChanged();

    if ( treasYieldCurveRatesProcessed )
        emit treasuryYieldCurveRatesChanged();

    // remove database connection
    removeConnection();

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QStringList AppDatabase::createFiles() const
{
    QStringList files;
    files.append( ":/db/createdb_app.sql" );
    files.append( ":/db/default_app.sql" );

    return files;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QStringList AppDatabase::upgradeFiles( const QString& fromStr, const QString& toStr ) const
{
    int from( fromStr.toInt() );
    int to( toStr.toInt() );

    LOG_INFO << "upgrade database from " << from << " " << to;

    // upgrade each version step-by-step
    QStringList files;

    while ( from < to )
        files.append( QString( ":/db/version%0_app.sql" ).arg( ++from ) );

    return files;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AppDatabase::addAccount( const QDateTime& stamp, const QJsonObject& obj )
{
    static const QString sql( "INSERT INTO accounts (accountId,"
        "type,isClosingOnlyRestricted,isDayTrader,roundTrips) "
            "VALUES (:accountId,"
                ":type,:isClosingOnlyRestricted,:isDayTrader,:roundTrips) "
            "ON CONFLICT(accountId) DO UPDATE SET "
                "type=:type, "
                "isClosingOnlyRestricted=:isClosingOnlyRestricted, "
                "isDayTrader=:isDayTrader, "
                "roundTrips=:roundTrips " );

    const QString accountId( obj[DB_ACCOUNT_ID].toString() );

    if ( accountId.isEmpty() )
        return false;

    QSqlQuery query( connection() );
    query.prepare( sql );

    bindQueryValues( query, obj );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during replace " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    // parse account balances
    if ( !parseAccountBalances( stamp, accountId, obj ) )
        return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AppDatabase::addAccountBalances( const QDateTime& stamp, const QString& accountId, const QString& type, const QJsonObject& obj )
{
    static const QString sql( "REPLACE INTO balances (stamp,accountId,type,"
        "accruedInterest,cashBalance,cashReceipts,longOptionMarketValue,liquidationValue,longMarketValue,moneyMarketFund,savings,shortMarketValue,pendingDeposits,"
        "shortOptionMarketValue,mutualFundValue,bondValue,cashAvailableForTrading,cashAvailableForWithdrawal,cashCall,longNonMarginableMarketValue,totalCash,cashDebitCallValue,unsettledCash,"
        "longStockValue,shortStockValue,accountValue,availableFunds,availableFundsNonMarginableTrade,buyingPower,buyingPowerNonMarginableTrade,dayTradingBuyingPower,dayTradingBuyingPowerCall,equity,"
        "equityPercentage,longMarginValue,maintenanceCall,maintenanceRequirement,marginBalance,regTCall,shortBalance,shortMarginValue,sma,isInCall,"
        "stockBuyingPower,optionBuyingPower,dayTradingEquityCall,margin,marginEquity) "
            "VALUES (:stamp,:accountId,:type,"
                ":accruedInterest,:cashBalance,:cashReceipts,:longOptionMarketValue,:liquidationValue,:longMarketValue,:moneyMarketFund,:savings,:shortMarketValue,:pendingDeposits,"
                ":shortOptionMarketValue,:mutualFundValue,:bondValue,:cashAvailableForTrading,:cashAvailableForWithdrawal,:cashCall,:longNonMarginableMarketValue,:totalCash,:cashDebitCallValue,:unsettledCash,"
                ":longStockValue,:shortStockValue,:accountValue,:availableFunds,:availableFundsNonMarginableTrade,:buyingPower,:buyingPowerNonMarginableTrade,:dayTradingBuyingPower,:dayTradingBuyingPowerCall,:equity,"
                ":equityPercentage,:longMarginValue,:maintenanceCall,:maintenanceRequirement,:marginBalance,:regTCall,:shortBalance,:shortMarginValue,:sma,:isInCall,"
                ":stockBuyingPower,:optionBuyingPower,:dayTradingEquityCall,:margin,:marginEquity)" );

    QSqlQuery query( connection() );
    query.prepare( sql );

    query.bindValue( ":" + DB_STAMP, stamp.toString( Qt::ISODateWithMs ) );
    query.bindValue( ":" + DB_ACCOUNT_ID, accountId );
    query.bindValue( ":" + DB_TYPE, type );

    bindQueryValues( query, obj );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during replace " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AppDatabase::addMarketHours( const QJsonObject& obj )
{
    static const QString sql( "REPLACE INTO marketHours (date,marketType,product,"
        "isOpen,category,exchange) "
            "VALUES (:date,:marketType,:product,"
                ":isOpen,:category,:exchange)" );

    const QDate date( QDate::fromString( obj[DB_DATE].toString(), "yyyy-MM-dd" ) );
    const QString marketType( obj[DB_MARKET_TYPE].toString() );
    const QString product( obj[DB_PRODUCT].toString() );

    if (( !date.isValid() ) || ( marketType.isEmpty() ) || ( product.isEmpty() ))
        return false;

    // add product type
    QJsonObject::const_iterator productName( obj.constFind( DB_PRODUCT_NAME ) );

    if (( obj.constEnd() != productName ) && ( productName->isString() ))
        if ( !addProductType( product, productName->toString() ) )
            return false;

    // add market hours
    QSqlQuery query( connection() );
    query.prepare( sql );

    bindQueryValues( query, obj );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during replace " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    // parse session hours (optional)
    QJsonObject::const_iterator sessionHours( obj.constFind( DB_SESSION_HOURS ) );

    if (( obj.constEnd() != sessionHours ) && ( sessionHours->isObject() ))
        if ( !parseSessionHours( date, marketType, product, sessionHours->toObject() ) )
            return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AppDatabase::addProductType( const QString& type, const QString& description )
{
    static const QString sql( "REPLACE INTO productType (type,"
        "name) "
            "VALUES (?,"
                "?)" );

    if ( type.isEmpty() )
        return false;
    else if ( description.length() )
    {
        QSqlQuery query( connection() );
        query.prepare( sql );

        query.addBindValue( type );
        query.addBindValue( description );

        // exec sql
        if ( !query.exec() )
        {
            const QSqlError e( query.lastError() );

            LOG_ERROR << "error during replace " << e.type() << " " << qPrintable( e.text() );
            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AppDatabase::addSessionHours( const QDate& date, const QString& marketType, const QString& product, const QString& sessionHoursType, const QJsonObject& obj )
{
    static const QString sql( "REPLACE INTO sessionHours (date,marketType,product,sessionHoursType,"
        "start,end) "
            "VALUES (:date,:marketType,:product,:sessionHoursType,"
                ":start,:end)" );

    const QDateTime start( QDateTime::fromString( obj[DB_START].toString(), Qt::ISODate ) );
    const QDateTime end( QDateTime::fromString( obj[DB_END].toString(), Qt::ISODate ) );

    if (( !start.isValid() ) || ( !end.isValid() ) || ( end <= start ))
        return false;

    QSqlQuery query( connection() );
    query.prepare( sql );

    query.bindValue( ":" + DB_DATE, date.toString( Qt::ISODate ) );
    query.bindValue( ":" + DB_MARKET_TYPE, marketType );
    query.bindValue( ":" + DB_PRODUCT, product );
    query.bindValue( ":" + DB_SESSION_HOURS_TYPE, sessionHoursType );

    bindQueryValues( query, obj );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during replace " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AppDatabase::addTreasuryBillRate( const QDateTime& stamp, const QJsonObject& obj )
{
    Q_UNUSED( stamp )

    static const QString sql( "REPLACE INTO riskFreeInterestRates (date,term,source,"
        "rate) "
            "VALUES (:date,:term,:source,"
                ":rate)" );

    const QDateTime date( QDateTime::fromString( obj[DB_DATE].toString(), Qt::ISODate ) );
    const QDateTime maturityDate( QDateTime::fromString( obj[DB_MATURITY_DATE].toString(), Qt::ISODate ) );
    const double rate( obj[DB_ROUND_CLOSE].toDouble() );

    if (( !date.isValid() ) || ( !maturityDate.isValid() ))
        return false;

    const double daysToMaturity( date.daysTo( maturityDate ) );

    QSqlQuery query( connection() );
    query.prepare( sql );

    query.bindValue( ":date", date.date().toString( Qt::ISODate ) );
    query.bindValue( ":term", (daysToMaturity / numDays_) );
    query.bindValue( ":source", DB_TREAS_BILL );
    query.bindValue( ":rate", (rate / 100.0) );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during replace " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AppDatabase::addTreasuryYieldCurveRate( const QDateTime& stamp, const QJsonObject& obj )
{
    Q_UNUSED( stamp )

    static const QString sql( "REPLACE INTO riskFreeInterestRates (date,term,source,"
        "rate) "
            "VALUES (:date,:term,:source,"
                ":rate)" );

    const QDateTime date( QDateTime::fromString( obj[DB_DATE].toString(), Qt::ISODate ) );
    const double months( obj[DB_MONTHS].toInt() );
    const double rate( obj[DB_RATE].toDouble() );

    if (( !date.isValid() ) || ( months <= 0.0 ))
        return false;

    QSqlQuery query( connection() );
    query.prepare( sql );

    query.bindValue( ":date", date.date().toString( Qt::ISODate ) );
    query.bindValue( ":term", (months / 12.0) );
    query.bindValue( ":source", DB_TREAS_YIELD_CURVE );
    query.bindValue( ":rate", (rate / 100.0) );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during replace " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AppDatabase::readSettings()
{
    // read settings
    QVariant v;

    if ( readSetting( "optionTradeCost", v ) )
        optionTradeCost_ = v.toDouble();
    if ( readSetting( "optionCalcMethod", v ) )
        optionCalcMethod_ = v.toString();

    if ( readSetting( "optionChainWatchLists", v ) )
        optionAnalysisWatchLists_ = v.toString();
    if ( readSetting( "optionAnalysisFilter", v ) )
        optionAnalysisFilter_ = v.toString();

    if ( readSetting( "numTradingDays", v ) )
        numTradingDays_ = v.toDouble();
    if ( readSetting( "numDays", v ) )
        numDays_ = v.toDouble();

    if ( readSetting( "palette", v ) )
        palette_ = v.toString();
    if ( readSetting( "paletteHighlight", v ) )
        paletteHighlight_.setNamedColor( v.toString() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AppDatabase::parseAccountBalances( const QDateTime& stamp, const QString& accountId, const QJsonObject& obj )
{
    QStringList balanceKeys;
    balanceKeys.append( DB_INITIAL_BALANCES );
    balanceKeys.append( DB_CURRENT_BALANCES );
    balanceKeys.append( DB_PROJECTED_BALANCES );

    foreach ( const QString& key, balanceKeys )
    {
        const QJsonObject::const_iterator it( obj.constFind( key ) );

        if (( obj.constEnd() != it ) && ( it->isObject() ))
            if ( !addAccountBalances( stamp, accountId, key, it->toObject() ) )
                return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AppDatabase::parseSessionHours( const QDate& date, const QString& marketType, const QString& product, const QJsonObject& obj )
{
    for ( QJsonObject::const_iterator i( obj.constBegin() ); i != obj.constEnd(); ++i )
        if ( i->isObject() )
            if ( !addSessionHours( date, marketType, product, i.key(), i->toObject() ) )
                return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AppDatabase::checkSessionHours( const QDateTime& dt, const QString& marketType, const QString& product, bool *isExtended ) const
{
    static const QString sql( "SELECT sessionHoursType FROM sessionHours WHERE DATETIME(start)<=DATETIME(:dt) AND DATETIME(:dt)<=DATETIME(end) AND marketType=:marketType AND product=:product" );

    QSqlQuery query( connection() );
    query.setForwardOnly( true );
    query.prepare( sql );

    query.bindValue( ":dt", dt.toString( Qt::ISODate ) );
    query.bindValue( ":marketType", marketType );
    query.bindValue( ":product", product );

    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return false;
    }
    else if ( !query.next() )
        return false;

    // check extended hours
    if ( isExtended )
    {
        do
        {
            const QSqlRecord rec( query.record() );

            (*isExtended) |= isExtendedHours( rec.value( "sessionHoursType" ).toString() );

        } while ( query.next() );
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AppDatabase::isExtendedHours( const QString& sessionHoursType ) const
{
    static const QString sql( "SELECT isExtendedHours FROM sessionHoursType WHERE :type=type" );

    QSqlQuery query( connection() );
    query.setForwardOnly( true );
    query.prepare( sql );

    query.bindValue( ":type", sessionHoursType );

    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    if ( query.next() )
    {
        const QSqlRecord rec( query.record() );

        // should only be one record
        if ( query.next() )
            return false;

        return rec.value( "isExtendedHours" ).toBool();
    }

    return false;
}

