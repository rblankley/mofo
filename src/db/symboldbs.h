/**
 * @file symboldbs.h
 * Symbol Databases.
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

#ifndef SYMBOLDBS_H
#define SYMBOLDBS_H

#include "candledata.h"
#include "optiondata.h"

#include <QDate>
#include <QDateTime>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSqlDatabase>

class SymbolDatabase;

class QJsonObject;
class QTimer;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Symbol Databases.
class SymbolDatabases : public QObject
{
    Q_OBJECT

    using _Myt = SymbolDatabases;
    using _Mybase = QObject;

signals:

    /// Signal for when candle data changed.
    /**
     * @param[in] symbol  symbol
     * @param[in] start  graph start date/time
     * @param[in] stop  graph stop date/time
     * @param[in] period  history period
     * @param[in] periodType  period type (day, month, year, or ytd)
     * @param[in] freq  frequency
     * @param[in] freqType  frequency type (minute, daily, weekly, or monthly)
     * @param[in] candles  list of candle data
     */
    void candleDataChanged( const QString& symbol, const QDateTime& start, const QDateTime& stop, int period, const QString& periodType, int freq, const QString& freqType, const QList<CandleData>& candles );

    /// Signal for when instruments changed.
    void instrumentsChanged();

    /// Signal for when option chains have changed.
    /**
     * @param[in] symbol  symbol
     * @param[in] expiryDates  expiration dates
     */
    void optionChainChanged( const QString& symbol, const QList<QDate>& expiryDates );

    /// Signal for when quote history has changed.
    /**
     * @param[in] symbol  symbol
     */
    void quoteHistoryChanged( const QString& symbol );

    /// Signal for when quotes have changed.
    /**
     * @param[in] symbols  symbols
     */
    void quotesChanged( const QStringList& symbols );

