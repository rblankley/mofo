/**
 * @file symboldb.h
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

#ifndef SYMBOLDB_H
#define SYMBOLDB_H

#include "candledata.h"
#include "optiondata.h"
#include "sqldb.h"

#include <QDate>
#include <QMutex>

class SymbolDatabases;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Symbol History Database.
class SymbolDatabase : public SqlDatabase
{
    Q_OBJECT
    Q_PROPERTY( QString cusip READ cusip STORED true )
    Q_PROPERTY( QString description READ description STORED true )
    Q_PROPERTY( double dividendYield READ dividendYield STORED true )
    Q_PROPERTY( QDateTime lastFundamentalProcessed READ lastFundamentalProcessed STORED true )
    Q_PROPERTY( QDateTime lastQuoteHistoryProcessed READ lastQuoteHistoryProcessed STORED true )
    Q_PROPERTY( bool locked READ isLocked )
    Q_PROPERTY( QString symbol READ symbol )

    using _Myt = SymbolDatabase;
    using _Mybase = SqlDatabase;

    friend class SymbolDatabases;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] symbol  stock symbol
     * @param[in] parent  parent
     */
    SymbolDatabase( const QString& symbol, QObject *parent = nullptr );

    /// Destructor.
    ~SymbolDatabase();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve cusip.
    /**
     * @return  cusip or empty string if not known.
     */
    virtual QString cusip() const;

    /// Retrieve description.
    /**
     * @return  description or empty string if not known.
     */
    virtual QString description() const;

    /// Retrieve dividend date and amount.
    /**
     * @param[out] date  dividend date
     * @param[out] frequency  dividend frequency (annualized)
     * @return  dividend amount (yearly)
     */
    virtual double dividendAmount( QDate& date, double& frequency ) const;

    /// Retrieve dividend yield.
    /**
     * @return  dividend yield (yearly)
     */
    virtual double dividendYield() const;

    /// Retrieve future volatility.
    /**
     * @param[out] data  map of volatility information by date
     * @param[in] start  starting date/time (optional)
     * @param[in] end  ending date/time (optional)
     */
    virtual void futureVolatility( QMap<QDate, FutureVolatilities>& data, const QDateTime& start = QDateTime(), const QDateTime& end = QDateTime() ) const;

    /// Retrieve historical volatility.
    /**
     * @param[in] date  date to retrieve
     * @param[in] depth  depth in days
     * @return  historical volatility
     */
    virtual double historicalVolatility( const QDate& date, int depth ) const;

    /// Retrieve historical volatility range.
    /**
     * @param[in] start  starting date to retrieve
     * @param[in] end  ending date to retrieve
     * @param[in] depth  depth in days
     * @param[out] min  minimum historic volatility
     * @param[out] min  minimum historic volatility
     */
    virtual void historicalVolatilityRange( const QDate& start, const QDate& end, int depth, double& min, double& max ) const;

    /// Retrieve historical volatilities
    /**
     * @param[in] start  starting date to retrieve
     * @param[in] end  ending date to retrieve
     * @param[out] data  volatilities
     */
    virtual void historicalVolatilities( const QDate& start, const QDate& end, QList<HistoricalVolatilities>& data ) const;

    /// Check if symbol is in use (references exist).
    /**
     * @return  @c true if locked, @c false otherwise
     */
    virtual bool isLocked() const;

    /// Retrieve last fundamental processed stamp.
    /**
     * @return  stamp of last fundamental processed
     */
    virtual QDateTime lastFundamentalProcessed() const;

    /// Retrieve last quote history processed stamp.
    /**
     * @return  stamp of last quote history processed
     */
    virtual QDateTime lastQuoteHistoryProcessed() const;

    /// Retrieve moving averages.
    /**
     * @param[in] start  starting date to retrieve
     * @param[in] end  ending date to retrieve
     * @param[out] data  moving averages
     */
    virtual void movingAverages( const QDate& start, const QDate& end, QList<MovingAverages>& data ) const;

    /// Retrieve moving average convergence/divergence (MACD)
    /**
     * @param[in] start  starting date to retrieve
     * @param[in] end  ending date to retrieve
     * @param[out] data  MACD data
     */
    virtual void movingAveragesConvergenceDivergence( const QDate& start, const QDate& end, QList<MovingAveragesConvergenceDivergence>& data ) const;

    /// Retrieve option chain curve expiration dates.
    /**
     * @param[out] expiryDates  option chain expiration dates
     * @param[in] start  starting date/time (optional)
     * @param[in] end  ending date/time (optional)
     * @return  stamp of most recent curve calculation within period [start, end]
     */
    virtual QDateTime optionChainCurveExpirationDates( QList<QDate>& expiryDates, const QDateTime& start = QDateTime(), const QDateTime& end = QDateTime() ) const;

    /// Retrieve option chain curves.
    /**
     * @param[in] expiryDate  option chain expiration date
     * @param[out] data  curve data
     * @param[in] start  starting date/time (optional)
     * @param[in] end  ending date/time (optional)
     * @return  stamp of most recent curve calculation within period [start, end]
     */
    virtual QDateTime optionChainCurves( const QDate& expiryDate, OptionChainCurves& data, const QDateTime& start = QDateTime(), const QDateTime& end = QDateTime() ) const;

    /// Retrieve option chain open interest
    /**
     * @param[in] expiryDate  option chain expiration date
     * @param[out] data  open interest
     * @param[in] start  starting date/time (optional)
     * @param[in] end  ending date/time (optional)
     * @return  stamp of most recent open interest data within period [start, end]
     */
    virtual QDateTime optionChainOpenInterest( const QDate& expiryDate, OptionChainOpenInterest& data, const QDateTime& start = QDateTime(), const QDateTime& end = QDateTime() ) const;

    /// Retrieve quote history date range.
    /**
     * @param[out] start  start date
     * @param[out] end  end date
     */
    virtual void quoteHistoryDateRange( QDate& start, QDate& end ) const;

    /// Retrieve RSI.
    /**
     * @param[in] start  starting date to retrieve
     * @param[in] end  ending date to retrieve
     * @param[out] data  RSI values
     */
    virtual void relativeStrengthIndex( const QDate& start, const QDate& end, QList<RelativeStrengthIndexes>& data ) const;

    /// Set option chain curves.
    /**
     * @param[in] expiryDate  option chain expiration date
     * @param[in] stamp  option chain stamp
     * @param[in] data  curve data
     */
    virtual void setOptionChainCurves( const QDate& expiryDate, const QDateTime& stamp, const OptionChainCurves& data );

    /// Retrieve symbol.
    /**
     * @return  stock symbol
     */
    virtual QString symbol() const {return symbol_;}

    // ========================================================================
    // Methods
    // ========================================================================

    /// Add reference to symbol.
    virtual void addRef();

    /// Remove reference to symbol.
    virtual void removeRef();

