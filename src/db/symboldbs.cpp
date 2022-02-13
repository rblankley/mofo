/**
 * @file symboldbs.cpp
 *
 * @copyright Copyright (C) 2022 Randy Blankley. All rights reserved.
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
#include "symboldbs.h"

#include <QApplication>
#include <QJsonArray>
#include <QJsonObject>
#include <QTimer>

static const QString DAILY( "daily" );

QMutex SymbolDatabases::instanceMutex_;
SymbolDatabases *SymbolDatabases::instance_( nullptr );

///////////////////////////////////////////////////////////////////////////////////////////////////
SymbolDatabases::SymbolDatabases() :
    _Mybase(),
#if QT_VERSION < QT_VERSION_CHECK( 5, 14, 0 )
    m_( QMutex::Recursive ),
#endif
    cleanup_( nullptr )
{
    // register meta types
    qRegisterMetaType<QList<CandleData>>();

    // setup timer for cleanup
    cleanup_ = new QTimer( this );
    cleanup_->setSingleShot( false );
    cleanup_->setInterval( REMOVE_DB_TIME );
    cleanup_->start();

    connect( cleanup_, &QTimer::timeout, this, &_Myt::onTimeout );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SymbolDatabases::~SymbolDatabases()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString SymbolDatabases::cusip( const QString& symbol ) const
{
    SymbolDatabase *child( const_cast<_Myt*>( this )->findSymbol( symbol ) );

    if ( child )
    {
        SymbolDatabaseRemoveRef deref( symbol );
        return child->cusip();
    }

    return QString();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString SymbolDatabases::description( const QString& symbol ) const
{
    SymbolDatabase *child( const_cast<_Myt*>( this )->findSymbol( symbol ) );

    if ( child )
    {
        SymbolDatabaseRemoveRef deref( symbol );
        return child->description();
    }

    return QString();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double SymbolDatabases::dividendAmount( const QString& symbol, QDate& date, double& frequency ) const
{
    SymbolDatabase *child( const_cast<_Myt*>( this )->findSymbol( symbol ) );

    if ( child )
    {
        SymbolDatabaseRemoveRef deref( symbol );
        return child->dividendAmount( date, frequency );
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double SymbolDatabases::dividendYield( const QString& symbol ) const
{
    SymbolDatabase *child( const_cast<_Myt*>( this )->findSymbol( symbol ) );

    if ( child )
    {
        SymbolDatabaseRemoveRef deref( symbol );
        return child->dividendYield();
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double SymbolDatabases::historicalVolatility( const QString& symbol, const QDate& date, int depth ) const
{
    SymbolDatabase *child( const_cast<_Myt*>( this )->findSymbol( symbol ) );

    if ( child )
    {
        SymbolDatabaseRemoveRef deref( symbol );
        return child->historicalVolatility( date, depth );
    }

    return 0.0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabases::historicalVolatilityRange( const QString& symbol, const QDate& start, const QDate& end, int depth, double& min, double& max ) const
{
    SymbolDatabase *child( const_cast<_Myt*>( this )->findSymbol( symbol ) );

    if ( child )
    {
        SymbolDatabaseRemoveRef deref( symbol );
        return child->historicalVolatilityRange( start, end, depth, min, max );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabases::historicalVolatilities( const QString& symbol, const QDate& start, const QDate& end, QList<HistoricalVolatilities>& data ) const
{
    SymbolDatabase *child( const_cast<_Myt*>( this )->findSymbol( symbol ) );

    if ( child )
    {
        SymbolDatabaseRemoveRef deref( symbol );
        child->historicalVolatilities( start, end, data );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QDateTime SymbolDatabases::lastFundamentalProcessed( const QString& symbol ) const
{
    SymbolDatabase *child( const_cast<_Myt*>( this )->findSymbol( symbol ) );

    if ( child )
    {
        SymbolDatabaseRemoveRef deref( symbol );
        return child->lastFundamentalProcessed();
    }

    return QDateTime();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QDateTime SymbolDatabases::lastQuoteHistoryProcessed( const QString& symbol ) const
{
    SymbolDatabase *child( const_cast<_Myt*>( this )->findSymbol( symbol ) );

    if ( child )
    {
        SymbolDatabaseRemoveRef deref( symbol );
        return child->lastQuoteHistoryProcessed();
    }

    return QDateTime();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabases::movingAverages( const QString& symbol, const QDate& start, const QDate& end, QList<MovingAverages>& data ) const
{
    SymbolDatabase *child( const_cast<_Myt*>( this )->findSymbol( symbol ) );

    if ( child )
    {
        SymbolDatabaseRemoveRef deref( symbol );
        child->movingAverages( start, end, data );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabases::movingAveragesConvergenceDivergence( const QString& symbol, const QDate& start, const QDate& end, QList<MovingAveragesConvergenceDivergence>& data ) const
{
    SymbolDatabase *child( const_cast<_Myt*>( this )->findSymbol( symbol ) );

    if ( child )
    {
        SymbolDatabaseRemoveRef deref( symbol );
        child->movingAveragesConvergenceDivergence( start, end, data );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabases::optionChainCurves( const QString& symbol, const QDate& expiryDate, const QDateTime& stamp, OptionChainCurves& data ) const
{
    SymbolDatabase *child( const_cast<_Myt*>( this )->findSymbol( symbol ) );

    if ( child )
    {
        SymbolDatabaseRemoveRef deref( symbol );
        child->optionChainCurves( expiryDate, stamp, data );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSqlDatabase SymbolDatabases::openDatabaseConnection( const QString& symbol ) const
{
    SymbolDatabase *child( const_cast<_Myt*>( this )->findSymbol( symbol ) );

    if ( child )
        return child->connection();

    return QSqlDatabase();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabases::removeRef( const QString& symbol )
{
    if ( symbol.isEmpty() )
        return;

    QMutexLocker guard_( &m_ );

    // find symbol
    SymbolDatabaseMap::const_iterator i( symbols_.constFind( symbol ) );

    if ( symbols_.constEnd() != i )
        i.value()->removeRef();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabases::quoteHistoryDateRange( const QString& symbol, QDate& start, QDate& end ) const
{
    SymbolDatabase *child( const_cast<_Myt*>( this )->findSymbol( symbol ) );

    if ( child )
    {
        SymbolDatabaseRemoveRef deref( symbol );
        child->quoteHistoryDateRange( start, end );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabases::relativeStrengthIndex( const QString& symbol, const QDate& start, const QDate& end, QList<RelativeStrengthIndexes>& data ) const
{
    SymbolDatabase *child( const_cast<_Myt*>( this )->findSymbol( symbol ) );

    if ( child )
    {
        SymbolDatabaseRemoveRef deref( symbol );
        child->relativeStrengthIndex( start, end, data );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabases::setOptionChainCurves( const QString& symbol, const QDate& expiryDate, const QDateTime& stamp, const OptionChainCurves& data )
{
    SymbolDatabase *child( const_cast<_Myt*>( this )->findSymbol( symbol ) );

    if ( child )
    {
        SymbolDatabaseRemoveRef deref( symbol );
        child->setOptionChainCurves( expiryDate, stamp, data );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SymbolDatabases *SymbolDatabases::instance()
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
bool SymbolDatabases::processData( const QJsonObject& obj )
{
    const QDateTime now( AppDatabase::instance()->currentDateTime() );

    bool instrumentsProcessed( false );
    bool quoteHistoryProcessed( false );
    bool quotesProcessed( false );
    bool optionChainProcessed( false );

    QString quoteHistorySymbol;
    QStringList quoteSymbols;
    QList<QDate> optionChainExpiryDates;
    QString optionChainSymbol;

    bool result( true );

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
                {
                    SymbolDatabaseRemoveRef deref( symbol );
                    result &= child->processInstrument( now, instrument );
                }
            }

        instrumentsProcessed = result;
    }

    // process quote history
    const QJsonObject::const_iterator quoteHistoryIt( obj.constFind( DB_QUOTE_HISTORY ) );

    if (( obj.constEnd() != quoteHistoryIt ) && ( quoteHistoryIt->isObject() ))
    {
        const QJsonObject quoteHistory( quoteHistoryIt->toObject() );

        const QString symbol( quoteHistory[DB_SYMBOL].toString() );

        const QJsonObject::const_iterator history( quoteHistory.constFind( DB_HISTORY ) );

        if (( symbol.length() ) && ( history->isArray() ))
        {
            const QDateTime start( QDateTime::fromString( quoteHistory[DB_START_DATE].toString(), Qt::ISODateWithMs ) );
            const QDateTime stop( QDateTime::fromString( quoteHistory[DB_END_DATE].toString(), Qt::ISODateWithMs ) );

            const int period( quoteHistory[DB_PERIOD].toInt() );
            const QString periodType( quoteHistory[DB_PERIOD_TYPE].toString() );
            const int freq( quoteHistory[DB_FREQUENCY].toInt() );
            const QString freqType( quoteHistory[DB_FREQUENCY_TYPE].toString() );

            // for daily, process as quote history
            if ( DAILY == freqType )
            {
                SymbolDatabase *child( findSymbol( symbol ) );

                if ( child )
                {
                    SymbolDatabaseRemoveRef deref( symbol );
                    result &= child->processQuoteHistory( quoteHistory );
                }

                quoteHistoryProcessed = result;
                quoteHistorySymbol = symbol;
            }

            LOG_TRACE << "parse candles";

            // parse out candles
            QList<CandleData> candles;

            foreach ( const QJsonValue& candleVal, history->toArray() )
               if ( candleVal.isObject() )
               {
                   const QJsonObject candle( candleVal.toObject() );

                   CandleData data;
                   data.stamp = QDateTime::fromString( candle[DB_DATETIME].toString(), Qt::ISODateWithMs );

                   data.openPrice = candle[DB_OPEN_PRICE].toDouble();
                   data.highPrice = candle[DB_HIGH_PRICE].toDouble();
                   data.lowPrice = candle[DB_LOW_PRICE].toDouble();
                   data.closePrice = candle[DB_CLOSE_PRICE].toDouble();
                   data.totalVolume = candle[DB_TOTAL_VOLUME].toVariant().toULongLong();

                   // append candle
                   candles.append( data );
               }

            LOG_TRACE << "candle data changed...";

            // emit signal
            emit candleDataChanged( symbol, start, stop, period, periodType, freq, freqType, candles );
            LOG_TRACE << "candle data changed... done";
        }
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
                {
                    SymbolDatabaseRemoveRef deref( symbol );
                    result &= child->processQuote( now, quote );
                }

                quotesProcessed = result;
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
        {
            SymbolDatabaseRemoveRef deref( symbol );
            result &= child->processOptionChain( now, optionChain, optionChainExpiryDates );
        }

        optionChainProcessed = result;
        optionChainSymbol = symbol;
    }

    // EMIT SIGNALS

    if ( instrumentsProcessed )
        emit instrumentsChanged();

    if ( quoteHistoryProcessed )
        emit quoteHistoryChanged( quoteHistorySymbol );

    if ( quotesProcessed )
        emit quotesChanged( quoteSymbols );

    if ( optionChainProcessed )
        emit optionChainChanged( optionChainSymbol, optionChainExpiryDates );

    // remove app database connection
    AppDatabase::instance()->removeConnection();

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabases::onTimeout()
{
    // After a few hundred connections the database peformance starts to slow
    // down dramatically. The purpose of this timer is to find and close open
    // stale database connections.
    removeStaleDatabases();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SymbolDatabase *SymbolDatabases::findSymbol( const QString& symbol )
{
    if ( symbol.isEmpty() )
        return nullptr;

    QMutexLocker guard( &m_ );

    // find symbol
    SymbolDatabase *child( nullptr );

    SymbolDatabaseMap::const_iterator i( symbols_.constFind( symbol ) );

    if ( symbols_.constEnd() != i )
        child = i.value();
    else
    {
        // create new symbol database
        child = new SymbolDatabase( symbol, this );

        if ( !child->isReady() )
        {
            LOG_WARN << "failed to create symbol db " << qPrintable( symbol );
            delete child;

            return nullptr;
        }

        // track database
        symbols_[symbol] = child;
    }

    // add reference
    child->addRef();

    return child;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDatabases::removeStaleDatabases()
{
    QMutexLocker guard( &m_ );

    const QStringList conns( QSqlDatabase::connectionNames() );

    int removed( 0 );

    // iterate all connections
    foreach ( const QString& conn, conns )
    {
        QString symbol;

        // parse symbol name
        const int index( conn.indexOf( '_' ) );

        if ( 0 < index )
            symbol = conn.left( index );
        else
            symbol = conn;

        // find symbol
        SymbolDatabaseMap::const_iterator i( symbols_.constFind( symbol ) );

        if ( symbols_.constEnd() != i )
            if ( !i.value()->isLocked() )
            {
                // remove database connection
                LOG_TRACE << "remove database " << qPrintable( conn );
                QSqlDatabase::removeDatabase( conn );

                ++removed;
            }
    }

    LOG_DEBUG << "open conns " << (conns.size() - removed) << " (" << removed << " removed)";
}