public:

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve cusip for symbol.
    /**
     * @param[in] symbol  symbol
     * @return  cusip or empty string upon failure
     */
    QString cusip( const QString& symbol ) const;

    /// Retrieve description for symbol.
    /**
     * @param[in] symbol  symbol
     * @return  description or empty string upon failure
     */
    QString description( const QString& symbol ) const;

    /// Retrieve dividend date and amount.
    /**
     * @param[in] symbol  symbol
     * @param[out] date  dividend date
     * @param[out] frequency  dividend frequency (annualized)
     * @return  dividend amount (yearly)
     */
    double dividendAmount( const QString& symbol, QDate& date, double& frequency ) const;

    /// Retrieve dividend yield.
    /**
     * @param[in] symbol  symbol
     * @return  dividend yield (yearly)
     */
    double dividendYield( const QString& symbol ) const;

    /// Retrieve historical volatility.
    /**
     * @param[in] symbol  symbol to retrieve
     * @param[in] date  date to retrieve
     * @param[in] depth  depth in days
     * @return  historical volatility
     */
    double historicalVolatility( const QString& symbol, const QDate& date, int depth ) const;

    /// Retrieve historical volatility range.
    /**
     * @param[in] symbol  symbol to retrieve
     * @param[in] start  starting date to retrieve
     * @param[in] end  ending date to retrieve
     * @param[in] depth  depth in days
     * @param[out] min  minimum historic volatility
     * @param[out] min  minimum historic volatility
     */
    void historicalVolatilityRange( const QString& symbol, const QDate& start, const QDate& end, int depth, double& min, double& max ) const;

    /// Retrieve historical volatilities
    /**
     * @param[in] symbol  symbol to retrieve
     * @param[in] start  starting date to retrieve
     * @param[in] end  ending date to retrieve
     * @param[out] data  volatilities
     */
    void historicalVolatilities( const QString& symbol, const QDate& start, const QDate& end, QList<HistoricalVolatilities>& data ) const;

    /// Retrieve last fundamental processed stamp.
    /**
     * @param[in] symbol  symbol
     * @return  stamp of last fundamental processed
     */
    QDateTime lastFundamentalProcessed( const QString& symbol ) const;

    /// Retrieve last quote history processed stamp.
    /**
     * @param[in] symbol  symbol
     * @return  stamp of last quote history processed
     */
    QDateTime lastQuoteHistoryProcessed( const QString& symbol ) const;

    /// Retrieve moving averages.
    /**
     * @param[in] symbol  symbol
     * @param[in] start  starting date to retrieve
     * @param[in] end  ending date to retrieve
     * @param[out] data  moving averages
     */
    void movingAverages( const QString& symbol, const QDate& start, const QDate& end, QList<MovingAverages>& data ) const;

    /// Retrieve moving averages convergence/divergence (MACD)
    /**
     * @param[in] symbol  symbol
     * @param[in] start  starting date to retrieve
     * @param[in] end  ending date to retrieve
     * @param[out] data  MACD data
     */
    void movingAveragesConvergenceDivergence( const QString& symbol, const QDate& start, const QDate& end, QList<MovingAveragesConvergenceDivergence>& data ) const;

    /// Retrieve option chain curve expiration dates.
    /**
     * @param[in] symbol  symbol
     * @param[out] expiryDates  option chain expiration dates
     * @return  stamp of most recent curve calculation
     */
    QDateTime optionChainCurveExpirationDates( const QString& symbol, QList<QDate>& expiryDates ) const;

    /// Retrieve option chain curves.
    /**
     * @param[in] symbol  symbol
     * @param[in] expiryDate  option chain expiration date
     * @param[in] stamp  option chain stamp
     * @param[out] data  curve data
     */
    void optionChainCurves( const QString& symbol, const QDate& expiryDate, const QDateTime& stamp, OptionChainCurves& data ) const;

    /// Retrieve quote history date range.
    /**
     * @param[in] symbol  symbol
     * @param[out] start  start date
     * @param[out] end  end date
     */
    void quoteHistoryDateRange( const QString& symbol, QDate& start, QDate& end ) const;

    /// Retrieve RSI.
    /**
     * @param[in] symbol  symbol
     * @param[in] start  starting date to retrieve
     * @param[in] end  ending date to retrieve
     * @param[out] data  RSI values
     */
    void relativeStrengthIndex( const QString& symbol, const QDate& start, const QDate& end, QList<RelativeStrengthIndexes>& data ) const;

    /// Set option chain curves.
    /**
     * @param[in] symbol  symbol
     * @param[in] expiryDate  option chain expiration date
     * @param[in] stamp  option chain stamp
     * @param[out] data  curve data
     */
    void setOptionChainCurves( const QString& symbol, const QDate& expiryDate, const QDateTime& stamp, const OptionChainCurves& data );

    // ========================================================================
    // Methods
    // ========================================================================

    /// Open database connection for symbol.
    /**
     * @warning
     * This method will leave an open reference to the symbol database, you must remove this manually.
     * @param[in]  symbol
     * @return  connection to database
     */
    QSqlDatabase openDatabaseConnection( const QString& symbol ) const;

    /// Remove reference to symbol database.
    /**
     * @param[in] symbol  symbol
     */
    void removeRef( const QString& symbol );

    // ========================================================================
    // Static Methods
    // ========================================================================

    /// Retrieve global instance.
    /**
     * @return  pointer to instance
     */
    static _Myt *instance();

public slots:

    // ========================================================================
    // Methods
    // ========================================================================

    /// Process object to database.
    /**
     * @param[in] obj  data
     * @return  @c true upon success, @c false otherwise
     */
    bool processData( const QJsonObject& obj );

private slots:

    /// Slot for timeout.
    void onTimeout();

private:

    using SymbolDatabaseMap = QMap<QString, SymbolDatabase*>;

    static constexpr int REMOVE_DB_TIME = 60 * 1000;        // 60s

    static QMutex instanceMutex_;
    static _Myt *instance_;

#if QT_VERSION_CHECK( 5, 14, 0 ) <= QT_VERSION
    mutable QRecursiveMutex m_;
#else
    mutable QMutex m_;
#endif

    SymbolDatabaseMap symbols_;

    QTimer *cleanup_;

    /// Constructor.
    SymbolDatabases();

    /// Destructor.
    ~SymbolDatabases();

    /// Find symbol database.
    SymbolDatabase *findSymbol( const QString& symbol );

    /// Remove stale databases.
    void removeStaleDatabases();

    // not implemented
    SymbolDatabases( const _Myt& ) = delete;

    // not implemented
    SymbolDatabases( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

/// Symbol Database Remove Reference RAII Helper.
class SymbolDatabaseRemoveRef
{
public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] symbol  symbol
     */
    SymbolDatabaseRemoveRef( const QString& symbol ) : symbol_( symbol ) {}

    /// Destructor.
    virtual ~SymbolDatabaseRemoveRef() {SymbolDatabases::instance()->removeRef( symbol_ );}

private:

    QString symbol_;

};

Q_DECLARE_METATYPE( QList<CandleData> );

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // SYMBOLDBS_H
