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
static const QString DB_VERSION( "4" );

static const QString CALL( "CALL" );
static const QString PUT( "PUT" );

static const QString SIMPLE( "simple" );
static const QString EXPONENTIAL( "exp" );

static const QString CUSIP( "cusip" );
static const QString LAST_FUNDAMENTAL( "lastFundamental" );
static const QString LAST_QUOTE_HISTORY( "lastQuoteHistory" );

///////////////////////////////////////////////////////////////////////////////////////////////////
SymbolDatabase::SymbolDatabase( const QString& symbol, QObject *parent ) :
    _Mybase( DB_NAME.arg( symbol ), DB_VERSION, parent ),
    symbol_( symbol ),
    divAmount_( 0.0 ),
    divYield_( 0.0 )
{
    // set object name
    setObjectName( symbol );

    // open database
    if ( open() )
    {
        QVariant v;

        // write out symbol
        writeSetting( "symbol", symbol );

        // read dividend information
        if ( readSetting( DB_DIV_AMOUNT, v ) )
            divAmount_ = v.toDouble();
        if ( readSetting( DB_DIV_YIELD, v ) )
            divYield_ = v.toDouble();

        if ( readSetting( DB_DIV_DATE, v ) )
            divDate_ = v.toDate();
        if ( readSetting( DB_DIV_FREQUENCY, v ) )
            divFrequency_ = v.toString();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SymbolDatabase::~SymbolDatabase()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString SymbolDatabase::cusip() const
{
    QVariant v;

    if ( readSetting( CUSIP, v ) )
        return v.toString();

    return QString();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double SymbolDatabase::dividendAmount( QDate& date, double& frequency ) const
{
    date = divDate_;

    if ( "Y" == divFrequency_ )
        frequency = 1.0;
    else if ( "B" == divFrequency_ )
        frequency = 0.5;
    else if ( "Q" == divFrequency_ )
        frequency = 0.25;
    else if ( "M" == divFrequency_ )
        frequency = (1.0 / 12.0);

    return divAmount_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double SymbolDatabase::dividendYield() const
{
    return (divYield_ / 100.0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double SymbolDatabase::historicalVolatility( const QDateTime& dt, int depth ) const
{
    static const QString sql( "SELECT date,depth,volatility FROM historicalVolatility "
        "WHERE DATE(date)=DATE(:date) "
        "ORDER BY depth ASC" );

    const QDate d( dt.date() );

    // this method is used by background threads, need to open dedicated connection
    QSqlDatabase conn( openDatabaseConnection() );

    for ( int days( 0 ); days < 7; ++days )
    {
        QSqlQuery query( conn );
        query.prepare( sql );

        query.bindValue( ":" + DB_DATE, d.addDays( -days ).toString( Qt::ISODate ) );

        if ( !query.exec() )
        {
            const QSqlError e( query.lastError() );

            LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        }
        else
        {
            double min( 0.0 );
            int min_depth( 0 );

            double max( 0.0 );
            int max_depth( 0 );

            // find bounding volatility
            QSqlQueryModel model;
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
            model.setQuery( std::move( query ) );
#else
            model.setQuery( query );
#endif

            if ( !model.rowCount() )
                continue;

            for ( int row( 0 ); row < model.rowCount(); ++row )
            {
                const QSqlRecord rec( model.record( row ) );

                const double v( rec.value( DB_VOLATILITY ).toDouble() );
                const int v_depth( rec.value( DB_DEPTH ).toInt() );

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
    }

    LOG_WARN << "no historical volatility found for " << qPrintable( symbol() ) << " " << qPrintable( d.toString() );
    return 0.0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::historicalVolatilities( const QDate& start, const QDate& end, QList<HistoricalVolatilities>& data ) const
{
    static const QString sql( "SELECT * FROM historicalVolatility "
        "WHERE DATE(:start)<=DATE(date) AND DATE(date)<=DATE(:end) "
        "ORDER BY DATE(date)" );

    // this method is used by background threads, need to open dedicated connection
    QSqlDatabase conn( openDatabaseConnection() );

    QSqlQuery query( conn );
    query.prepare( sql );
    query.bindValue( ":start", start.toString( Qt::ISODate ) );
    query.bindValue( ":end", end.toString( Qt::ISODate ) );

    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return;
    }

    QSqlQueryModel model;
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
    model.setQuery( std::move( query ) );
#else
    model.setQuery( query );
#endif

    // fetch all rows
    while ( model.canFetchMore() )
        model.fetchMore();

    // extract data
    QMap<QDate, HistoricalVolatilities*> vols;

    for ( int row( 0 ); row < model.rowCount(); ++row )
    {
        const QSqlRecord rec( model.record( row ) );

        const QDate dt( QDate::fromString( rec.value( DB_DATE ).toString(), Qt::ISODate ) );

        if ( !vols.contains( dt ) )
        {
            vols[dt] = new HistoricalVolatilities();
            vols[dt]->date = dt;
        }

        const int depth( rec.value( DB_DEPTH ).toInt() );
        const double vol( 100.0 * rec.value( DB_VOLATILITY ).toDouble() );

        vols[dt]->volatilities[depth] = vol;
    }

    // populate results
    foreach ( HistoricalVolatilities *vol, vols )
    {
        data.append( *vol );
        delete vol;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QDateTime SymbolDatabase::lastFundamentalProcessed() const
{
    QDateTime stamp;

    QVariant v;

    if ( readSetting( LAST_FUNDAMENTAL, v ) )
        stamp = QDateTime::fromString( v.toString(), Qt::ISODateWithMs );

    return stamp;
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
void SymbolDatabase::movingAverages( const QDate& start, const QDate& end, QList<MovingAverages>& data ) const
{
    static const QString sql( "SELECT * FROM movingAverage "
        "WHERE DATE(:start)<=DATE(date) AND DATE(date)<=DATE(:end) "
        "ORDER BY DATE(date)" );

    // this method is used by background threads, need to open dedicated connection
    QSqlDatabase conn( openDatabaseConnection() );

    QSqlQuery query( conn );
    query.prepare( sql );
    query.bindValue( ":start", start.toString( Qt::ISODate ) );
    query.bindValue( ":end", end.toString( Qt::ISODate ) );

    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return;
    }

    QSqlQueryModel model;
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
    model.setQuery( std::move( query ) );
#else
    model.setQuery( query );
#endif

    // fetch all rows
    while ( model.canFetchMore() )
        model.fetchMore();

    // extract data
    QMap<QDate, MovingAverages*> avgs;

    for ( int row( 0 ); row < model.rowCount(); ++row )
    {
        const QSqlRecord rec( model.record( row ) );

        const QDate dt( QDate::fromString( rec.value( DB_DATE ).toString(), Qt::ISODate ) );

        if ( !avgs.contains( dt ) )
        {
            avgs[dt] = new MovingAverages();
            avgs[dt]->date = dt;
        }

        const QString t( rec.value( DB_TYPE ).toString() );

        const int depth( rec.value( DB_DEPTH ).toInt() );
        const double avg( rec.value( DB_AVERAGE ).toDouble() );

        if ( SIMPLE == t )
            avgs[dt]->sma[depth] = avg;
        else if ( EXPONENTIAL == t )
            avgs[dt]->ema[depth] = avg;
    }

    // populate results
    foreach ( MovingAverages *avg, avgs )
    {
        data.append( *avg );
        delete avg;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::movingAveragesConvergenceDivergence( const QDate& start, const QDate& end, QList<MovingAveragesConvergenceDivergence>& data ) const
{
    static const QString sql( "SELECT * FROM movingAverageConvergenceDivergence "
        "WHERE DATE(:start)<=DATE(date) AND DATE(date)<=DATE(:end) "
        "ORDER BY DATE(date)" );

    // this method is used by background threads, need to open dedicated connection
    QSqlDatabase conn( openDatabaseConnection() );

    QSqlQuery query( conn );
    query.prepare( sql );
    query.bindValue( ":start", start.toString( Qt::ISODate ) );
    query.bindValue( ":end", end.toString( Qt::ISODate ) );

    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return;
    }

    QSqlQueryModel model;
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
    model.setQuery( std::move( query ) );
#else
    model.setQuery( query );
#endif

    // fetch all rows
    while ( model.canFetchMore() )
        model.fetchMore();

    // extract data
    for ( int row( 0 ); row < model.rowCount(); ++row )
    {
        const QSqlRecord rec( model.record( row ) );

        MovingAveragesConvergenceDivergence macd;
        macd.date = QDate::fromString( rec.value( DB_DATE ).toString(), Qt::ISODate );
        macd.ema[12] = rec.value( DB_EMA12 ).toDouble();
        macd.ema[26] = rec.value( DB_EMA26 ).toDouble();
        macd.macd = rec.value( DB_VALUE ).toDouble();
        macd.signal = rec.value( DB_SIGNAL_VALUE ).toDouble();
        macd.histogram = rec.value( DB_DIFF ).toDouble();

        data.append( macd );
    }
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

        end = QDate::fromString( rec.value( DB_DATE ).toString(), Qt::ISODate );
        break;
    }

    QSqlQueryModel modelStart;
    modelStart.setQuery( sql.arg( "ASC" ), db_ );

    for ( int i( 0 ); i < modelStart.rowCount(); ++i )
    {
        const QSqlRecord rec( modelStart.record( i ) );

        start = QDate::fromString( rec.value( DB_DATE ).toString(), Qt::ISODate );
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::relativeStrengthIndex( const QDate& start, const QDate& end, QList<RelativeStrengthIndexes>& data ) const
{
    static const QString sql( "SELECT * FROM relativeStrengthIndex "
        "WHERE DATE(:start)<=DATE(date) AND DATE(date)<=DATE(:end) "
        "ORDER BY DATE(date)" );

    // this method is used by background threads, need to open dedicated connection
    QSqlDatabase conn( openDatabaseConnection() );

    QSqlQuery query( conn );
    query.prepare( sql );
    query.bindValue( ":start", start.toString( Qt::ISODate ) );
    query.bindValue( ":end", end.toString( Qt::ISODate ) );

    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return;
    }

    QSqlQueryModel model;
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
    model.setQuery( std::move( query ) );
#else
    model.setQuery( query );
#endif

    // fetch all rows
    while ( model.canFetchMore() )
        model.fetchMore();

    // extract data
    QMap<QDate, RelativeStrengthIndexes*> values;

    for ( int row( 0 ); row < model.rowCount(); ++row )
    {
        const QSqlRecord rec( model.record( row ) );

        const QDate dt( QDate::fromString( rec.value( DB_DATE ).toString(), Qt::ISODate ) );

        if ( !values.contains( dt ) )
        {
            values[dt] = new RelativeStrengthIndexes();
            values[dt]->date = dt;
        }

        const int depth( rec.value( DB_DEPTH ).toInt() );
        const double value( rec.value( DB_VALUE ).toDouble() );

        values[dt]->values[depth] = value;
    }

    // populate results
    foreach ( RelativeStrengthIndexes *value, values )
    {
        data.append( *value );
        delete value;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::processInstrument( const QDateTime& stamp, const QJsonObject& obj )
{
    if ( symbol() != obj[DB_SYMBOL].toString() )
        return false;

    // this method is used by background threads, need to open dedicated connection
    QSqlDatabase conn( openDatabaseConnection() );

    if ( !conn.transaction() )
    {
        LOG_ERROR << "failed to start transaction";
        return false;
    }

    LOG_DEBUG << "process instrument for " << qPrintable( symbol() );

    bool result( true );
    bool fundamentalProcessed( false );

    // add fundamental (optional)
    const QJsonObject::const_iterator fundamental( obj.constFind( DB_FUNDAMENTAL ) );

    if (( obj.constEnd() != fundamental ) && ( fundamental->isObject() ))
    {
        result &= addFundamental( conn, stamp, fundamental->toObject() );
        fundamentalProcessed = true;
    }

    // commit to database
    if (( result ) && ( !(result = conn.commit()) ))
        LOG_ERROR << "commit failed";

    if (( !result ) && ( !conn.rollback() ))
        LOG_FATAL << "rollback failed";

    // save last fundamental
    if (( result ) && ( fundamentalProcessed ))
        writeSetting( LAST_FUNDAMENTAL, AppDatabase::instance()->currentDateTime().toString( Qt::ISODateWithMs ), conn );

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::processOptionChain( const QDateTime& stamp, const QJsonObject& obj, QList<QDate>& expiryDates )
{
    if ( symbol() != obj[DB_UNDERLYING].toString() )
        return false;

    // this method is used by background threads, need to open dedicated connection
    QSqlDatabase conn( openDatabaseConnection() );

    if ( !conn.transaction() )
    {
        LOG_ERROR << "failed to start transaction";
        return false;
    }

    LOG_DEBUG << "process option chain for " << qPrintable( symbol() );

    bool result( true );

    // add option chain
    result &= addOptionChain( conn, stamp, obj, expiryDates );

    // add quotes (optional)
    const QJsonObject::const_iterator quotes( obj.constFind( DB_QUOTES ) );

    if (( obj.constEnd() != quotes ) && ( quotes->isArray() ))
    {
        foreach ( const QJsonValue& v, quotes->toArray() )
            if ( v.isObject() )
                result &= addQuote( conn, v.toObject() );
    }

    // commit to database
    if (( result ) && ( !(result = conn.commit()) ))
        LOG_ERROR << "commit failed";

    if (( !result ) && ( !conn.rollback() ))
        LOG_FATAL << "rollback failed";

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::processQuote( const QDateTime& stamp, const QJsonObject& obj )
{
    Q_UNUSED( stamp )

    // this method is used by background threads, need to open dedicated connection
    QSqlDatabase conn( openDatabaseConnection() );

    if ( !conn.transaction() )
    {
        LOG_ERROR << "failed to start transaction";
        return false;
    }

    LOG_DEBUG << "process quote for " << qPrintable( symbol() );

    bool result( true );

    // check for option
    if ( obj.contains( DB_UNDERLYING ) )
        result &= addOption( conn, obj );
    else
        result &= addQuote( conn, obj );

    // commit to database
    if (( result ) && ( !(result = conn.commit()) ))
        LOG_ERROR << "commit failed";

    if (( !result ) && ( !conn.rollback() ))
        LOG_FATAL << "rollback failed";

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::processQuoteHistory( const QJsonObject& obj )
{
    if ( symbol() != obj[DB_SYMBOL].toString() )
        return false;

    // this method is used by background threads, need to open dedicated connection
    QSqlDatabase conn( openDatabaseConnection() );

    if ( !conn.transaction() )
    {
        LOG_ERROR << "failed to start transaction";
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
                result &= addQuoteHistory( conn, v.toObject() );

        LOG_TRACE << "calc historical...";

        // calculate historical volatility
        calcHistoricalVolatility( conn );

        // calculate moving averages
        calcMovingAverage( conn );

        // calculate RSI
        calcRelativeStrengthIndex( conn );

        // calculate MACD
        calcMovingAverageConvergenceDivergence( conn );
    }

    // commit to database
    LOG_TRACE << "commit...";

    if (( result ) && ( !(result = conn.commit()) ))
        LOG_ERROR << "commit failed";

    if (( !result ) && ( !conn.rollback() ))
        LOG_FATAL << "rollback failed";

    // save last quote history
    if ( result )
        writeSetting( LAST_QUOTE_HISTORY, AppDatabase::instance()->currentDateTime().toString( Qt::ISODateWithMs ), conn );

    LOG_TRACE << "done";
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
        files.append( QString( ":/db/version%0_symbol.sql" ).arg( ++from ) );

    return files;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::addFundamental( const QSqlDatabase& conn, const QDateTime& stamp, const QJsonObject& obj )
{
    static const QString sql( "INSERT INTO fundamentals (stamp,symbol,"
        "high52,low52,divAmount,divYield,divDate,divFrequency,peRatio,pegRatio,pbRatio,prRatio,pcfRatio,"
        "grossMarginTTM,grossMarginMRQ,netProfitMarginTTM,netProfitMarginMRQ,operatingMarginTTM,operatingMarginMRQ,returnOnEquity,returnOnAssets,returnOnInvestment,quickRatio,"
        "currentRatio,interestCoverage,totalDebtToCapital,ltDebtToEquity,totalDebtToEquity,epsTTM,epsChangePercentTTM,epsChangeYear,epsChange,revChangeYear,"
        "revChangeTTM,revChangeIn,sharesOutstanding,marketCapFloat,marketCap,bookValuePerShare,shortIntToFloat,shortIntDayToCover,divGrowthRate3Year,divPayAmount,"
        "divPayDate,beta,vol1DayAvg,vol10DayAvg,vol3MonthAvg) "
            "VALUES (:stamp,:symbol,"
                ":high52,:low52,:divAmount,:divYield,:divDate,:divFrequency,:peRatio,:pegRatio,:pbRatio,:prRatio,:pcfRatio,"
                ":grossMarginTTM,:grossMarginMRQ,:netProfitMarginTTM,:netProfitMarginMRQ,:operatingMarginTTM,:operatingMarginMRQ,:returnOnEquity,:returnOnAssets,:returnOnInvestment,:quickRatio,"
                ":currentRatio,:interestCoverage,:totalDebtToCapital,:ltDebtToEquity,:totalDebtToEquity,:epsTTM,:epsChangePercentTTM,:epsChangeYear,:epsChange,:revChangeYear,"
                ":revChangeTTM,:revChangeIn,:sharesOutstanding,:marketCapFloat,:marketCap,:bookValuePerShare,:shortIntToFloat,:shortIntDayToCover,:divGrowthRate3Year,:divPayAmount,"
                ":divPayDate,:beta,:vol1DayAvg,:vol10DayAvg,:vol3MonthAvg) " );

    QSqlQuery query( conn );
    query.prepare( sql );
    query.bindValue( ":" + DB_STAMP, stamp.toString( Qt::ISODateWithMs ) );
    query.bindValue( ":" + DB_SYMBOL, symbol() );

    bindQueryValues( query, obj );

    // compute dividend frequency
    if ( divFrequency_.isEmpty() )
    {
        if ( obj.contains( DB_DIV_DATE ) )
            calcDividendFrequencyFromDate( conn, obj[DB_DIV_DATE] );

        if ( obj.contains( DB_DIV_PAY_DATE ) )
            calcDividendFrequencyFromPayDate( conn, obj[DB_DIV_PAY_DATE] );

        if (( obj.contains( DB_DIV_PAY_AMOUNT ) ) && ( obj.contains( DB_DIV_AMOUNT ) ))
            calcDividendFrequencyFromPayAmount( conn, obj[DB_DIV_PAY_AMOUNT], obj[DB_DIV_AMOUNT] );
    }

    // fill in missing values
    updateDefaultValue( query, obj, DB_DIV_AMOUNT, conn );
    updateDefaultValue( query, obj, DB_DIV_YIELD, conn );
    updateDefaultValue( query, obj, DB_DIV_DATE, conn );
    updateDefaultValue( query, obj, DB_DIV_FREQUENCY, conn );

    updateDefaultValue( query, obj, DB_DIV_PAY_AMOUNT, conn );
    updateDefaultValue( query, obj, DB_DIV_PAY_DATE, conn );

    updateDefaultValue( query, obj, DB_PE_RATIO, conn );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during insert " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::addOption( const QSqlDatabase& conn, const QJsonObject& obj )
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

    QSqlQuery query( conn );
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

        LOG_ERROR << "error during replace " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::addOptionChain( const QSqlDatabase& conn, const QDateTime& stamp, const QJsonObject& obj, QList<QDate>& expiryDates )
{
    static const QString sql( "INSERT INTO optionChains (stamp,underlying,"
        "underlyingPrice,interestRate,isDelayed,isIndex,numberOfContracts,volatility) "
            "VALUES (:stamp,:underlying,"
                ":underlyingPrice,:interestRate,:isDelayed,:isIndex,:numberOfContracts,:volatility) " );

    QSqlQuery query( conn );
    query.prepare( sql );
    query.bindValue( ":" + DB_STAMP, stamp.toString( Qt::ISODateWithMs ) );

    bindQueryValues( query, obj );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during insert " << e.type() << " " << qPrintable( e.text() );
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
                if ( !addOption( conn, option ) )
                    return false;

                // add strike price to chain
                else if ( !addOptionChainStrikePrice( conn, stamp, optionStampIt->toString(), optionSymbolIt->toString(), typeIt->toString(), expiryDate.date().toString( Qt::ISODate ), strikePriceIt->toDouble() ) )
                    return false;

                // track expiry dates for caller
                if ( !expiryDates.contains( expiryDate.date() ) )
                    expiryDates.append( expiryDate.date() );
            }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::addOptionChainStrikePrice( const QSqlDatabase& conn, const QDateTime& stamp, const QString& optionStamp, const QString& optionSymbol, const QString& type, const QString& expiryDate, double strikePrice )
{
    static const QString sql( "INSERT INTO optionChainStrikePrices (stamp,underlying,expirationDate,strikePrice,"
        "%1Stamp,%1Symbol) "
            "VALUES (:stamp,:underlying,:expirationDate,:strikePrice,"
                ":optionStamp,:optionSymbol) "
            "ON CONFLICT (stamp,underlying,expirationDate,strikePrice) DO UPDATE SET "
                "%1Stamp=:optionStamp,%1Symbol=:optionSymbol " );

    QSqlQuery query( conn );
    query.prepare( sql.arg( type ) );
    query.bindValue( ":" + DB_STAMP, stamp.toString( Qt::ISODateWithMs ) );
    query.bindValue( ":" + DB_UNDERLYING, symbol() );
    query.bindValue( ":" + DB_EXPIRY_DATE, expiryDate );
    query.bindValue( ":" + DB_STRIKE_PRICE, strikePrice );

    query.bindValue( ":optionStamp", optionStamp );
    query.bindValue( ":optionSymbol", optionSymbol );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during insert " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::addQuote( const QSqlDatabase& conn, const QJsonObject& obj )
{
    static const QString sql( "REPLACE INTO quotes (stamp,symbol,"
        "description,assetMainType,assetSubType,assetType,cusip,bidAskSize,bidPrice,bidSize,bidId,bidTick,"
        "askPrice,askSize,askId,lastPrice,lastSize,lastId,openPrice,highPrice,lowPrice,closePrice,"
        "change,percentChange,totalVolume,quoteTime,tradeTime,mark,markChange,markPercentChange,fiftyTwoWeekHigh,fiftyTwoWeekLow,percentBelowFiftyTwoWeekHigh,percentAboveFiftyTwoWeekLow,fiftyTwoWeekPriceRange,"
        "exchange,exchangeName,isMarginable,isShortable,isDelayed,volatility,digits,nAV,peRatio,impliedYield,"
        "divAmount,divYield,divDate,divFrequency,securityStatus,regularMarketLastPrice,regularMarketLastSize,regularMarketChange,regularMarketPercentChange,regularMarketTradeTime,tick,"
        "tickAmount,product,tradingHours,isTradable,marketMaker) "
            "VALUES (:stamp,:symbol,"
                ":description,:assetMainType,:assetSubType,:assetType,:cusip,:bidAskSize,:bidPrice,:bidSize,:bidId,:bidTick,"
                ":askPrice,:askSize,:askId,:lastPrice,:lastSize,:lastId,:openPrice,:highPrice,:lowPrice,:closePrice,"
                ":change,:percentChange,:totalVolume,:quoteTime,:tradeTime,:mark,:markChange,:markPercentChange,:fiftyTwoWeekHigh,:fiftyTwoWeekLow,:percentBelowFiftyTwoWeekHigh,:percentAboveFiftyTwoWeekLow,:fiftyTwoWeekPriceRange,"
                ":exchange,:exchangeName,:isMarginable,:isShortable,:isDelayed,:volatility,:digits,:nAV,:peRatio,:impliedYield,"
                ":divAmount,:divYield,:divDate,:divFrequency,:securityStatus,:regularMarketLastPrice,:regularMarketLastSize,:regularMarketChange,:regularMarketPercentChange,:regularMarketTradeTime,:tick,"
                ":tickAmount,:product,:tradingHours,:isTradable,:marketMaker) " );

    QSqlQuery query( conn );
    query.prepare( sql );

    // update values
    bindQueryValues( query, obj );

    // compute dividend frequency
    if ( divFrequency_.isEmpty() )
    {
        if ( obj.contains( DB_DIV_DATE ) )
            calcDividendFrequencyFromDate( conn, obj[DB_DIV_DATE] );
    }

    // fill in missing values
    updateDefaultValue( query, obj, DB_ASSET_MAIN_TYPE, conn );
    updateDefaultValue( query, obj, DB_ASSET_SUB_TYPE, conn );
    updateDefaultValue( query, obj, DB_ASSET_TYPE, conn );
    updateDefaultValue( query, obj, DB_CUSIP, conn );

    updateDefaultValue( query, obj, DB_DIV_AMOUNT, conn );
    updateDefaultValue( query, obj, DB_DIV_YIELD, conn );
    updateDefaultValue( query, obj, DB_DIV_DATE, conn );
    updateDefaultValue( query, obj, DB_DIV_FREQUENCY, conn );

    updateDefaultValue( query, obj, DB_SECURITY_STATUS, conn );

    updateDefaultValue( query, obj, DB_NAV, conn );
    updateDefaultValue( query, obj, DB_PE_RATIO, conn );

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
bool SymbolDatabase::addQuoteHistory( const QSqlDatabase& conn, const QJsonObject& obj )
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

    const QDateTime dt( QDateTime::fromString( obj[DB_DATETIME].toString(), Qt::ISODateWithMs ) );

    QSqlQuery query( conn );
    query.prepare( sql );
    query.bindValue( ":" + DB_DATE, dt.date().toString( Qt::ISODate ) );
    query.bindValue( ":" + DB_SYMBOL, symbol() );

    bindQueryValues( query, obj );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during insert " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::writeSetting( const QString& key, const QVariant& value )
{
    return _Mybase::writeSetting( key, value );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::writeSetting( const QString& key, const QVariant& value, const QSqlDatabase& conn )
{
    bool result;

    // write setting
    if ( (result = _Mybase::writeSetting( key, value, conn )) )
    {
        // dividend information
        if ( DB_DIV_AMOUNT == key )
            divAmount_ = value.toDouble();
        else if ( DB_DIV_YIELD == key )
            divYield_ = value.toDouble();
        else if ( DB_DIV_DATE == key )
            divDate_ = QDate::fromString( value.toString(), Qt::ISODate );
        else if ( DB_DIV_FREQUENCY == key )
            divFrequency_ = value.toString();
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::calcHistoricalVolatility( const QSqlDatabase& conn )
{
    const double annualized( sqrt( AppDatabase::instance()->numTradingDays() ) );

    // records for quote history
    QVariantList quoteDates;
    QVariantList quoteSymbols;
    QVariantList quoteDepths;

    // records for historic volatility
    QVariantList histVolDates;
    QVariantList histVolSymbols;
    QVariantList histVolDepths;
    QVariantList histVol;

    // historic volatility days
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

    static const QString sql( "SELECT * FROM quoteHistory ORDER BY date ASC" );

    QSqlQuery query( conn );
    query.prepare( sql );

    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return;
    }

    QSqlQueryModel model;
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
    model.setQuery( std::move( query ) );
#else
    model.setQuery( query );
#endif

    // fetch all rows
    while ( model.canFetchMore() )
        model.fetchMore();

    // force update of last N rows
    const int forced( model.rowCount() - FORCED_UPDATE );

    // calculate returns
    QVector<double> r;
    r.reserve( model.rowCount() );

    double prevClose( 0.0 );

    for ( int i( 0 ); i < model.rowCount(); ++i )
    {
        const QSqlRecord rec( model.record( i ) );

        const double close( rec.value( DB_CLOSE_PRICE ).toDouble() );

        if (( i ) && ( 0.0 < close ) && ( 0.0 < prevClose ))
        {
            // calc log of interday return
            r.append( log( close / prevClose ) );

            // lookup depth of this record
            bool update( false );
            int depth( 0 );

            if ( !rec.isNull( DB_HV_DEPTH ) )
                depth = rec.value( DB_HV_DEPTH ).toInt();

            // calc historical volatility for each depth
            foreach ( int d, hvd )
            {
                if ( r.length() < d )
                    break;
                else if (( i < forced ) && ( d <= depth ))
                    continue;

                histVolDates.append( rec.value( DB_DATE ).toString() );
                histVolSymbols.append( symbol() );
                histVolDepths.append( d );
                histVol.append( annualized * Stats::calcStdDeviation( r.mid( r.length() - d ) ) );

                update = true;
                depth = d;
            }

            // update record
            if ( update )
            {
                quoteDates.append( rec.value( DB_DATE ).toString() );
                quoteSymbols.append( symbol() );
                quoteDepths.append( depth );
            }
        }

        prevClose = close;
    }

    // ---- //

    static const QString quoteSql( "UPDATE quoteHistory "
        "SET hvDepth=:hvDepth "
            "WHERE date=:date AND symbol=:symbol" );

    QSqlQuery quoteQuery( conn );
    quoteQuery.prepare( quoteSql );

    quoteQuery.bindValue( ":" + DB_DATE, quoteDates );
    quoteQuery.bindValue( ":" + DB_SYMBOL, quoteSymbols );
    quoteQuery.bindValue( ":" + DB_HV_DEPTH, quoteDepths );

    // exec sql
    if ( !quoteQuery.execBatch() )
    {
        const QSqlError e( quoteQuery.lastError() );

        LOG_ERROR << "error during update " << e.type() << " " << qPrintable( e.text() );
    }

    // ---- //

    static const QString valuesSql( "REPLACE INTO historicalVolatility (date,symbol,depth,"
        "volatility) "
            "VALUES (:date,:symbol,:depth,"
                ":volatility) " );

    QSqlQuery valuesQuery( conn );
    valuesQuery.prepare( valuesSql );

    valuesQuery.bindValue( ":" + DB_DATE, histVolDates );
    valuesQuery.bindValue( ":" + DB_SYMBOL, histVolSymbols );
    valuesQuery.bindValue( ":" + DB_DEPTH, histVolDepths );
    valuesQuery.bindValue( ":" + DB_VOLATILITY, histVol );

    // exec sql
    if ( !valuesQuery.execBatch() )
    {
        const QSqlError e( valuesQuery.lastError() );

        LOG_ERROR << "error during replace " << e.type() << " " << qPrintable( e.text() );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::calcMovingAverage( const QSqlDatabase& conn )
{
    // records for quote history
    QVariantList quoteDates;
    QVariantList quoteSymbols;
    QVariantList quoteDepths;

    // records for moving averages
    QVariantList movingAvgDates;
    QVariantList movingAvgSymbols;
    QVariantList movingAvgTypes;
    QVariantList movingAvgDepths;
    QVariantList movingAvg;

    // moving average days
    QVector<int> mad;
    mad.append( 5 );        // 5d
    mad.append( 10 );       // 10d
    mad.append( 15 );       // 15d
    mad.append( 20 );       // 20d
    mad.append( 30 );       // 30d
    mad.append( 50 );       // 50d
    mad.append( 100 );      // 100d
    mad.append( 200 );      // 200d

    // ---- //

    static const QString sql( "SELECT * FROM quoteHistory ORDER BY date ASC" );

    QSqlQuery query( conn );
    query.prepare( sql );

    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return;
    }

    QSqlQueryModel model;
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
    model.setQuery( std::move( query ) );
#else
    model.setQuery( query );
#endif

    // fetch all rows
    while ( model.canFetchMore() )
        model.fetchMore();

    // force update of last N rows
    const int forced( model.rowCount() - FORCED_UPDATE );

    // calculate averages
    QVector<double> a;
    a.reserve( model.rowCount() );

    QMap<int, double> ema;

    for ( int i( 0 ); i < model.rowCount(); ++i )
    {
        const QSqlRecord rec( model.record( i ) );

        const double close( rec.value( DB_CLOSE_PRICE ).toDouble() );

        if ( 0.0 < close )
        {
            a.append( close );

            // lookup depth of this record
            bool update( false );
            int depth( 0 );

            if ( !rec.isNull( DB_MA_DEPTH ) )
                depth = rec.value( DB_MA_DEPTH ).toInt();

            // calc moving average for each depth
            foreach ( int d, mad )
            {
                if ( a.length() < d )
                    break;

                // calc ema
                if ( !ema.contains( d ) )
                    ema[d] = Stats::calcMean( a.mid( a.length() - d ) ); // use sma
                else
                {
                    const double w( 2.0 / (1.0 + d) );

                    ema[d] = close*w + ema[d]*(1.0 - w);
                }

                // check if this already exists in db
                if (( i < forced ) && ( d <= depth ))
                    continue;

                // simple moving average
                movingAvgDates.append( rec.value( DB_DATE ).toString() );
                movingAvgSymbols.append( symbol() );
                movingAvgTypes.append( SIMPLE );
                movingAvgDepths.append( d );
                movingAvg.append( Stats::calcMean( a.mid( a.length() - d ) ) );

                // exponential moving average
                movingAvgDates.append( rec.value( DB_DATE ).toString() );
                movingAvgSymbols.append( symbol() );
                movingAvgTypes.append( EXPONENTIAL );
                movingAvgDepths.append( d );
                movingAvg.append( ema[d] );

                update = true;
                depth = d;
            }

            // update record
            if ( update )
            {
                quoteDates.append( rec.value( DB_DATE ).toString() );
                quoteSymbols.append( symbol() );
                quoteDepths.append( depth );
            }
        }
    }

    // ---- //

    static const QString quoteSql( "UPDATE quoteHistory "
        "SET maDepth=:maDepth "
            "WHERE date=:date AND symbol=:symbol" );

    QSqlQuery quoteQuery( conn );
    quoteQuery.prepare( quoteSql );

    quoteQuery.bindValue( ":" + DB_DATE, quoteDates );
    quoteQuery.bindValue( ":" + DB_SYMBOL, quoteSymbols );
    quoteQuery.bindValue( ":" + DB_MA_DEPTH, quoteDepths );

    // exec sql
    if ( !quoteQuery.execBatch() )
    {
        const QSqlError e( quoteQuery.lastError() );

        LOG_ERROR << "error during update " << e.type() << " " << qPrintable( e.text() );
    }

    // ---- //

    static const QString valuesSql( "REPLACE INTO movingAverage (date,symbol,type,depth,"
        "average) "
            "VALUES (:date,:symbol,:type,:depth,"
                ":average) " );

    QSqlQuery valuesQuery( conn );
    valuesQuery.prepare( valuesSql );

    valuesQuery.bindValue( ":" + DB_DATE, movingAvgDates );
    valuesQuery.bindValue( ":" + DB_SYMBOL, movingAvgSymbols );
    valuesQuery.bindValue( ":" + DB_TYPE, movingAvgTypes );
    valuesQuery.bindValue( ":" + DB_DEPTH, movingAvgDepths );
    valuesQuery.bindValue( ":" + DB_AVERAGE, movingAvg );

    // exec sql
    if ( !valuesQuery.execBatch() )
    {
        const QSqlError e( valuesQuery.lastError() );

        LOG_ERROR << "error during replace " << e.type() << " " << qPrintable( e.text() );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::calcRelativeStrengthIndex( const QSqlDatabase& conn )
{
    // records for quote history
    QVariantList quoteDates;
    QVariantList quoteSymbols;
    QVariantList quoteDepths;

    // records for rsi
    QVariantList rsiDates;
    QVariantList rsiSymbols;
    QVariantList rsiDepths;
    QVariantList rsi;

    // rsi days
    QVector<int> rsid;
    rsid.append( 2 );       // 2d
    rsid.append( 3 );       // 3d
    rsid.append( 4 );       // 4d
    rsid.append( 5 );       // 5d
    rsid.append( 6 );       // 6d
    rsid.append( 10 );      // 10d
    rsid.append( 14 );      // 14d
    rsid.append( 20 );      // 20d
    rsid.append( 50 );      // 50d

    // ---- //

    static const QString sql( "SELECT * FROM quoteHistory ORDER BY date ASC" );

    QSqlQuery query( conn );
    query.prepare( sql );

    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return;
    }

    QSqlQueryModel model;
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
    model.setQuery( std::move( query ) );
#else
    model.setQuery( query );
#endif

    // fetch all rows
    while ( model.canFetchMore() )
        model.fetchMore();

    // force update of last N rows
    const int forced( model.rowCount() - FORCED_UPDATE );

    // calculate returns
    QVector<double> r;
    r.reserve( model.rowCount() );

    double prevClose( 0.0 );

    QMap<int, double> avgGain;
    QMap<int, double> avgLoss;

    for ( int i( 0 ); i < model.rowCount(); ++i )
    {
        const QSqlRecord rec( model.record( i ) );

        const double close( rec.value( DB_CLOSE_PRICE ).toDouble() );

        if (( i ) && ( 0.0 < close ) && ( 0.0 < prevClose ))
        {
            const double current( close - prevClose );

            // calc returns
            r.append( current );

            // lookup depth of this record
            bool update( false );
            int depth( 0 );

            if ( !rec.isNull( DB_RSI_DEPTH ) )
                depth = rec.value( DB_RSI_DEPTH ).toInt();

            // calc rsi for each depth
            foreach ( int d, rsid )
            {
                if ( r.length() < d )
                    break;

                // average gain/loss
                if ( !avgGain.contains( d ) )
                {
                    double gains( 0.0 );
                    double losses( 0.0 );

                    // returns for period
                    const QVector<double> rp( r.mid( r.length() - d ) );

                    foreach ( const double& v, rp )
                    {
                        if ( v < 0.0 )
                            losses += v;
                        else
                            gains += v;
                    }

                    avgGain[d] = gains / d;
                    avgLoss[d] = std::fabs( losses ) / d;
                }
                else
                {
                    // loss
                    if ( current < 0.0 )
                    {
                        avgGain[d] = avgGain[d]*(d-1) / d;
                        avgLoss[d] = (avgLoss[d]*(d-1) + std::fabs( current )) / d;
                    }
                    // gain
                    else
                    {
                        avgGain[d] = (avgGain[d]*(d-1) + current) / d;
                        avgLoss[d] = avgLoss[d]*(d-1) / d;
                    }
                }

                // check if this already exists in db
                if (( i < forced ) && ( d <= depth ))
                    continue;

                const double rs( avgGain[d] / std::fmax( avgLoss[d], 1.0e-10 ) );

                double index( 100.0 - 100.0 / (1.0+rs) );
                index = std::fmin( index, 100.0 );
                index = std::fmax( index, 0.0 );

                // RSI
                rsiDates.append( rec.value( DB_DATE ).toString() );
                rsiSymbols.append( symbol() );
                rsiDepths.append( d );
                rsi.append( index );

                update = true;
                depth = d;
            }

            // update record
            if ( update )
            {
                quoteDates.append( rec.value( DB_DATE ).toString() );
                quoteSymbols.append( symbol() );
                quoteDepths.append( depth );
            }
        }

        prevClose = close;
    }

    // ---- //

    static const QString quoteSql( "UPDATE quoteHistory "
        "SET rsiDepth=:rsiDepth "
            "WHERE date=:date AND symbol=:symbol" );

    QSqlQuery quoteQuery( conn );
    quoteQuery.prepare( quoteSql );

    quoteQuery.bindValue( ":" + DB_DATE, quoteDates );
    quoteQuery.bindValue( ":" + DB_SYMBOL, quoteSymbols );
    quoteQuery.bindValue( ":" + DB_RSI_DEPTH, quoteDepths );

    // exec sql
    if ( !quoteQuery.execBatch() )
    {
        const QSqlError e( quoteQuery.lastError() );

        LOG_ERROR << "error during update " << e.type() << " " << qPrintable( e.text() );
    }

    // ---- //

    static const QString valuesSql( "REPLACE INTO relativeStrengthIndex (date,symbol,depth,"
        "value) "
            "VALUES (:date,:symbol,:depth,"
                ":value) " );

    QSqlQuery valuesQuery( conn );
    valuesQuery.prepare( valuesSql );

    valuesQuery.bindValue( ":" + DB_DATE, rsiDates );
    valuesQuery.bindValue( ":" + DB_SYMBOL, rsiSymbols );
    valuesQuery.bindValue( ":" + DB_DEPTH, rsiDepths );
    valuesQuery.bindValue( ":" + DB_VALUE, rsi );

    // exec sql
    if ( !valuesQuery.execBatch() )
    {
        const QSqlError e( valuesQuery.lastError() );

        LOG_ERROR << "error during replace " << e.type() << " " << qPrintable( e.text() );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::calcMovingAverageConvergenceDivergence( const QSqlDatabase& conn )
{
    // records for quote history
    QVariantList quoteDates;
    QVariantList quoteSymbols;
    QVariantList quoteExists;

    // records for MACD
    QVariantList convDivDates;
    QVariantList convDivSymbols;
    QVariantList convDivExp12Vals;
    QVariantList convDivExp26Vals;
    QVariantList convDiv;
    QVariantList convDivSignal;
    QVariantList convDivDiff;

    // moving average days
    QVector<int> mad;
    mad.append( 12 );       // 12d
    mad.append( 26 );       // 26d

    // ---- //

    static const QString sql( "SELECT * FROM quoteHistory ORDER BY date ASC" );

    QSqlQuery query( conn );
    query.prepare( sql );

    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return;
    }

    QSqlQueryModel model;
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
    model.setQuery( std::move( query ) );
#else
    model.setQuery( query );
#endif

    // fetch all rows
    while ( model.canFetchMore() )
        model.fetchMore();

    // force update of last N rows
    const int forced( model.rowCount() - FORCED_UPDATE );

    // calculate averages
    QVector<double> a;
    a.reserve( model.rowCount() );

    QMap<int, double> ema;
    QVector<double> macdVals;

    for ( int i( 0 ); i < model.rowCount(); ++i )
    {
        const QSqlRecord rec( model.record( i ) );

        const double close( rec.value( DB_CLOSE_PRICE ).toDouble() );

        if ( 0.0 < close )
        {
            a.append( close );

            // lookup depth of this record
            bool exists( false );

            if ( !rec.isNull( DB_MACD ) )
                exists = rec.value( DB_MACD ).toBool();

            // calc moving average for each depth
            foreach ( int d, mad )
            {
                if ( a.length() < d )
                    break;

                // calc ema
                if ( !ema.contains( d ) )
                    ema[d] = Stats::calcMean( a.mid( a.length() - d ) ); // use sma
                else
                {
                    const double w( 2.0 / (1.0 + d) );

                    ema[d] = close*w + ema[d]*(1.0 - w);
                }
            }

            // check we can calc MACD
            if ( !ema.contains( 26 ) )
                continue;

            const double macd( ema[12] - ema[26] );

            // calc ema9 of macd
            if ( !ema.contains( 9 ) )
            {
                macdVals.append( macd );

                if ( 9 == macdVals.size() )
                    ema[9] = Stats::calcMean( macdVals ); // use sma
                else
                {
                    continue;
                }
            }
            else
            {
                const double w( 2.0 / (1.0 + 9.0) );

                ema[9] = macd*w + ema[9]*(1.0 - w);
            }

            // check if this already exists in db
            if (( i < forced ) && ( exists ))
                continue;

            // MACD
            convDivDates.append( rec.value( DB_DATE ).toString() );
            convDivSymbols.append( symbol() );
            convDivExp12Vals.append( ema[12] );
            convDivExp26Vals.append( ema[26] );
            convDiv.append( macd );
            convDivSignal.append( ema[9] );
            convDivDiff.append( macd - ema[9] );

            quoteDates.append( rec.value( DB_DATE ).toString() );
            quoteSymbols.append( symbol() );
            quoteExists.append( true );
        }
    }

    // ---- //

    static const QString quoteSql( "UPDATE quoteHistory "
        "SET macd=:macd "
            "WHERE date=:date AND symbol=:symbol" );

    QSqlQuery quoteQuery( conn );
    quoteQuery.prepare( quoteSql );

    quoteQuery.bindValue( ":" + DB_DATE, quoteDates );
    quoteQuery.bindValue( ":" + DB_SYMBOL, quoteSymbols );
    quoteQuery.bindValue( ":" + DB_MACD, quoteExists );

    // exec sql
    if ( !quoteQuery.execBatch() )
    {
        const QSqlError e( quoteQuery.lastError() );

        LOG_ERROR << "error during update " << e.type() << " " << qPrintable( e.text() );
    }

    // ---- //

    static const QString valuesSql( "REPLACE INTO movingAverageConvergenceDivergence (date,symbol,"
        "ema12,ema26,value,signalValue,diff) "
            "VALUES (:date,:symbol,"
                ":ema12,:ema26,:value,:signalValue,:diff) " );

    QSqlQuery valuesQuery( conn );
    valuesQuery.prepare( valuesSql );

    valuesQuery.bindValue( ":" + DB_DATE, convDivDates );
    valuesQuery.bindValue( ":" + DB_SYMBOL, convDivSymbols );
    valuesQuery.bindValue( ":" + DB_EMA12, convDivExp12Vals );
    valuesQuery.bindValue( ":" + DB_EMA26, convDivExp26Vals );
    valuesQuery.bindValue( ":" + DB_VALUE, convDiv );
    valuesQuery.bindValue( ":" + DB_SIGNAL_VALUE, convDivSignal );
    valuesQuery.bindValue( ":" + DB_DIFF, convDivDiff );

    // exec sql
    if ( !valuesQuery.execBatch() )
    {
        const QSqlError e( valuesQuery.lastError() );

        LOG_ERROR << "error during replace " << e.type() << " " << qPrintable( e.text() );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::calcDividendFrequencyFromDate( const QSqlDatabase& conn, const QJsonValue& newValue )
{
    if ( divFrequency_.length() )
        return;

    QVariant oldValue;

    if (( readSetting( DB_DIV_DATE, oldValue ) ) && ( oldValue.isValid() ))
    {
        const QDate oldDate( QDate::fromString( oldValue.toString(), Qt::ISODate ) );
        const QDate newDate( QDate::fromString( newValue.toString(), Qt::ISODate ) );

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

            writeSetting( DB_DIV_FREQUENCY, freq, conn );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::calcDividendFrequencyFromPayAmount( const QSqlDatabase& conn, const QJsonValue& payAmountVal, const QJsonValue& amountVal )
{
    if ( divFrequency_.length() )
        return;
    else if (( !payAmountVal.isDouble() ) || ( !amountVal.isDouble() ))
        return;

    const double payAmount( payAmountVal.toDouble() );
    const double amount( amountVal.toDouble() );

    if (( 0.0 < payAmount ) && ( payAmount <= amount ))
    {
        const int delta( std::round( (365.0 * payAmount) / amount ) );

        QString freq;

        if ( 350 < delta )
            freq = "Y";
        else if ( 175 < delta )
            freq = "B";
        else if ( delta < 45 )
            freq = "M";
        else
            freq = "Q";

        writeSetting( DB_DIV_FREQUENCY, freq, conn );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::calcDividendFrequencyFromPayDate( const QSqlDatabase& conn, const QJsonValue& newValue )
{
    if ( divFrequency_.length() )
        return;

    QVariant oldValue;

    if (( readSetting( DB_DIV_PAY_DATE, oldValue ) ) && ( oldValue.isValid() ))
    {
        const QDate oldDate( QDate::fromString( oldValue.toString(), Qt::ISODate ) );
        const QDate newDate( QDate::fromString( newValue.toString(), Qt::ISODate ) );

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

            writeSetting( DB_DIV_FREQUENCY, freq, conn );
        }
    }
}
