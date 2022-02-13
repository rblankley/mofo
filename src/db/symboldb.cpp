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

static const QString DB_NAME( "%1.db" );
static const QString DB_VERSION( "5" );

static const QString CALL( "CALL" );
static const QString PUT( "PUT" );

static const QString SIMPLE( "simple" );
static const QString EXPONENTIAL( "exp" );

static const QString CUSIP( "cusip" );
static const QString DESCRIPTION( "description" );
static const QString LAST_FUNDAMENTAL( "lastFundamental" );
static const QString LAST_QUOTE_HISTORY( "lastQuoteHistory" );

// sql statement for prepared query
static const QString SQL_OPTION( "REPLACE INTO options (stamp,symbol,"
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

// sql statement for prepared query
static const QString SQL_OPTION_CHAIN_STRIKES_CALL( "INSERT INTO optionChainStrikePrices (stamp,underlying,expirationDate,strikePrice,"
    "callStamp,callSymbol) "
        "VALUES (:stamp,:underlying,:expirationDate,:strikePrice,"
            ":optionStamp,:optionSymbol) "
        "ON CONFLICT (stamp,underlying,expirationDate,strikePrice) DO UPDATE SET "
            "callStamp=:optionStamp,callSymbol=:optionSymbol " );

// sql statement for prepared query
static const QString SQL_OPTION_CHAIN_STRIKES_PUT( "INSERT INTO optionChainStrikePrices (stamp,underlying,expirationDate,strikePrice,"
    "putStamp,putSymbol) "
        "VALUES (:stamp,:underlying,:expirationDate,:strikePrice,"
            ":optionStamp,:optionSymbol) "
        "ON CONFLICT (stamp,underlying,expirationDate,strikePrice) DO UPDATE SET "
            "putStamp=:optionStamp,putSymbol=:optionSymbol " );

///////////////////////////////////////////////////////////////////////////////////////////////////
SymbolDatabase::SymbolDatabase( const QString& symbol, QObject *parent ) :
    _Mybase( DB_NAME.arg( symbol ), DB_VERSION, parent ),
    symbol_( symbol ),
    divAmount_( 0.0 ),
    divYield_( 0.0 ),
#if QT_VERSION < QT_VERSION_CHECK( 5, 14, 0 )
    m_( QMutex::Recursive ),
#endif
    ref_( 0 )
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
QString SymbolDatabase::description() const
{
    QVariant v;

    if ( readSetting( DESCRIPTION, v ) )
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
double SymbolDatabase::historicalVolatility( const QDate& date, int depth ) const
{
    // find a recent hv
    for ( int days( 0 ); days < 7; ++days )
    {
        const QDate dt( date.addDays( -days ) );

        double min( 0.0 );
        double max( 0.0 );

        // range for one day should be same
        historicalVolatilityRange( dt, dt, depth, min, max );

        if (( 0.0 != min ) && ( min == max ))
            return min;
    }

    LOG_WARN << "no historical volatility found for " << qPrintable( symbol() ) << " " << qPrintable( date.toString() );
    return 0.0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::historicalVolatilityRange( const QDate& start, const QDate& end, int depth, double& min, double& max ) const
{
    static const QString sqlDepths( "SELECT DISTINCT depth FROM historicalVolatility "
        "WHERE DATE(:start)<=DATE(date) AND DATE(date)<=DATE(:end)" );

    QSqlQuery queryDepths( connection() );
    queryDepths.setForwardOnly( true );
    queryDepths.prepare( sqlDepths );

    queryDepths.bindValue( ":start", start.toString( Qt::ISODate ) );
    queryDepths.bindValue( ":end", end.toString( Qt::ISODate ) );

    if ( !queryDepths.exec() )
    {
        const QSqlError e( queryDepths.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return;
    }

    // determine depth above and below
    int above( 999999 );
    int below( 0 );

    while ( queryDepths.next() )
    {
        const QSqlRecord rec( queryDepths.record() );

        const int d( rec.value( DB_DEPTH ).toInt() );

        if ( depth <= d )
            above = qMin( d, above );

        if ( d <= depth )
            below = qMax( d, below );
    }

    // ---- //

    static const QString sql( "SELECT * FROM historicalVolatility "
        "WHERE DATE(:start)<=DATE(date) AND DATE(date)<=DATE(:end) "
        "ORDER BY DATE(date)" );

    QSqlQuery query( connection() );
    query.setForwardOnly( true );
    query.prepare( sql );

    query.bindValue( ":start", start.toString( Qt::ISODate ) );
    query.bindValue( ":end", end.toString( Qt::ISODate ) );

    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return;
    }

    // extract data
    QMap<QDate, QPair<double, double>> vols;

    while ( query.next() )
    {
        const QSqlRecord rec( query.record() );

        const int d( rec.value( DB_DEPTH ).toInt() );

        if (( d != below ) && ( d != above ))
            continue;

        const QDate dt( QDate::fromString( rec.value( DB_DATE ).toString(), Qt::ISODate ) );
        const double v( rec.value( DB_VOLATILITY ).toDouble() );

        QPair<double, double> vol( 0.0, 0.0 );

        if ( vols.contains( dt ) )
            vol = vols[dt];

        if ( d == below )
            vol.first = v;
        if ( d == above )
            vol.second = v;

        vols[dt] = vol;
    }

    if ( vols.isEmpty() )
        return;

    // ---- //

    bool first( true );

    for ( QMap<QDate, QPair<double, double>>::const_iterator i( vols.constBegin() ); i != vols.constEnd(); ++i )
    {
        double v;

        if ( i->first <= 0.0 )
            v = i->second;
        else
        {
            v = i->first;

            // interpolate
            if (( 0.0 < i->second ) && ( below < above ))
                v += ((double)(depth - below) / (double)(above - below)) * (i->second - i->first);
        }

        if ( first )
        {
            min = max = v;
            first = false;
        }
        else
        {
            min = qMin( v, min );
            max = qMax( v, max );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::historicalVolatilities( const QDate& start, const QDate& end, QList<HistoricalVolatilities>& data ) const
{
    static const QString sql( "SELECT * FROM historicalVolatility "
        "WHERE DATE(:start)<=DATE(date) AND DATE(date)<=DATE(:end) "
        "ORDER BY DATE(date)" );

    QSqlQuery query( connection() );
    query.setForwardOnly( true );
    query.prepare( sql );

    query.bindValue( ":start", start.toString( Qt::ISODate ) );
    query.bindValue( ":end", end.toString( Qt::ISODate ) );

    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return;
    }

    // extract data
    QMap<QDate, HistoricalVolatilities*> vols;

    while ( query.next() )
    {
        const QSqlRecord rec( query.record() );

        const QDate dt( QDate::fromString( rec.value( DB_DATE ).toString(), Qt::ISODate ) );

        if ( !vols.contains( dt ) )
        {
            vols[dt] = new HistoricalVolatilities();
            vols[dt]->date = dt;
        }

        const int depth( rec.value( DB_DEPTH ).toInt() );
        const double vol( rec.value( DB_VOLATILITY ).toDouble() );

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
bool SymbolDatabase::isLocked() const
{
    QMutexLocker guard( &m_ );
    return (0 < ref_);
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

    QSqlQuery query( connection() );
    query.setForwardOnly( true );
    query.prepare( sql );

    query.bindValue( ":start", start.toString( Qt::ISODate ) );
    query.bindValue( ":end", end.toString( Qt::ISODate ) );

    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return;
    }

    // extract data
    QMap<QDate, MovingAverages*> avgs;

    while ( query.next() )
    {
        const QSqlRecord rec( query.record() );

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

    QSqlQuery query( connection() );
    query.setForwardOnly( true );
    query.prepare( sql );

    query.bindValue( ":start", start.toString( Qt::ISODate ) );
    query.bindValue( ":end", end.toString( Qt::ISODate ) );

    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return;
    }

    // extract data
    while ( query.next() )
    {
        const QSqlRecord rec( query.record() );

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
void SymbolDatabase::optionChainCurves( const QDate& expiryDate, const QDateTime& stamp, OptionChainCurves& data ) const
{
    static const QString sql( "SELECT * FROM optionChainStrikePrices "
        "WHERE DATE(:date)=DATE(expirationData) AND %1" );

    QSqlQuery query( connection() );
    query.setForwardOnly( true );

    if ( stamp.isValid() )
        query.prepare( sql.arg( "DATETIME(:stamp)=DATETIME(stamp)" ) );
    else
        query.prepare( sql.arg( "stamp=(SELECT MAX(stamp) FROM optionChainView)" ) );

    query.bindValue( ":" + DB_EXPIRY_DATE, expiryDate.toString( Qt::ISODate ) );
    query.bindValue( ":" + DB_STAMP, stamp.toString( Qt::ISODateWithMs ) );

    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return;
    }

    // extract data
    while ( query.next() )
    {
        const QSqlRecord rec( query.record() );

        const double strikePrice( rec.value( DB_STRIKE_PRICE ).toDouble() );

        data.callVolatility[strikePrice] = rec.value( DB_CALL_VOLATILITY ).toDouble();
        data.putVolatility[strikePrice] = rec.value( DB_PUT_VOLATILITY ).toDouble();
        data.volatility[strikePrice] = rec.value( DB_VOLATILITY ).toDouble();

        data.itmProbability[strikePrice] = rec.value( DB_ITM_PROBABILITY ).toDouble();
        data.otmProbability[strikePrice] = rec.value( DB_OTM_PROBABILITY ).toDouble();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::quoteHistoryDateRange( QDate& start, QDate& end ) const
{
    const QString sql( "SELECT date FROM quoteHistory ORDER BY DATE(date) %1 LIMIT 5" );

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

        end = QDate::fromString( rec.value( DB_DATE ).toString(), Qt::ISODate );
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

        start = QDate::fromString( rec.value( DB_DATE ).toString(), Qt::ISODate );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::relativeStrengthIndex( const QDate& start, const QDate& end, QList<RelativeStrengthIndexes>& data ) const
{
    static const QString sql( "SELECT * FROM relativeStrengthIndex "
        "WHERE DATE(:start)<=DATE(date) AND DATE(date)<=DATE(:end) "
        "ORDER BY DATE(date)" );

    QSqlQuery query( connection() );
    query.setForwardOnly( true );
    query.prepare( sql );

    query.bindValue( ":start", start.toString( Qt::ISODate ) );
    query.bindValue( ":end", end.toString( Qt::ISODate ) );

    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return;
    }

    // extract data
    QMap<QDate, RelativeStrengthIndexes*> values;

    while ( query.next() )
    {
        const QSqlRecord rec( query.record() );

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
void SymbolDatabase::setOptionChainCurves( const QDate& expiryDate, const QDateTime& stamp, const OptionChainCurves& data )
{
    static const QString sql( "UPDATE optionChainStrikePrices SET "
        "volatility=:volatility,callVolatility=:callVolatility,putVolatility=:putVolatility,"
        "itmProbability=:itmProbability,otmProbability=:otmProbability "
            "WHERE stamp=:stamp AND underlying=:underlying AND expirationDate=:expirationDate AND strikePrice=:strikePrice" );

    QMutexLocker guard( &writer_ );

    // start transaction
    QSqlDatabase conn( connection() );

    if ( !conn.transaction() )
    {
        const QSqlError e( conn.lastError() );

        LOG_ERROR << "failed to start transaction " << e.type() << " " << qPrintable( e.text() );
        return;
    }

    const QList<double> strikes( data.itmProbability.keys() );
    bool result( true );

    QSqlQuery query( conn );
    query.prepare( sql );

    foreach ( const double strike, strikes )
    {
        query.bindValue( ":" + DB_STAMP, stamp.toString( Qt::ISODateWithMs ) );
        query.bindValue( ":" + DB_UNDERLYING, symbol() );
        query.bindValue( ":" + DB_EXPIRY_DATE, expiryDate.toString( Qt::ISODate ) );
        query.bindValue( ":" + DB_STRIKE_PRICE, strike );

        if (( data.volatility.contains( strike ) ) && ( 0.0 < data.volatility[strike] ))
            query.bindValue( ":" + DB_VOLATILITY, data.volatility[strike] );
        if (( data.callVolatility.contains( strike ) ) && ( 0.0 < data.callVolatility[strike] ))
            query.bindValue( ":" + DB_CALL_VOLATILITY, data.callVolatility[strike] );
        if (( data.putVolatility.contains( strike ) ) && ( 0.0 < data.putVolatility[strike] ))
            query.bindValue( ":" + DB_PUT_VOLATILITY, data.putVolatility[strike] );

        query.bindValue( ":" + DB_ITM_PROBABILITY, data.itmProbability[strike] );
        query.bindValue( ":" + DB_OTM_PROBABILITY, data.otmProbability[strike] );

        // exec sql
        result &= query.exec();

        if ( !result )
        {
            const QSqlError e( query.lastError() );

            LOG_ERROR << "error during update " << e.type() << " " << qPrintable( e.text() );
            break;
        }
    }

    // commit to database
    if (( result ) && ( !(result = conn.commit()) ))
    {
        const QSqlError e( conn.lastError() );

        LOG_ERROR << "commit failed " << e.type() << " " << qPrintable( e.text() );
    }

    if (( !result ) && ( !conn.rollback() ))
        LOG_FATAL << "rollback failed";
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::addRef()
{
    QMutexLocker guard( &m_ );
    ++ref_;

    LOG_TRACE << qPrintable( symbol_ ) << " refs " << ref_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::removeRef()
{
    QMutexLocker guard( &m_ );

    if ( --ref_ < 0 )
        ref_ = 0;

    LOG_TRACE << qPrintable( symbol_ ) << " refs " << ref_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::processInstrument( const QDateTime& stamp, const QJsonObject& obj )
{
    if ( symbol() != obj[DB_SYMBOL].toString() )
        return false;

    QMutexLocker guard( &writer_ );

    // start transaction
    QSqlDatabase conn( connection() );

    if ( !conn.transaction() )
    {
        const QSqlError e( conn.lastError() );

        LOG_ERROR << "failed to start transaction " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    LOG_DEBUG << "process instrument for " << qPrintable( symbol() );

    bool result( true );
    bool fundamentalProcessed( false );

    // add fundamental (optional)
    const QJsonObject::const_iterator fundamental( obj.constFind( DB_FUNDAMENTAL ) );

    if (( obj.constEnd() != fundamental ) && ( fundamental->isObject() ))
    {
        result &= addFundamental( stamp, fundamental->toObject() );
        fundamentalProcessed = true;
    }

    // commit to database
    if (( result ) && ( !(result = conn.commit()) ))
    {
        const QSqlError e( conn.lastError() );

        LOG_ERROR << "commit failed " << e.type() << " " << qPrintable( e.text() );
    }

    if (( !result ) && ( !conn.rollback() ))
        LOG_FATAL << "rollback failed";

    // save last fundamental
    if (( result ) && ( fundamentalProcessed ))
        writeSetting( LAST_FUNDAMENTAL, AppDatabase::instance()->currentDateTime().toString( Qt::ISODateWithMs ) );

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::processOptionChain( const QDateTime& stamp, const QJsonObject& obj, QList<QDate>& expiryDates )
{
    if ( symbol() != obj[DB_UNDERLYING].toString() )
        return false;

    QMutexLocker guard( &writer_ );

    // start transaction
    QSqlDatabase conn( connection() );

    if ( !conn.transaction() )
    {
        const QSqlError e( conn.lastError() );

        LOG_ERROR << "failed to start transaction " << e.type() << " " << qPrintable( e.text() );
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
    if (( result ) && ( !(result = conn.commit()) ))
    {
        const QSqlError e( conn.lastError() );

        LOG_ERROR << "commit failed " << e.type() << " " << qPrintable( e.text() );
    }

    if (( !result ) && ( !conn.rollback() ))
        LOG_FATAL << "rollback failed";

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::processQuote( const QDateTime& stamp, const QJsonObject& obj )
{
    Q_UNUSED( stamp )

    QMutexLocker guard( &writer_ );

    // start transaction
    QSqlDatabase conn( connection() );

    if ( !conn.transaction() )
    {
        const QSqlError e( conn.lastError() );

        LOG_ERROR << "failed to start transaction " << e.type() << " " << qPrintable( e.text() );
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
    if (( result ) && ( !(result = conn.commit()) ))
    {
        const QSqlError e( conn.lastError() );

        LOG_ERROR << "commit failed " << e.type() << " " << qPrintable( e.text() );
    }

    if (( !result ) && ( !conn.rollback() ))
        LOG_FATAL << "rollback failed";

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::processQuoteHistory( const QJsonObject& obj )
{
    if ( symbol() != obj[DB_SYMBOL].toString() )
        return false;

    QMutexLocker guard( &writer_ );

    // start transaction
    QSqlDatabase conn( connection() );

    if ( !conn.transaction() )
    {
        const QSqlError e( conn.lastError() );

        LOG_ERROR << "failed to start transaction " << e.type() << " " << qPrintable( e.text() );
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

        LOG_TRACE << "calc historical...";

        // calculate historical volatility
        calcHistoricalVolatility();

        // calculate moving averages
        calcMovingAverage();

        // calculate RSI
        calcRelativeStrengthIndex();

        // calculate MACD
        calcMovingAverageConvergenceDivergence();
    }

    // commit to database
    LOG_TRACE << "commit...";

    if (( result ) && ( !(result = conn.commit()) ))
    {
        const QSqlError e( conn.lastError() );

        LOG_ERROR << "commit failed " << e.type() << " " << qPrintable( e.text() );
    }

    if (( !result ) && ( !conn.rollback() ))
        LOG_FATAL << "rollback failed";

    // save last quote history
    if ( result )
        writeSetting( LAST_QUOTE_HISTORY, AppDatabase::instance()->currentDateTime().toString( Qt::ISODateWithMs ) );

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
bool SymbolDatabase::addFundamental( const QDateTime& stamp, const QJsonObject& obj )
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

    QSqlQuery query( connection() );
    query.prepare( sql );

    query.bindValue( ":" + DB_STAMP, stamp.toString( Qt::ISODateWithMs ) );
    query.bindValue( ":" + DB_SYMBOL, symbol() );

    bindQueryValues( query, obj );

    // compute dividend frequency
    if ( divFrequency_.isEmpty() )
    {
        if ( obj.contains( DB_DIV_DATE ) )
            calcDividendFrequencyFromDate( obj[DB_DIV_DATE] );

        if ( obj.contains( DB_DIV_PAY_DATE ) )
            calcDividendFrequencyFromPayDate( obj[DB_DIV_PAY_DATE] );

        if (( obj.contains( DB_DIV_PAY_AMOUNT ) ) && ( obj.contains( DB_DIV_AMOUNT ) ))
            calcDividendFrequencyFromPayAmount( obj[DB_DIV_PAY_AMOUNT], obj[DB_DIV_AMOUNT] );
    }

    // fill in missing values
    updateDefaultValue( query, obj, DB_DIV_AMOUNT );
    updateDefaultValue( query, obj, DB_DIV_YIELD );
    updateDefaultValue( query, obj, DB_DIV_DATE );
    updateDefaultValue( query, obj, DB_DIV_FREQUENCY );

    updateDefaultValue( query, obj, DB_DIV_PAY_AMOUNT );
    updateDefaultValue( query, obj, DB_DIV_PAY_DATE );

    updateDefaultValue( query, obj, DB_PE_RATIO );

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
bool SymbolDatabase::addOption( const QJsonObject& obj )
{
    QSqlQuery query( connection() );
    query.prepare( SQL_OPTION );

    return addOption( obj, query );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::addOption( const QJsonObject& obj, QSqlQuery& query )
{
    const double strikePrice( obj[DB_STRIKE_PRICE].toDouble() );
    const QString type( obj[DB_TYPE].toString() );

    // bind values to query
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
bool SymbolDatabase::addOptionChain( const QDateTime& stamp, const QJsonObject& obj, QList<QDate>& expiryDates )
{
    static const QString sql( "INSERT INTO optionChains (stamp,underlying,"
        "underlyingPrice,interestRate,isDelayed,isIndex,numberOfContracts,volatility) "
            "VALUES (:stamp,:underlying,"
                ":underlyingPrice,:interestRate,:isDelayed,:isIndex,:numberOfContracts,:volatility) " );

    QSqlQuery query( connection() );
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
        // prepare query objects
        QSqlQuery queryOption( connection() );
        queryOption.prepare( SQL_OPTION );

        QSqlQuery queryOptionChainStrikesCall( connection() );
        queryOptionChainStrikesCall.prepare( SQL_OPTION_CHAIN_STRIKES_CALL );

        QSqlQuery queryOptionChainStrikesPut( connection() );
        queryOptionChainStrikesPut.prepare( SQL_OPTION_CHAIN_STRIKES_PUT );

        // iterate
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
                const QString type( typeIt->toString() );

                // add option
                if ( !addOption( option, queryOption ) )
                    return false;

                // add strike price to chain
                else if (( CALL == type ) && ( !addOptionChainStrikePrice( stamp, optionStampIt->toString(), optionSymbolIt->toString(), expiryDate.date().toString( Qt::ISODate ), strikePriceIt->toDouble(), queryOptionChainStrikesCall ) ))
                    return false;
                else if (( PUT == type ) && ( !addOptionChainStrikePrice( stamp, optionStampIt->toString(), optionSymbolIt->toString(), expiryDate.date().toString( Qt::ISODate ), strikePriceIt->toDouble(), queryOptionChainStrikesPut ) ))
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
    QSqlQuery query( connection() );

    if ( CALL == type )
        query.prepare( SQL_OPTION_CHAIN_STRIKES_CALL );
    else if ( PUT == type )
        query.prepare( SQL_OPTION_CHAIN_STRIKES_PUT );
    else
    {
        LOG_WARN << "unknown type " << qPrintable( type );
        return false;
    }

    return addOptionChainStrikePrice( stamp, optionStamp, optionSymbol, expiryDate, strikePrice, query );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolDatabase::addOptionChainStrikePrice( const QDateTime& stamp, const QString& optionStamp, const QString& optionSymbol, const QString& expiryDate, double strikePrice, QSqlQuery& query )
{
    // bind values to query
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
bool SymbolDatabase::addQuote( const QJsonObject& obj )
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

    QSqlQuery query( connection() );
    query.prepare( sql );

    // update values
    bindQueryValues( query, obj );

    // compute dividend frequency
    if ( divFrequency_.isEmpty() )
    {
        if ( obj.contains( DB_DIV_DATE ) )
            calcDividendFrequencyFromDate( obj[DB_DIV_DATE] );
    }

    // fill in missing values
    updateDefaultValue( query, obj, DB_ASSET_MAIN_TYPE );
    updateDefaultValue( query, obj, DB_ASSET_SUB_TYPE );
    updateDefaultValue( query, obj, DB_ASSET_TYPE );
    updateDefaultValue( query, obj, DB_CUSIP );
    updateDefaultValue( query, obj, DB_DESCRIPTION );

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

        LOG_ERROR << "error during replace " << e.type() << " " << qPrintable( e.text() );
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

    const QDateTime dt( QDateTime::fromString( obj[DB_DATETIME].toString(), Qt::ISODateWithMs ) );

    QSqlQuery query( connection() );
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
    bool result;

    // write setting
    if ( (result = _Mybase::writeSetting( key, value )) )
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
int SymbolDatabase::quoteHistoryRowCount() const
{
    static const QString sql( "SELECT COUNT(*) FROM quoteHistory" );

    QSqlQuery query( connection() );
    query.setForwardOnly( true );

    if ( !query.exec( sql ) )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
    }
    else if ( query.next() )
    {
        const QSqlRecord rec( query.record() );

        return rec.value( 0 ).toInt();
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::calcHistoricalVolatility()
{
    const double annualized( sqrt( AppDatabase::instance()->numTradingDays() ) );

    // records for quote history
    static const QString quoteSql( "UPDATE quoteHistory "
        "SET hvDepth=:hvDepth "
            "WHERE date=:date AND symbol=:symbol" );

    QSqlQuery quoteQuery( connection() );
    quoteQuery.prepare( quoteSql );

    // records for historic volatility
    static const QString valuesSql( "REPLACE INTO historicalVolatility (date,symbol,depth,"
        "volatility) "
            "VALUES (:date,:symbol,:depth,"
                ":volatility) " );

    QSqlQuery valuesQuery( connection() );
    valuesQuery.prepare( valuesSql );

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

    const int rows( quoteHistoryRowCount() );

    static const QString sql( "SELECT * FROM quoteHistory ORDER BY date ASC" );

    QSqlQuery query( connection() );
    query.setForwardOnly( true );

    if ( !query.exec( sql ) )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return;
    }

    // force update of last N rows
    const int forced( rows - FORCED_UPDATE );

    // calculate returns
    QVector<double> r;
    r.reserve( rows );

    int row( 0 );
    double prevClose( 0.0 );

    while ( query.next() )
    {
        const QSqlRecord rec( query.record() );

        const double close( rec.value( DB_CLOSE_PRICE ).toDouble() );

        if (( row ) && ( 0.0 < close ) && ( 0.0 < prevClose ))
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
                else if (( row < forced ) && ( d <= depth ))
                    continue;

                // update
                valuesQuery.bindValue( ":" + DB_DATE, rec.value( DB_DATE ).toString() );
                valuesQuery.bindValue( ":" + DB_SYMBOL, symbol() );
                valuesQuery.bindValue( ":" + DB_DEPTH, d );
                valuesQuery.bindValue( ":" + DB_VOLATILITY, annualized * Stats::calcStdDeviation( r.mid( r.length() - d ) ) );

                // exec sql
                if ( !valuesQuery.exec() )
                {
                    const QSqlError e( valuesQuery.lastError() );

                    LOG_ERROR << "error during replace " << e.type() << " " << qPrintable( e.text() );
                }

                update = true;
                depth = d;
            }

            // update record
            if ( update )
            {
                quoteQuery.bindValue( ":" + DB_DATE, rec.value( DB_DATE ).toString() );
                quoteQuery.bindValue( ":" + DB_SYMBOL, symbol() );
                quoteQuery.bindValue( ":" + DB_HV_DEPTH, depth );

                // exec sql
                if ( !quoteQuery.exec() )
                {
                    const QSqlError e( quoteQuery.lastError() );

                    LOG_ERROR << "error during update " << e.type() << " " << qPrintable( e.text() );
                }
            }
        }

        ++row;
        prevClose = close;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::calcMovingAverage()
{
    // records for quote history
    static const QString quoteSql( "UPDATE quoteHistory "
        "SET maDepth=:maDepth "
            "WHERE date=:date AND symbol=:symbol" );

    QSqlQuery quoteQuery( connection() );
    quoteQuery.prepare( quoteSql );

    // records for moving averages
    static const QString valuesSql( "REPLACE INTO movingAverage (date,symbol,type,depth,"
        "average) "
            "VALUES (:date,:symbol,:type,:depth,"
                ":average) " );

    QSqlQuery valuesQuery( connection() );
    valuesQuery.prepare( valuesSql );

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

    const int rows( quoteHistoryRowCount() );

    static const QString sql( "SELECT * FROM quoteHistory ORDER BY date ASC" );

    QSqlQuery query( connection() );
    query.setForwardOnly( true );

    if ( !query.exec( sql ) )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return;
    }

    // force update of last N rows
    const int forced( rows - FORCED_UPDATE );

    // calculate averages
    QVector<double> a;
    a.reserve( rows );

    int row( 0 );

    QMap<int, double> ema;

    while ( query.next() )
    {
        const QSqlRecord rec( query.record() );

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
                if (( row < forced ) && ( d <= depth ))
                    continue;

                // simple moving average
                valuesQuery.bindValue( ":" + DB_DATE, rec.value( DB_DATE ).toString() );
                valuesQuery.bindValue( ":" + DB_SYMBOL, symbol() );
                valuesQuery.bindValue( ":" + DB_TYPE, SIMPLE );
                valuesQuery.bindValue( ":" + DB_DEPTH, d );
                valuesQuery.bindValue( ":" + DB_AVERAGE, Stats::calcMean( a.mid( a.length() - d ) ) );

                // exec sql
                if ( !valuesQuery.exec() )
                {
                    const QSqlError e( valuesQuery.lastError() );

                    LOG_ERROR << "error during replace " << e.type() << " " << qPrintable( e.text() );
                }

                // exponential moving average
                valuesQuery.bindValue( ":" + DB_DATE, rec.value( DB_DATE ).toString() );
                valuesQuery.bindValue( ":" + DB_SYMBOL, symbol() );
                valuesQuery.bindValue( ":" + DB_TYPE, EXPONENTIAL );
                valuesQuery.bindValue( ":" + DB_DEPTH, d );
                valuesQuery.bindValue( ":" + DB_AVERAGE, ema[d] );

                // exec sql
                if ( !valuesQuery.exec() )
                {
                    const QSqlError e( valuesQuery.lastError() );

                    LOG_ERROR << "error during replace " << e.type() << " " << qPrintable( e.text() );
                }

                update = true;
                depth = d;
            }

            // update record
            if ( update )
            {
                quoteQuery.bindValue( ":" + DB_DATE, rec.value( DB_DATE ).toString() );
                quoteQuery.bindValue( ":" + DB_SYMBOL, symbol() );
                quoteQuery.bindValue( ":" + DB_MA_DEPTH, depth );

                // exec sql
                if ( !quoteQuery.exec() )
                {
                    const QSqlError e( quoteQuery.lastError() );

                    LOG_ERROR << "error during update " << e.type() << " " << qPrintable( e.text() );
                }
            }
        }

        ++row;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::calcRelativeStrengthIndex()
{
    // records for quote history
    static const QString quoteSql( "UPDATE quoteHistory "
        "SET rsiDepth=:rsiDepth "
            "WHERE date=:date AND symbol=:symbol" );

    QSqlQuery quoteQuery( connection() );
    quoteQuery.prepare( quoteSql );

    // records for rsi
    static const QString valuesSql( "REPLACE INTO relativeStrengthIndex (date,symbol,depth,"
        "value) "
            "VALUES (:date,:symbol,:depth,"
                ":value) " );

    QSqlQuery valuesQuery( connection() );
    valuesQuery.prepare( valuesSql );

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

    const int rows( quoteHistoryRowCount() );

    static const QString sql( "SELECT * FROM quoteHistory ORDER BY date ASC" );

    QSqlQuery query( connection() );
    query.setForwardOnly( true );

    if ( !query.exec( sql ) )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return;
    }

    // force update of last N rows
    const int forced( rows - FORCED_UPDATE );

    // calculate returns
    QVector<double> r;
    r.reserve( rows );

    int row( 0 );
    double prevClose( 0.0 );

    QMap<int, double> avgGain;
    QMap<int, double> avgLoss;

    while ( query.next() )
    {
        const QSqlRecord rec( query.record() );

        const double close( rec.value( DB_CLOSE_PRICE ).toDouble() );

        if (( row ) && ( 0.0 < close ) && ( 0.0 < prevClose ))
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
                if (( row < forced ) && ( d <= depth ))
                    continue;

                const double rs( avgGain[d] / std::fmax( avgLoss[d], 1.0e-10 ) );

                double index( 100.0 - 100.0 / (1.0+rs) );
                index = std::fmin( index, 100.0 );
                index = std::fmax( index, 0.0 );

                // RSI
                valuesQuery.bindValue( ":" + DB_DATE, rec.value( DB_DATE ).toString() );
                valuesQuery.bindValue( ":" + DB_SYMBOL, symbol() );
                valuesQuery.bindValue( ":" + DB_DEPTH, d );
                valuesQuery.bindValue( ":" + DB_VALUE, index );

                // exec sql
                if ( !valuesQuery.exec() )
                {
                    const QSqlError e( valuesQuery.lastError() );

                    LOG_ERROR << "error during replace " << e.type() << " " << qPrintable( e.text() );
                }

                update = true;
                depth = d;
            }

            // update record
            if ( update )
            {
                quoteQuery.bindValue( ":" + DB_DATE, rec.value( DB_DATE ).toString() );
                quoteQuery.bindValue( ":" + DB_SYMBOL, symbol() );
                quoteQuery.bindValue( ":" + DB_RSI_DEPTH, depth );

                // exec sql
                if ( !quoteQuery.exec() )
                {
                    const QSqlError e( quoteQuery.lastError() );

                    LOG_ERROR << "error during update " << e.type() << " " << qPrintable( e.text() );
                }
            }
        }

        ++row;
        prevClose = close;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::calcMovingAverageConvergenceDivergence()
{
    // records for quote history
    static const QString quoteSql( "UPDATE quoteHistory "
        "SET macd=:macd "
            "WHERE date=:date AND symbol=:symbol" );

    QSqlQuery quoteQuery( connection() );
    quoteQuery.prepare( quoteSql );

    // records for MACD
    static const QString valuesSql( "REPLACE INTO movingAverageConvergenceDivergence (date,symbol,"
        "ema12,ema26,value,signalValue,diff) "
            "VALUES (:date,:symbol,"
                ":ema12,:ema26,:value,:signalValue,:diff) " );

    QSqlQuery valuesQuery( connection() );
    valuesQuery.prepare( valuesSql );

    // moving average days
    QVector<int> mad;
    mad.append( 12 );       // 12d
    mad.append( 26 );       // 26d

    // ---- //

    const int rows( quoteHistoryRowCount() );

    static const QString sql( "SELECT * FROM quoteHistory ORDER BY date ASC" );

    QSqlQuery query( connection() );
    query.setForwardOnly( true );

    if ( !query.exec( sql ) )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return;
    }

    // force update of last N rows
    const int forced( rows - FORCED_UPDATE );

    // calculate averages
    QVector<double> a;
    a.reserve( rows );

    int row( 0 );

    QMap<int, double> ema;
    QVector<double> macdVals;

    while ( query.next() )
    {
        const QSqlRecord rec( query.record() );

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
            if (( row < forced ) && ( exists ))
                continue;

            // MACD
            valuesQuery.bindValue( ":" + DB_DATE, rec.value( DB_DATE ).toString() );
            valuesQuery.bindValue( ":" + DB_SYMBOL, symbol() );
            valuesQuery.bindValue( ":" + DB_EMA12, ema[12] );
            valuesQuery.bindValue( ":" + DB_EMA26, ema[26] );
            valuesQuery.bindValue( ":" + DB_VALUE, macd );
            valuesQuery.bindValue( ":" + DB_SIGNAL_VALUE, ema[9] );
            valuesQuery.bindValue( ":" + DB_DIFF, macd - ema[9] );

            // exec sql
            if ( !valuesQuery.exec() )
            {
                const QSqlError e( valuesQuery.lastError() );

                LOG_ERROR << "error during replace " << e.type() << " " << qPrintable( e.text() );
            }

            // update record
            quoteQuery.bindValue( ":" + DB_DATE, rec.value( DB_DATE ).toString() );
            quoteQuery.bindValue( ":" + DB_SYMBOL, symbol() );
            quoteQuery.bindValue( ":" + DB_MACD, true );

            // exec sql
            if ( !quoteQuery.exec() )
            {
                const QSqlError e( quoteQuery.lastError() );

                LOG_ERROR << "error during update " << e.type() << " " << qPrintable( e.text() );
            }
        }

        ++row;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::calcDividendFrequencyFromDate( const QJsonValue& newValue )
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

            writeSetting( DB_DIV_FREQUENCY, freq );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::calcDividendFrequencyFromPayAmount( const QJsonValue& payAmountVal, const QJsonValue& amountVal )
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

        writeSetting( DB_DIV_FREQUENCY, freq );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabase::calcDividendFrequencyFromPayDate( const QJsonValue& newValue )
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

            writeSetting( DB_DIV_FREQUENCY, freq );
        }
    }
}
