/**
 * @file symboldb.cpp
 * Symbol History Database.
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

#include "../util/stats.h"

#include <cmath>

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlTableModel>

static const QString DB_NAME( "%1.db" );
static const QString DB_VERSION( "1" );

static const QString CALL( "CALL" );
static const QString PUT( "PUT" );

static const QString LAST_QUOTE_HISTORY( "lastQuoteHistory" );

///////////////////////////////////////////////////////////////////////////////////////////////////
SymbolDatabase::SymbolDatabase( const QString& symbol, QObject *parent ) :
    _Mybase( DB_NAME.arg( symbol ), DB_VERSION, parent ),
    symbol_( symbol )
{
    // set object name
    setObjectName( symbol );

    // open database
    if ( open() )
        writeSetting( "symbol", symbol );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SymbolDatabase::~SymbolDatabase()
{
    db_.close();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double SymbolDatabase::historicalVolatility( const QDateTime& dt, int depth ) const
{
    static const QString sql( "SELECT date,depth,volatility FROM historicalVolatility "
        "WHERE DATE(date)=DATE(:date) "
        "ORDER BY depth ASC" );

    QSqlQuery query( db_ );
    query.prepare( sql );

    query.bindValue( ":" + DB_DATE, dt.date().toString( Qt::ISODate ) );

     if ( !query.exec() )
     {
         const QSqlError e( query.lastError() );

         LOG_WARN << "error during insert " << e.type() << " " << qPrintable( e.text() );
     }
     else
     {
         double min( 0.0 );
         int min_depth( 0 );

         double max( 0.0 );
         int max_depth( 0 );

         // find bounding volatility
         QSqlQueryModel model;
         model.setQuery( query );

         for ( int row( 0 ); row < model.rowCount(); ++row )
         {
             const QSqlRecord rec( model.record( row ) );

             const double v( rec.value( "volatility" ).toDouble() );
             const int v_depth( rec.value( "depth" ).toInt() );

             if ( v_depth == depth )
                 return v;
             else if ( v_depth < depth )
             {
                 min = v;
                 min_depth = v_depth;
             }
             else
             {
                 max = v;
                 max_depth = v_depth;
                 break;
             }
         }

        // requested depth BELOW table values
        if (( !min_depth ) && ( max_depth ))
            return max;

        // requested depth ABOVE table values
        if (( min_depth ) && ( !max_depth ))
            return min;

        // interpolate
        return min + ((double)(depth - min_depth) / (double)(max_depth - min_depth)) * (max - min);
     }

    return 0.0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QDateTime SymbolDatabase::lastQuoteHistoryProcessed() const
{
    QDateTime stamp;

    QVariant v;

    if ( readSetting( LAST_QUOTE_HISTORY, v ) )
        stamp = QDateTime::fromString( v.toString(), Qt::ISODateWithMs );

    return stamp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::quoteHistoryDateRange( QDate& start, QDate& end ) const
{
    const QString sql( "SELECT date FROM quoteHistory ORDER BY DATE(date) %1 LIMIT 5" );

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
bool SymbolDatabase::processOptionChain( const QDateTime& stamp, const QJsonObject& obj, QList<QDate>& expiryDates )
{
    if ( symbol() != obj[DB_UNDERLYING].toString() )
        return false;

    if ( !db_.transaction() )
    {
        LOG_WARN << "failed to start transaction";
        return false;
    }

    LOG_DEBUG << "process option chain for " << qPrintable( symbol() );

    bool result( true );

    // add option chain
    result &= addOptionChain( stamp, obj, expiryDates );

    // add quotes (optional)
    const QJsonObject::const_iterator quotes( obj.constFind( DB_QUOTES ) );

    if (( obj.constEnd() != quotes ) && ( quotes->isArray() ))
    {
        foreach ( const QJsonValue& v, quotes->toArray() )
            if ( v.isObject() )
                result &= addQuote( v.toObject() );
    }

    // commit to database
    if (( result ) && ( !(result = db_.commit()) ))
        LOG_WARN << "commit failed";

    if (( !result ) && ( !db_.rollback() ))
        LOG_ERROR << "rollback failed";

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::processQuote( const QDateTime& stamp, const QJsonObject& obj )
{
    Q_UNUSED( stamp )

    if ( !db_.transaction() )
    {
        LOG_WARN << "failed to start transaction";
        return false;
    }

    LOG_DEBUG << "process quote for " << qPrintable( symbol() );

    bool result( true );

    // check for option
    if ( obj.contains( DB_UNDERLYING ) )
        result &= addOption( obj );
    else
        result &= addQuote( obj );

    // commit to database
    if (( result ) && ( !(result = db_.commit()) ))
        LOG_WARN << "commit failed";

    if (( !result ) && ( !db_.rollback() ))
        LOG_ERROR << "rollback failed";

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::processQuoteHistory( const QJsonObject& obj )
{
    if ( symbol() != obj[DB_SYMBOL].toString() )
        return false;

    if ( !db_.transaction() )
    {
        LOG_WARN << "failed to start transaction";
        return false;
    }

    LOG_DEBUG << "process quote history for " << qPrintable( symbol() );

    bool result( true );

    // add history
    QJsonObject::const_iterator history( obj.constFind( DB_HISTORY ) );

    if (( obj.constEnd() != history ) && ( history->isArray() ))
    {
        foreach ( const QJsonValue& v, history->toArray() )
            if ( v.isObject() )
                result &= addQuoteHistory( v.toObject() );

        // calculate historical volatility
        calcHistoricalVolatility();
    }

    // commit to database
    if (( result ) && ( !(result = db_.commit()) ))
        LOG_WARN << "commit failed";

    if (( !result ) && ( !db_.rollback() ))
        LOG_ERROR << "rollback failed";

    // save last quote history
    if ( result )
        writeSetting( LAST_QUOTE_HISTORY, AppDatabase::instance()->currentDateTime().toString( Qt::ISODateWithMs ) );

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QStringList SymbolDatabase::createFiles() const
{
    QStringList files;
    files.append( ":/db/createdb_symbol.sql" );
    files.append( ":/db/default_symbol.sql" );

    return files;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QStringList SymbolDatabase::upgradeFiles( const QString& fromStr, const QString& toStr ) const
{
    int from( fromStr.toInt() );
    int to( toStr.toInt() );

    LOG_INFO << "upgrade database from " << from << " " << to;

    // upgrade each version step-by-step
    QStringList files;

    while ( from < to )
        files.append( QString( ":/db/version%0_symbol.sql" ).arg( from++ ) );

    return files;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::addOption( const QJsonObject& obj )
{
    static const QString sql( "REPLACE INTO options (stamp,symbol,"
        "underlying,type,strikePrice,description,bidAskSize,bidPrice,bidSize,askPrice,askSize,lastPrice,"
        "lastSize,breakEvenPrice,intrinsicValue,openPrice,highPrice,lowPrice,closePrice,change,percentChange,totalVolume,"
        "quoteTime,tradeTime,mark,markChange,markPercentChange,exchangeName,volatility,delta,gamma,theta,"
        "vega,rho,timeValue,openInterest,isInTheMoney,theoreticalOptionValue,theoreticalVolatility,isMini,isNonStandard,isIndex,"
        "isWeekly,isQuarterly,expirationDate,expirationType,daysToExpiration,lastTradingDay,multiplier,settlementType,deliverableNote) "
            "VALUES (:stamp,:symbol,"
                ":underlying,:type,:strikePrice,:description,:bidAskSize,:bidPrice,:bidSize,:askPrice,:askSize,:lastPrice,"
                ":lastSize,:breakEvenPrice,:intrinsicValue,:openPrice,:highPrice,:lowPrice,:closePrice,:change,:percentChange,:totalVolume,"
                ":quoteTime,:tradeTime,:mark,:markChange,:markPercentChange,:exchangeName,:volatility,:delta,:gamma,:theta,"
                ":vega,:rho,:timeValue,:openInterest,:isInTheMoney,:theoreticalOptionValue,:theoreticalVolatility,:isMini,:isNonStandard,:isIndex,"
                ":isWeekly,:isQuarterly,:expirationDate,:expirationType,:daysToExpiration,:lastTradingDay,:multiplier,:settlementType,:deliverableNote) " );

    const double strikePrice( obj[DB_STRIKE_PRICE].toDouble() );
    const QString type( obj[DB_TYPE].toString() );

    QSqlQuery query( db_ );
    query.prepare( sql );
    query.bindValue( ":" + DB_UNDERLYING, symbol() );

    bindQueryValues( query, obj );

    // calculate break even price
    const QJsonObject::const_iterator theoOptionValueIt( obj.constFind( DB_THEO_OPTION_VALUE ) );
    const QJsonObject::const_iterator multiplierIt( obj.constFind( DB_MULTIPLIER ) );

    if (( obj.constEnd() != theoOptionValueIt ) && ( obj.constEnd() != multiplierIt ))
    {
        const double theoValue( theoOptionValueIt->toDouble() );
        const int multiplier( multiplierIt->toInt() );

        const double premium( (multiplier * theoValue) - AppDatabase::instance()->optionTradeCost() );

        double breakEven( strikePrice );

        if ( CALL == type )
            breakEven += premium / multiplier;
        else if ( PUT == type )
            breakEven -= premium / multiplier;

        query.bindValue( ":" + DB_BREAK_EVEN_PRICE, breakEven );
    }

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_WARN << "error during insert " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::addOptionChain( const QDateTime& stamp, const QJsonObject& obj, QList<QDate>& expiryDates )
{
    static const QString sql( "INSERT INTO optionChains (stamp,underlying,"
        "underlyingPrice,interestRate,isDelayed,isIndex,numberOfContracts,volatility) "
            "VALUES (:stamp,:underlying,"
                ":underlyingPrice,:interestRate,:isDelayed,:isIndex,:numberOfContracts,:volatility) " );

    QSqlQuery query( db_ );
    query.prepare( sql );
    query.bindValue( ":stamp", stamp.toString( Qt::ISODateWithMs ) );

    bindQueryValues( query, obj );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_WARN << "error during insert " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    // iterate options
    QJsonObject::const_iterator options( obj.constFind( DB_OPTIONS ) );

    if (( obj.constEnd() != options ) && ( options->isArray() ))
    {
        foreach ( const QJsonValue& v, options->toArray() )
            if ( v.isObject() )
            {
                const QJsonObject option( v.toObject() );

                const QJsonObject::const_iterator optionStampIt( option.constFind( DB_STAMP ) );
                const QJsonObject::const_iterator optionSymbolIt( option.constFind( DB_SYMBOL ) );

                const QJsonObject::const_iterator expiryDateIt( option.constFind( DB_EXPIRY_DATE ) );
                const QJsonObject::const_iterator strikePriceIt( option.constFind( DB_STRIKE_PRICE ) );
                const QJsonObject::const_iterator typeIt( option.constFind( DB_TYPE ) );

                if (( option.constEnd() == optionStampIt ) ||
                    ( option.constEnd() == optionSymbolIt ) ||
                    ( option.constEnd() == expiryDateIt ) ||
                    ( option.constEnd() == strikePriceIt ) ||
                    ( option.constEnd() == typeIt ))
                {
                    LOG_WARN << "bad or missing value(s)";
                    return false;
                }

                const QDateTime expiryDate( QDateTime::fromString( expiryDateIt->toString(), Qt::ISODate ) );

                // add option
                if ( !addOption( option ) )
                    return false;

                // add strike price to chain
                else if ( !addOptionChainStrikePrice( stamp, optionStampIt->toString(), optionSymbolIt->toString(), typeIt->toString(), expiryDate.date().toString( Qt::ISODate ), strikePriceIt->toDouble() ) )
                    return false;

                // track expiry dates for caller
                if ( !expiryDates.contains( expiryDate.date() ) )
                    expiryDates.append( expiryDate.date() );
            }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::addOptionChainStrikePrice( const QDateTime& stamp, const QString& optionStamp, const QString& optionSymbol, const QString& type, const QString& expiryDate, double strikePrice )
{
    static const QString sql( "INSERT INTO optionChainStrikePrices (stamp,underlying,expirationDate,strikePrice,"
        "%1Stamp,%1Symbol) "
            "VALUES (:stamp,:underlying,:expirationDate,:strikePrice,"
                ":optionStamp,:optionSymbol) "
            "ON CONFLICT (stamp,underlying,expirationDate,strikePrice) DO UPDATE SET "
                "%1Stamp=:optionStamp,%1Symbol=:optionSymbol " );

    QSqlQuery query( db_ );
    query.prepare( sql.arg( type ) );
    query.bindValue( ":stamp", stamp.toString( Qt::ISODateWithMs ) );
    query.bindValue( ":underlying", symbol() );
    query.bindValue( ":expirationDate", expiryDate );
    query.bindValue( ":strikePrice", strikePrice );

    query.bindValue( ":optionStamp", optionStamp );
    query.bindValue( ":optionSymbol", optionSymbol );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_WARN << "error during insert " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::addQuote( const QJsonObject& obj )
{
    static const QString sql( "REPLACE INTO quotes (stamp,symbol,"
        "description,assetMainType,assetSubType,assetType,cusip,bidAskSize,bidPrice,bidSize,bidId,bidTick,"
        "askPrice,askSize,askId,lastPrice,lastSize,lastId,openPrice,highPrice,lowPrice,closePrice,"
        "change,percentChange,totalVolume,quoteTime,tradeTime,mark,markChange,markPercentChange,fiftyTwoWeekHigh,fiftyTwoWeekLow,"
        "exchange,exchangeName,isMarginable,isShortable,isDelayed,volatility,digits,nAV,peRatio,impliedYield,"
        "divAmount,divYield,divDate,divFrequency,securityStatus,regularMarketLastPrice,regularMarketLastSize,regularMarketChange,regularMarketPercentChange,regularMarketTradeTime,tick,"
        "tickAmount,product,tradingHours,isTradable,marketMaker) "
            "VALUES (:stamp,:symbol,"
                ":description,:assetMainType,:assetSubType,:assetType,:cusip,:bidAskSize,:bidPrice,:bidSize,:bidId,:bidTick,"
                ":askPrice,:askSize,:askId,:lastPrice,:lastSize,:lastId,:openPrice,:highPrice,:lowPrice,:closePrice,"
                ":change,:percentChange,:totalVolume,:quoteTime,:tradeTime,:mark,:markChange,:markPercentChange,:fiftyTwoWeekHigh,:fiftyTwoWeekLow,"
                ":exchange,:exchangeName,:isMarginable,:isShortable,:isDelayed,:volatility,:digits,:nAV,:peRatio,:impliedYield,"
                ":divAmount,:divYield,:divDate,:divFrequency,:securityStatus,:regularMarketLastPrice,:regularMarketLastSize,:regularMarketChange,:regularMarketPercentChange,:regularMarketTradeTime,:tick,"
                ":tickAmount,:product,:tradingHours,:isTradable,:marketMaker) " );

    QSqlQuery query( db_ );
    query.prepare( sql );

    // update values
    bindQueryValues( query, obj );

    // compute dividend frequency
    if ( obj.contains( DB_DIV_DATE ) )
    {
        const QJsonValue newValue( obj[DB_DIV_DATE] );

        QVariant oldValue;

        if (( readSetting( DB_DIV_DATE, oldValue ) ) && ( oldValue.isValid() ))
        {
            const QDateTime oldDate( QDateTime::fromString( oldValue.toString(), Qt::ISODateWithMs ) );
            const QDateTime newDate( QDateTime::fromString( newValue.toString(), Qt::ISODateWithMs ) );

            if (( oldDate.isValid() ) && ( newDate.isValid() ) && ( oldDate < newDate ))
            {
                const int delta( oldDate.daysTo( newDate ) );

                QString freq;

                if ( 350 < delta )
                    freq = "Y";
                else if ( 175 < delta )
                    freq = "B";
                else if ( delta < 45 )
                    freq = "M";
                else
                    freq = "Q";

                writeSetting( DB_DIV_FREQUENCY, freq );
            }
        }
    }

    // fill in missing values
    updateDefaultValue( query, obj, DB_ASSET_MAIN_TYPE );
    updateDefaultValue( query, obj, DB_ASSET_SUB_TYPE );
    updateDefaultValue( query, obj, DB_ASSET_TYPE );
    updateDefaultValue( query, obj, DB_CUSIP );

    updateDefaultValue( query, obj, DB_DIV_AMOUNT );
    updateDefaultValue( query, obj, DB_DIV_YIELD );
    updateDefaultValue( query, obj, DB_DIV_DATE );
    updateDefaultValue( query, obj, DB_DIV_FREQUENCY );

    updateDefaultValue( query, obj, DB_SECURITY_STATUS );

    updateDefaultValue( query, obj, DB_NAV );
    updateDefaultValue( query, obj, DB_PE_RATIO );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_WARN << "error during insert " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::addQuoteHistory( const QJsonObject& obj )
{
    static const QString sql( "INSERT INTO quoteHistory (date,symbol,"
        "openPrice,highPrice,lowPrice,closePrice,totalVolume) "
            "VALUES (:date,:symbol,"
                ":openPrice,:highPrice,:lowPrice,:closePrice,:totalVolume) "
            "ON CONFLICT(date,symbol) DO UPDATE SET "
                "openPrice=:openPrice, "
                "highPrice=:highPrice, "
                "lowPrice=:lowPrice, "
                "closePrice=:closePrice, "
                "totalVolume=:totalVolume " );

    QSqlQuery query( db_ );
    query.prepare( sql );
    query.bindValue( ":" + DB_SYMBOL, symbol() );

    bindQueryValues( query, obj );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_WARN << "error during insert " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::calcHistoricalVolatility()
{
    const double annualized( sqrt( AppDatabase::instance()->numTradingDays() ) );

    QVariantList dates;
    QVariantList symbols;
    QVariantList depths;
    QVariantList hv;

    QVector<int> hvd;
    hvd.append( 5 );        // 5d
    hvd.append( 10 );       // 10d
    hvd.append( 20 );       // 20d
    hvd.append( 30 );       // 1m
    hvd.append( 60 );       // 2m
    hvd.append( 90 );       // 3m
    hvd.append( 120 );      // 4m
    hvd.append( 240 );      // 8m
    hvd.append( 480 );      // 16m

    // ---- //

    QSqlTableModel model( nullptr, db_ );
    model.setEditStrategy( QSqlTableModel::OnManualSubmit );
    model.setTable( "quoteHistory" );
    model.setSort( model.fieldIndex( DB_STAMP ), Qt::AscendingOrder );
    model.select();

    // fetch all rows
    while ( model.canFetchMore() )
        model.fetchMore();

    // force update of last N rows
    const int forced( model.rowCount() - 5 );

    // calculate returns
    QVector<double> r;

    double prevClose( 0.0 );

    for ( int i( 0 ); i < model.rowCount(); ++i )
    {
        QSqlRecord rec( model.record( i ) );

        const double close( rec.value( DB_CLOSE_PRICE ).toDouble() );

        if (( i ) && ( 0.0 < close ) && ( 0.0 < prevClose ))
        {
            // calc log of interday return
            r.push_back( log( close / prevClose ) );

            if ( hvd.last() < r.length() )
                r.pop_front();

            // lookup depth of this record
            int depth( 0 );

            if ( !rec.isNull( DB_DEPTH ) )
                depth = rec.value( DB_DEPTH ).toInt();

            // calc historical volatility for each depth
            foreach ( int d, hvd )
            {
                if ( r.length() < d )
                    break;
                else if (( i < forced ) && ( d <= depth ))
                    continue;

                dates.append( rec.value( DB_DATE ).toString() );
                symbols.append( symbol() );
                depths.append( d );
                hv.append( annualized * Stats::calcStdDeviation( r.mid( r.length() - d ) ) );

                rec.setValue( DB_DEPTH, d );
                model.setRecord( i, rec );
            }
        }

        prevClose = close;
    }

    model.submitAll();

    // ---- //

    static const QString sql( "REPLACE INTO historicalVolatility (date,symbol,depth,"
        "volatility) "
            "VALUES (:date,:symbol,:depth,"
                ":volatility) " );

    QSqlQuery query( db_ );
    query.prepare( sql );

    query.bindValue( ":" + DB_DATE, dates );
    query.bindValue( ":" + DB_SYMBOL, symbols );
    query.bindValue( ":" + DB_DEPTH, depths );
    query.bindValue( ":" + DB_VOLATILITY, hv );

    // exec sql
    if ( !query.execBatch() )
    {
        const QSqlError e( query.lastError() );

        LOG_WARN << "error during insert " << e.type() << " " << qPrintable( e.text() );
    }
}
