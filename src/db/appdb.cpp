/**
 * @file appdb.cpp
 * Application Database.
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
#include "symboldb.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>

static const QString DB_NAME( "appdb.db" );
static const QString DB_VERSION( "7" );

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

    QSqlQuery query( db_ );
    query.prepare( sql );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_WARN << "error during select " << e.type() << " " << qPrintable( e.text() );
    }
    else
    {
        QSqlQueryModel model;
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
        model.setQuery( std::move( query ) );
#else
        model.setQuery( query );
#endif

        for ( int i( 0 ); i < model.rowCount(); ++i )
        {
            const QSqlRecord rec( model.record( i ) );

            result.append(
                QString( "%1 (%2)" )
                    .arg( rec.value( "accountId" ).toString() )
                    .arg( rec.value( "type" ).toString() ) );
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
QString AppDatabase::cusip( const QString& symbol ) const
{
    SymbolDatabase *child( const_cast<_Myt*>( this )->findSymbol( symbol ) );

    if ( child )
        return child->cusip();
    else
    {
        LOG_WARN << "could not find symbol " << qPrintable( symbol );
    }

    return QString();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double AppDatabase::dividendAmount( const QString& symbol, QDate& date, double& frequency ) const
{
    SymbolDatabase *child( const_cast<_Myt*>( this )->findSymbol( symbol ) );

    if ( child )
        return child->dividendAmount( date, frequency );
    else
    {
        LOG_WARN << "could not find symbol " << qPrintable( symbol );
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double AppDatabase::dividendYield( const QString& symbol ) const
{
    SymbolDatabase *child( const_cast<_Myt*>( this )->findSymbol( symbol ) );

    if ( child )
        return child->dividendYield();
    else
    {
        LOG_WARN << "could not find symbol " << qPrintable( symbol );
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QByteArray AppDatabase::filter( const QString& name ) const
{
    static const QString sql( "SELECT * FROM filters WHERE name=:name" );

    QByteArray result;

    // this method is used by background threads, need to open dedicated connection
    QSqlDatabase conn( openDatabaseConnection() );

    QSqlQuery query( conn );
    query.prepare( sql );
    query.bindValue( ":name", name );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_WARN << "error during select " << e.type() << " " << qPrintable( e.text() );
    }
    else
    {
        QSqlQueryModel model;
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
        model.setQuery( std::move( query ) );
#else
        model.setQuery( query );
#endif

        if ( !model.rowCount() )
             LOG_WARN << "no row(s) found";
        else
        {
            const QSqlRecord rec( model.record( 0 ) );

            result.append( rec.value( "value" ).toByteArray() );
        }
    }

    conn.close();

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QStringList AppDatabase::filters() const
{
    static const QString sql( "SELECT DISTINCT name FROM filters" );

    QStringList result;

    QSqlQuery query( db_ );
    query.prepare( sql );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_WARN << "error during select " << e.type() << " " << qPrintable( e.text() );
    }
    else
    {
        QSqlQueryModel model;
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
        model.setQuery( std::move( query ) );
#else
        model.setQuery( query );
#endif

        for ( int i( 0 ); i < model.rowCount(); ++i )
        {
            const QSqlRecord rec( model.record( i ) );

            result.append( rec.value( "name" ).toString() );
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double AppDatabase::historicalVolatility( const QString& symbol, const QDateTime& dt, int depth ) const
{
    SymbolDatabase *child( const_cast<_Myt*>( this )->findSymbol( symbol ) );

    if ( child )
        return child->historicalVolatility( dt, depth );
    else
    {
        LOG_WARN << "could not find symbol " << qPrintable( symbol );
    }

    return 0.0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AppDatabase::isMarketOpen( const QDateTime& dt, const QString& marketType, const QString& product, bool *isExtended ) const
{
    QString sql( "SELECT isOpen, product FROM marketHours WHERE DATE(date)=DATE(:date) AND marketType=:marketType" );

    if ( product.length() )
        sql += " AND product=:product";

    QSqlQuery query( db_ );
    query.prepare( sql );
    query.bindValue( ":date", dt.date().toString( Qt::ISODate ) );
    query.bindValue( ":marketType", marketType );

    if ( product.length() )
        query.bindValue( ":product", product );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_WARN << "error during select " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    QSqlQueryModel model;
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
    model.setQuery( std::move( query ) );
#else
    model.setQuery( query );
#endif

    if ( !model.rowCount() )
        return false;

    // ---- //

    if ( isExtended )
        (*isExtended) = false;

    for ( int i( 0 ); i < model.rowCount(); ++i )
    {
        const QSqlRecord rec( model.record( i ) );

        if ( !rec.value( "isOpen" ).toBool() )
            return false;
        else if ( !checkSessionHours( dt, marketType, rec.value( "product" ).toString(), isExtended ) )
            return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QDateTime AppDatabase::lastQuoteHistoryProcessed( const QString& symbol ) const
{
    SymbolDatabase *child( const_cast<_Myt*>( this )->findSymbol( symbol ) );

    if ( child )
        return child->lastQuoteHistoryProcessed();
    else
    {
        LOG_WARN << "could not find symbol " << qPrintable( symbol );
    }

    return QDateTime();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AppDatabase::marketHoursExist( const QDate& date, const QString& marketType ) const
{
    QString sql( "SELECT isOpen FROM marketHours WHERE DATE(date)=DATE(:date)" );

    if ( marketType.length() )
        sql += " AND marketType=:marketType";

    QSqlQuery query( db_ );
    query.prepare( sql );
    query.bindValue( ":date", date.toString( Qt::ISODate ) );

    if ( marketType.length() )
        query.bindValue( ":marketType", marketType );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_WARN << "error during select " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    return query.first();
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

    QSqlQueryModel model;
    model.setQuery( sql, db_ );

    for ( int i( 0 ); i < model.rowCount(); ++i )
    {
        const QSqlRecord rec( model.record( i ) );

        const QString type( rec.value( "type" ).toString() );

        if ( types.contains( type ) )
            result.append( type );
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSqlDatabase AppDatabase::openDatabaseConnection( const QString& symbol ) const
{
    SymbolDatabase *child( const_cast<_Myt*>( this )->findSymbol( symbol ) );

    if ( child )
        return child->openDatabaseConnection();
    else
    {
        LOG_WARN << "could not find symbol " << qPrintable( symbol );
    }

    return QSqlDatabase();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AppDatabase::quoteHistoryDateRange( const QString& symbol, QDate& start, QDate& end ) const
{
    SymbolDatabase *child( const_cast<_Myt*>( this )->findSymbol( symbol ) );

    if ( child )
        child->quoteHistoryDateRange( start, end );
    else
    {
        LOG_WARN << "could not find symbol " << qPrintable( symbol );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AppDatabase::removeFilter( const QString& name )
{
    static const QString sql( "DELETE FROM filters WHERE name=:name" );

    QSqlQuery query( db_ );
    query.prepare( sql );
    query.bindValue( ":name", name );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_WARN << "error during delete " << e.type() << " " << qPrintable( e.text() );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AppDatabase::removeWatchlist( const QString& name )
{
    static const QString sql( "DELETE FROM %1 WHERE name=:name" );

    QStringList tables;
    tables.append( "watchlist" );
    tables.append( "indices" );

    foreach ( const QString& table, tables )
    {
        QSqlQuery query( db_ );
        query.prepare( sql.arg( table ) );
        query.bindValue( ":name", name );

        // exec sql
        if ( !query.exec() )
        {
            const QSqlError e( query.lastError() );

            LOG_WARN << "error during delete " << e.type() << " " << qPrintable( e.text() );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AppDatabase::removeWidgetState( WidgetType type, const QString& groupName, const QString& name )
{
    static const QString sql( "DELETE FROM %1 WHERE groupName=:groupName AND name=:name" );

    QString table;

    // determine table
    if ( HeaderView == type )
        table = "headerStates";
    else if ( Splitter == type )
        table = "splitterStates";

    if ( table.length() )
    {
        // create new filter
        QSqlQuery query( db_ );
        query.prepare( sql.arg( table ) );
        query.bindValue( ":groupName", groupName );
        query.bindValue( ":name", name );

        // exec sql
        if ( !query.exec() )
        {
            const QSqlError e( query.lastError() );

            LOG_WARN << "error during delete " << e.type() << " " << qPrintable( e.text() );
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

    // this method is used by background threads, need to open dedicated connection
    QSqlDatabase conn( openDatabaseConnection() );

    QSqlQuery query( conn );
    query.prepare( sql );
    query.bindValue( ":dateMin", dateMin.toString( Qt::ISODate ) );
    query.bindValue( ":dateMax", dateMax.toString( Qt::ISODate ) );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_WARN << "error during select " << e.type() << " " << qPrintable( e.text() );
    }
    else
    {
        QSqlQueryModel model;
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
        model.setQuery( std::move( query ) );
#else
        model.setQuery( query );
#endif

        for ( int i( 0 ); i < model.rowCount(); ++i )
        {
            const QSqlRecord rec( model.record( i ) );

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

    conn.close();

    return rate;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AppDatabase::setConfigs( const QJsonObject& value )
{
    // write each value
    for ( QJsonObject::const_iterator i( value.constBegin() ); i != value.constEnd(); ++i )
    {
        const QString v( i->toString() );

        if ( !writeSetting( i.key(), v ) )
            LOG_WARN << "failed to write setting " << qPrintable( i.key() ) << " '" << qPrintable( v ) << "'";
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

    // remove old filter
    removeFilter( name );

    // create new filter
    QSqlQuery query( db_ );
    query.prepare( sql );
    query.bindValue( ":name", name );
    query.bindValue( ":value", value );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_WARN << "error during insert " << e.type() << " " << qPrintable( e.text() );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AppDatabase::setWatchlist( const QString& name, const QStringList& symbols )
{
    static const QString sql( "INSERT INTO watchlist (name,symbol) "
            "VALUES (:name,:symbol)" );

    // remove old list
    removeWatchlist( name );

    // create new list
    foreach ( const QString& symbol, symbols )
    {
        QSqlQuery query( db_ );
        query.prepare( sql );
        query.bindValue( ":name", name );
        query.bindValue( ":symbol", symbol );

        // exec sql
        if ( !query.exec() )
        {
            const QSqlError e( query.lastError() );

            LOG_WARN << "error during insert " << e.type() << " " << qPrintable( e.text() );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AppDatabase::setWidgetState( WidgetType type, const QString& groupName, const QString& name, const QByteArray& state )
{
    static const QString sql( "REPLACE INTO %1 (groupName,name,state) "
            "VALUES (:groupName,:name,:state)" );

    QString table;

    // determine table
    if ( HeaderView == type )
        table = "headerStates";
    else if ( Splitter == type )
        table = "splitterStates";

    if ( table.length() )
    {
        // create new filter
        QSqlQuery query( db_ );
        query.prepare( sql.arg( table ) );
        query.bindValue( ":groupName", groupName );
        query.bindValue( ":name", name );
        query.bindValue( ":state", state );

        // exec sql
        if ( !query.exec() )
        {
            const QSqlError e( query.lastError() );

            LOG_WARN << "error during replace " << e.type() << " " << qPrintable( e.text() );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AppDatabase::treasuryYieldCurveDateRange( QDate& start, QDate& end ) const
{
    const QString sql( "SELECT date FROM riskFreeInterestRates WHERE "
        "source='" + DB_TREAS_YIELD_CURVE + "' ORDER BY DATE(date) %1 LIMIT 5" );

    QSqlQueryModel modelEnd;
    modelEnd.setQuery( sql.arg( "DESC" ), db_ );

    for ( int i( 0 ); i < modelEnd.rowCount(); ++i )
    {
        const QSqlRecord rec( modelEnd.record( i ) );

        end = QDate::fromString( rec.value( "date" ).toString(), Qt::ISODate );
        break;
    }

    QSqlQueryModel modelStart;
    modelStart.setQuery( sql.arg( "ASC" ), db_ );

    for ( int i( 0 ); i < modelStart.rowCount(); ++i )
    {
        const QSqlRecord rec( modelStart.record( i ) );

        start = QDate::fromString( rec.value( "date" ).toString(), Qt::ISODate );
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QStringList AppDatabase::watchlist( const QString& name ) const
{
    static const QString sql( "SELECT * FROM watchlist WHERE name=:name" );

    QStringList result;

    QSqlQuery query( db_ );
    query.prepare( sql );
    query.bindValue( ":name", name );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_WARN << "error during select " << e.type() << " " << qPrintable( e.text() );
    }
    else
    {
        QSqlQueryModel model;
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
        model.setQuery( std::move( query ) );
#else
        model.setQuery( query );
#endif

        for ( int i( 0 ); i < model.rowCount(); ++i )
        {
            const QSqlRecord rec( model.record( i ) );

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

    QSqlQuery query( db_ );
    query.prepare( sql.arg( "watchlist" ) );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_WARN << "error during select " << e.type() << " " << qPrintable( e.text() );
    }
    else
    {
        QSqlQueryModel model;
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
        model.setQuery( std::move( query ) );
#else
        model.setQuery( query );
#endif

        for ( int i( 0 ); i < model.rowCount(); ++i )
        {
            const QSqlRecord rec( model.record( i ) );

            result.append( rec.value( "name" ).toString() );
        }
    }

    // remove each index from list
    if ( !includeIndices )
    {
        QSqlQuery indicesQuery( db_ );
        indicesQuery.prepare( sql.arg( "indices" ) );

        // exec sql
        if ( !indicesQuery.exec() )
        {
            const QSqlError e( indicesQuery.lastError() );

            LOG_WARN << "error during select " << e.type() << " " << qPrintable( e.text() );
        }
        else
        {
            QSqlQueryModel model;
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
            model.setQuery( std::move( indicesQuery ) );
#else
            model.setQuery( indicesQuery );
#endif

            for ( int i( 0 ); i < model.rowCount(); ++i )
            {
                const QSqlRecord rec( model.record( i ) );
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

    QString table;
    QStringList result;

    // determine table
    if ( HeaderView == type )
        table = "headerStates";
    else if ( Splitter == type )
        table = "splitterStates";

    if ( table.length() )
    {
        QSqlQuery query( db_ );
        query.prepare( sql.arg( table ) );

        // exec sql
        if ( !query.exec() )
        {
            const QSqlError e( query.lastError() );

            LOG_WARN << "error during select " << e.type() << " " << qPrintable( e.text() );
        }
        else
        {
            QSqlQueryModel model;
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
            model.setQuery( std::move( query ) );
#else
            model.setQuery( query );
#endif

            for ( int i( 0 ); i < model.rowCount(); ++i )
            {
                const QSqlRecord rec( model.record( i ) );

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

    QString table;
    QByteArray result;

    // determine table
    if ( HeaderView == type )
        table = "headerStates";
    else if ( Splitter == type )
        table = "splitterStates";

    if ( table.length() )
    {
        QSqlQuery query( db_ );
        query.prepare( sql.arg( table ) );
        query.bindValue( ":groupName", groupName );
        query.bindValue( ":name", name );

        // exec sql
        if ( !query.exec() )
        {
            const QSqlError e( query.lastError() );

            LOG_WARN << "error during select " << e.type() << " " << qPrintable( e.text() );
        }
        else
        {
            QSqlQueryModel model;
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
            model.setQuery( std::move( query ) );
#else
            model.setQuery( query );
#endif

            if ( !model.rowCount() )
                 LOG_WARN << "no row(s) found";
            else
            {
                const QSqlRecord rec( model.record( 0 ) );

                result = rec.value( "state" ).toByteArray();
            }
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QStringList AppDatabase::widgetStates( WidgetType type, const QString& groupName ) const
{
    static const QString sql( "SELECT name FROM %1 WHERE groupName=:groupName AND name NOT LIKE '[[%]]' ORDER BY name ASC" );

    QString table;
    QStringList result;

    // determine table
    if ( HeaderView == type )
        table = "headerStates";
    else if ( Splitter == type )
        table = "splitterStates";

    if ( table.length() )
    {
        QSqlQuery query( db_ );
        query.prepare( sql.arg( table ) );
        query.bindValue( ":groupName", groupName );

        // exec sql
        if ( !query.exec() )
        {
            const QSqlError e( query.lastError() );

            LOG_WARN << "error during select " << e.type() << " " << qPrintable( e.text() );
        }
        else
        {
            QSqlQueryModel model;
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
            model.setQuery( std::move( query ) );
#else
            model.setQuery( query );
#endif

            for ( int i( 0 ); i < model.rowCount(); ++i )
            {
                const QSqlRecord rec( model.record( i ) );

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
bool AppDatabase::processData( const QJsonObject& obj )
{
    const QDateTime now( currentDateTime() );

    bool accountsProcessed( false );
    bool instrumentsProcessed( false );
    bool marketHoursProcessed( false );
    bool quoteHistoryProcessed( false );
    bool quotesProcessed( false );
    bool optionChainProcessed( false );
    bool treasBillRatesProcessed( false );
    bool treasYieldCurveRatesProcessed( false );

    QString quoteHistorySymbol;
    QStringList quoteSymbols;
    QList<QDate> optionChainExpiryDates;
    QString optionChainSymbol;

    // START DB TRANSACION
    if ( !db_.transaction() )
    {
        LOG_WARN << "failed to start transaction";
        return false;
    }

    bool result( true );

    // iterate accounts
    const QJsonObject::const_iterator accounts( obj.constFind( DB_ACCOUNTS ) );

    if (( obj.constEnd() != accounts ) && ( accounts->isArray() ))
    {
        foreach ( const QJsonValue& accountVal, accounts->toArray() )
           if ( accountVal.isObject() )
              result &= addAccount( now, accountVal.toObject() );

        accountsProcessed = true;
    }

    // iterate instruments
    const QJsonObject::const_iterator instruments( obj.constFind( DB_INSTRUMENTS ) );

    if (( obj.constEnd() != instruments ) && ( instruments->isArray() ))
    {
        foreach ( const QJsonValue& instrumentVal, instruments->toArray() )
           if ( instrumentVal.isObject() )
           {
               const QJsonObject instrument( instrumentVal.toObject() );

               const QString symbol( instrument[DB_SYMBOL].toString() );

               SymbolDatabase *child( findSymbol( symbol ) );

               if ( child )
                   result &= child->processInstrument( now, instrument );
           }

        instrumentsProcessed = true;
    }

    // iterate market hours
    const QJsonObject::const_iterator marketHours( obj.constFind( DB_MARKET_HOURS ) );

    if (( obj.constEnd() != marketHours ) && ( marketHours->isArray() ))
    {
        foreach ( const QJsonValue& hoursVal, marketHours->toArray() )
           if ( hoursVal.isObject() )
              result &= addMarketHours( hoursVal.toObject() );

        marketHoursProcessed = true;
    }

    // process quote history
    const QJsonObject::const_iterator quoteHistoryIt( obj.constFind( DB_QUOTE_HISTORY ) );

    if (( obj.constEnd() != quoteHistoryIt ) && ( quoteHistoryIt->isObject() ))
    {
        const QJsonObject quoteHistory( quoteHistoryIt->toObject() );

        const QString symbol( quoteHistory[DB_SYMBOL].toString() );

        SymbolDatabase *child( findSymbol( symbol ) );

        if ( child )
            result &= child->processQuoteHistory( quoteHistory );

        quoteHistoryProcessed = true;
        quoteHistorySymbol = symbol;
    }

    // process quotes
    const QJsonObject::const_iterator quotes( obj.constFind( DB_QUOTES ) );

    if (( obj.constEnd() != quotes ) && ( quotes->isArray() ))
    {
        foreach ( const QJsonValue& quoteVal, quotes->toArray() )
            if ( quoteVal.isObject() )
            {
                const QJsonObject quote( quoteVal.toObject() );

                QString symbol( quote[DB_SYMBOL].toString() );

                // check for option
                const QString underlying( quote[DB_UNDERLYING].toString() );

                if ( underlying.length() )
                {
                    LOG_DEBUG << "processing option quote " << qPrintable( underlying );
                    symbol = underlying;
                }

                SymbolDatabase *child( findSymbol( symbol ) );

                if ( child )
                    result &= child->processQuote( now, quote );

                quotesProcessed = true;
                quoteSymbols.append( symbol );
            }
    }

    // process option chain
    const QJsonObject::const_iterator optionChainIt( obj.constFind( DB_OPTION_CHAIN ) );

    if (( obj.constEnd() != optionChainIt ) && ( optionChainIt->isObject() ))
    {
        const QJsonObject optionChain( optionChainIt->toObject() );

        const QString symbol( optionChain[DB_UNDERLYING].toString() );

        SymbolDatabase *child( findSymbol( symbol ) );

        if ( child )
            result &= child->processOptionChain( now, optionChain, optionChainExpiryDates );

        optionChainProcessed = true;
        optionChainSymbol = symbol;
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

            treasBillRatesProcessed = true;
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

            treasYieldCurveRatesProcessed = true;
        }
    }

    // COMMIT DB TRANSACION

    // commit to database
    if (( result ) && ( !(result = db_.commit()) ))
        LOG_WARN << "commit failed";

    if (( !result ) && ( !db_.rollback() ))
        LOG_ERROR << "rollback failed";

    if ( !result )
        return false;

    // EMIT SIGNALS

    if ( accountsProcessed )
        emit accountsChanged();

    if ( instrumentsProcessed )
        emit instrumentsChanged();

    if ( marketHoursProcessed )
        emit marketHoursChanged();

    if ( quoteHistoryProcessed )
        emit quoteHistoryChanged( quoteHistorySymbol );

    if ( quotesProcessed )
        emit quotesChanged( quoteSymbols );

    if ( optionChainProcessed )
        emit optionChainChanged( optionChainSymbol, optionChainExpiryDates );

    if ( treasBillRatesProcessed )
        emit treasuryBillRatesChanged();

    if ( treasYieldCurveRatesProcessed )
        emit treasuryYieldCurveRatesChanged();

    return true;
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
    static const QString sql( "REPLACE INTO accounts (accountId,"
        "type,isClosingOnlyRestricted,isDayTrader,roundTrips) "
            "VALUES (:accountId,"
                ":type,:isClosingOnlyRestricted,:isDayTrader,:roundTrips)" );

    const QString accountId( obj[DB_ACCOUNT_ID].toString() );

    if ( accountId.isEmpty() )
        return false;

    QSqlQuery query( db_ );
    query.prepare( sql );

    bindQueryValues( query, obj );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_WARN << "error during replace " << e.type() << " " << qPrintable( e.text() );
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

    QSqlQuery query( db_ );
    query.prepare( sql );
    query.bindValue( ":" + DB_STAMP, stamp.toString( Qt::ISODateWithMs ) );
    query.bindValue( ":" + DB_ACCOUNT_ID, accountId );
    query.bindValue( ":" + DB_TYPE, type );

    bindQueryValues( query, obj );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_WARN << "error during replace " << e.type() << " " << qPrintable( e.text() );
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
    QSqlQuery query( db_ );
    query.prepare( sql );

    bindQueryValues( query, obj );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_WARN << "error during replace " << e.type() << " " << qPrintable( e.text() );
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
        QSqlQuery query( db_ );
        query.prepare( sql );
        query.addBindValue( type );
        query.addBindValue( description );

        // exec sql
        if ( !query.exec() )
        {
            const QSqlError e( query.lastError() );

            LOG_WARN << "error during replace " << e.type() << " " << qPrintable( e.text() );
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

    QSqlQuery query( db_ );
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

        LOG_WARN << "error during replace " << e.type() << " " << qPrintable( e.text() );
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

    QSqlQuery query( db_ );
    query.prepare( sql );
    query.bindValue( ":date", date.date().toString( Qt::ISODate ) );
    query.bindValue( ":term", (daysToMaturity / numDays_) );
    query.bindValue( ":source", DB_TREAS_BILL );
    query.bindValue( ":rate", (rate / 100.0) );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_WARN << "error during replace " << e.type() << " " << qPrintable( e.text() );
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

    QSqlQuery query( db_ );
    query.prepare( sql );
    query.bindValue( ":date", date.date().toString( Qt::ISODate ) );
    query.bindValue( ":term", (months / 12.0) );
    query.bindValue( ":source", DB_TREAS_YIELD_CURVE );
    query.bindValue( ":rate", (rate / 100.0) );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_WARN << "error during replace " << e.type() << " " << qPrintable( e.text() );
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
SymbolDatabase *AppDatabase::findSymbol( const QString& symbol )
{
    if ( symbol.isEmpty() )
        return nullptr;

    SymbolDatabase *child( findChild<SymbolDatabase*>( symbol ) );

    if ( !child )
    {
        child = new SymbolDatabase( symbol, this );

        if ( !child->isReady() )
        {
            LOG_WARN << "failed to create symbol db " << qPrintable( symbol );
            return nullptr;
        }
    }

    return child;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AppDatabase::checkSessionHours( const QDateTime& dt, const QString& marketType, const QString& product, bool *isExtended ) const
{
    static const QString sql( "SELECT sessionHoursType FROM sessionHours WHERE DATETIME(start)<=DATETIME(:dt) AND DATETIME(:dt)<=DATETIME(end) AND marketType=:marketType AND product=:product" );

    QSqlQuery query( db_ );
    query.prepare( sql );
    query.bindValue( ":dt", dt.toString( Qt::ISODate ) );
    query.bindValue( ":marketType", marketType );
    query.bindValue( ":product", product );

    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_WARN << "error during select " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    QSqlQueryModel model;
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
    model.setQuery( std::move( query ) );
#else
    model.setQuery( query );
#endif

    if ( !model.rowCount() )
        return false;

    // check extended hours
    if ( isExtended )
    {
        for ( int i( 0 ); i < model.rowCount(); ++i )
        {
            const QSqlRecord rec( model.record( i ) );

            (*isExtended) |= isExtendedHours( rec.value( "sessionHoursType" ).toString() );
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AppDatabase::isExtendedHours( const QString& sessionHoursType ) const
{
    static const QString sql( "SELECT isExtendedHours FROM sessionHoursType WHERE :type=type" );

    QSqlQuery query( db_ );
    query.prepare( sql );
    query.bindValue( ":type", sessionHoursType );

    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_WARN << "error during select " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    QSqlQueryModel model;
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
    model.setQuery( std::move( query ) );
#else
    model.setQuery( query );
#endif

    if ( 1 != model.rowCount() )
        return false;

    const QSqlRecord rec( model.record( 0 ) );

    return rec.value( "isExtendedHours" ).toBool();
}