public slots:

    // ========================================================================
    // Methods
    // ========================================================================

    /// Process instrument to database.
    /**
     * @param[in] stamp  data time
     * @param[in] obj  data
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool processInstrument( const QDateTime& stamp, const QJsonObject& obj );

    /// Process option chain to database.
    /**
     * @param[in] stamp  data time
     * @param[in] obj  data
     * @param[out] expiryDates  expiration dates added
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool processOptionChain( const QDateTime& stamp, const QJsonObject& obj, QList<QDate>& expiryDates );

    /// Process quote to database.
    /**
     * @param[in] stamp  data time
     * @param[in] obj  data
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool processQuote( const QDateTime& stamp, const QJsonObject& obj );

    /// Process quote history to database.
    /**
     * @param[in] obj  data
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool processQuoteHistory( const QJsonObject& obj );

protected:

    QString symbol_;                                ///< Stock symbol.

    double divAmount_;                              ///< Dividend amount (per year).
    double divYield_;                               ///< Dividend yield (per year).

    QDate divDate_;                                 ///< Dividend date.
    QString divFrequency_;                          ///< Dividend frequency.

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve database connection name.
    /**
     * Use symbol name as connection name.
     * @return  connection name
     */
    virtual QString connectionName() const override {return symbol_;}

    /// Retrieve sql files to create database.
    /**
     * @return  list of files to parse
     */
    virtual QStringList createFiles() const override;

    /// Retrieve sql files to upgrade database.
    /**
     * @param[in] from  current database version
     * @param[in] to  desired database version
     * @return  list of files to parse
     */
    virtual QStringList upgradeFiles( const QString& from, const QString& to ) const override;

    // ========================================================================
    // Methods
    // ========================================================================

    /// Add fundamental to database.
    /**
     * @param[in] stamp  date time
     * @param[in] obj  data
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool addFundamental( const QDateTime& stamp, const QJsonObject& obj );

    /// Add option to database.
    /**
     * @param[in] obj  data
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool addOption( const QJsonObject& obj );

    /// Add option to database using prepared query.
    /**
     * @param[in] obj  data
     * @param[in] query  prepared query
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool addOption( const QJsonObject& obj, QSqlQuery& query );

    /// Add option chain to database.
    /**
     * @param[in] stamp  date time
     * @param[in] obj  data
     * @param[out] expiryDates  expiration dates added
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool addOptionChain( const QDateTime& stamp, const QJsonObject& obj, QList<QDate>& expiryDates );

    /// Add option chain strike price to database.
    /**
     * @param[in] stamp  date time
     * @param[in] optionStamp  option date time
     * @param[in] optionSymbol  option symbol
     * @param[in] type  option type
     * @param[in] expiryDate  expiration date
     * @param[in] strikePrice  strike price
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool addOptionChainStrikePrice( const QDateTime& stamp, const QString& optionStamp, const QString& optionSymbol, const QString& type, const QString& expiryDate, double strikePrice );

    /// Add option chain strike price to database using prepared query
    /**
     * @param[in] stamp  date time
     * @param[in] optionStamp  option date time
     * @param[in] optionSymbol  option symbol
     * @param[in] expiryDate  expiration date
     * @param[in] strikePrice  strike price
     * @param[in] query  prepared query
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool addOptionChainStrikePrice( const QDateTime& stamp, const QString& optionStamp, const QString& optionSymbol, const QString& expiryDate, double strikePrice, QSqlQuery& query );

    /// Add quote information.
    /**
     * @param[in] obj  data
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool addQuote( const QJsonObject& obj );

    /// Add quote history.
    /**
     * @param[in] obj  data
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool addQuoteHistory( const QJsonObject& obj );

    /// Write setting to db.
    /**
     * @param[in] key  setting to write
     * @param[in] value  setting value
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool writeSetting( const QString& key, const QVariant& value ) override;

private:

    static const int FORCED_UPDATE = 5;

#if QT_VERSION_CHECK( 5, 14, 0 ) <= QT_VERSION
    mutable QRecursiveMutex m_;
#else
    mutable QMutex m_;
#endif

    int ref_;

    /// Retrieve number of rows in quote history.
    int quoteHistoryRowCount() const;

    /// Update option chain curve data.
    void updateOptionChainCurves( const QDateTime& stamp );

    /// Retrieve list of option expiration dates.
    QList<QDate> optionExpirationDates( const QDateTime& dt ) const;

    /// Calculate historical volatility (standard deviation).
    void calcHistoricalVolatility();

    /// Calculate moving averages.
    void calcMovingAverage();

    /// Calculate relative strength index.
    void calcRelativeStrengthIndex();

    /// Calculate moving average convergence/divergence (MACD).
    void calcMovingAverageConvergenceDivergence();

    /// Calculate dividend frequency.
    void calcDividendFrequencyFromDate( const QJsonValue& date );

    /// Calculate dividend frequency.
    void calcDividendFrequencyFromPayAmount( const QJsonValue& payAmount, const QJsonValue& amount );

    /// Calculate dividend frequency.
    void calcDividendFrequencyFromPayDate( const QJsonValue& date );

    // not implemented
    SymbolDatabase( const _Myt& ) = delete;

    // not implemented
    SymbolDatabase( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // SYMBOLDB_H
