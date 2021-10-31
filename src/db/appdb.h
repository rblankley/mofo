/**
 * @file appdb.h
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

#ifndef APPDB_H
#define APPDB_H

#include "sqldb.h"

#include <QColor>
#include <QDate>
#include <QDateTime>
#include <QList>
#include <QMutex>

class SymbolDatabase;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Application Database.
class AppDatabase : public SqlDatabase
{
    Q_OBJECT
    Q_PROPERTY( QStringList accounts READ accounts NOTIFY accountsChanged STORED true )
    Q_PROPERTY( QDateTime currentDateTime READ currentDateTime WRITE setCurrentDateTime )
    Q_PROPERTY( QStringList marketTypes READ marketTypes STORED true )

    using _Myt = AppDatabase;
    using _Mybase = SqlDatabase;

signals:

    /// Signal for when accounts changed.
    void accountsChanged();

    /// Signal for when configuration changed.
    void configurationChanged();

    /// Signal for when market hours changed.
    void marketHoursChanged();

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

    /// Signal for when treasury bill rates changed.
    void treasuryBillRatesChanged();

    /// Signal for when treasury yield curve rates changed.
    void treasuryYieldCurveRatesChanged();

public:

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve accounts.
    virtual QStringList accounts() const;

    /// Retrieve configuration.
    /**
     * @return  configs
     */
    virtual QJsonObject configs() const;

    /// Retrieve current date time.
    /**
     * @return  stamp
     */
    virtual QDateTime currentDateTime() const;

    /// Retrieve filter.
    /**
     * @param[in] name  filter name
     * @return  filter
     */
    virtual QByteArray filter( const QString& name ) const;

    /// Retrieve filters.
    virtual QStringList filters() const;

    /// Retrieve header state.
    /**
     * @param[in] name  header name
     * @return  state
     */
    virtual QByteArray headerState( const QString& name ) const;

    /// Retrieve historical volatility.
    /**
     * @param[in] symbol  symbol to retrieve
     * @param[in] dt  date to retrieve
     * @param[in] depth  depth in days
     * @return  historical volatility
     */
    virtual double historicalVolatility( const QString& symbol, const QDateTime& dt, int depth ) const;

    /// Check if market is open.
    /**
     * @param[in] dt  datetime
     * @param[in] marketType  market type
     * @param[in] product  product
     * @param[out] isExtended  set to @c true if exteded hours, @c false otherwise
     * @return  @c true if open, @c flase otherwise
     */
    virtual bool isMarketOpen( const QDateTime& dt, const QString& marketType, const QString& product = QString(), bool *isExtended = nullptr ) const;

    /// Retrieve last quote history processed stamp.
    /**
     * @param[in] symbol  symbol
     * @return  stamp of last quote history processed
     */
    virtual QDateTime lastQuoteHistoryProcessed( const QString& symbol ) const;

    /// Check if market hours exist.
    /**
     * @param[in] date  date
     * @param[in] marketType  market type, or empty string for any market type
     * @return  @c true if market hours exist, @c false otherwise
     */
    virtual bool marketHoursExist( const QDate& date, const QString& marketType = QString() ) const;

    /// Retrieve market types.
    /**
     * @param[in] hasHours  @c true for only markets with hours of operation, @c false otherwise
     * @return  list of market types
     */
    virtual QStringList marketTypes( bool hasHours = true ) const;

    /// Retrieve number of regular days in a year.
    /**
     * @return  num days
     */
    virtual double numDays() const {return numDays_;}

    /// Retrieve number of trading days in a year.
    /**
     * @return  num days
     */
    virtual double numTradingDays() const {return numTradingDays_;}

    /// Retrieve filter to use for option analysis.
    /**
     * @return  filter name
     */
    virtual QString optionAnalysisFilter() const {return optionAnalysisFilter_;}

    /// Retrieve watch lists to use for option analysis.
    /**
     * @return  watch lists
     */
    virtual QString optionAnalysisWatchLists() const {return optionAnalysisWatchLists_;}

    /// Retrieve option calc method.
    /**
     * @return  method
     */
    virtual QString optionCalcMethod() const {return optionCalcMethod_;}

    /// Retrieve option trade cost.
    /**
     * @return  cost
     */
    virtual double optionTradeCost() const {return optionTradeCost_;}

    /// Retrieve application palette.
    /**
     * @return  palette name
     */
    virtual QString palette() const {return palette_;}

    /// Retrieve application palette highlight color.
    /**
     * @return  palette highlight color
     */
    virtual QColor paletteHighlight() const {return paletteHighlight_;}

    /// Retrieve quote history date range.
    /**
     * @param[in] symbol  symbol
     * @param[out] start  start date
     * @param[out] end  end date
     */
    virtual void quoteHistoryDateRange( const QString& symbol, QDate& start, QDate& end ) const;

    /// Retrieve risk free interest rate.
    /**
     * @warning
     * Requested period @p term must be <= 30.0
     * @param[in] term  term (years)
     * @return  interest rate
     */
    virtual double riskFreeRate( double term ) const;

    /// Set configuration.
    /**
     * @param[in] value  configs
     */
    virtual void setConfigs( const QJsonObject& value );

    /// Set current date time.
    /**
     * @param[in] value  stamp
     */
    virtual void setCurrentDateTime( const QDateTime& value ) {now_ = value;}

    /// Set filter.
    /**
     * @param[in] name  filter name
     * @param[in] value  filter data
     */
    virtual void setFilter( const QString& name, const QByteArray& value = QByteArray() );

    /// Set header state.
    /**
     * @param[in] name  header name
     * @param[in] state  state
     */
    virtual void setHeaderState( const QString& name, const QByteArray& state );

    /// Set watchlist.
    /**
     * @param[in] name  watchlist name
     * @param[in] symbols  symbols
     */
    virtual void setWatchlist( const QString& name, const QStringList& symbols );

    /// Retrieve treasury yield curve date range.
    /**
     * @param[out] start  start date
     * @param[out] end  end date
     */
    virtual void treasuryYieldCurveDateRange( QDate& start, QDate& end ) const;

    /// Retrieve watchlist.
    /**
     * @param[in] name  watchlist name
     * @return  list of stock symbols
     */
    virtual QStringList watchlist( const QString& name ) const;

    /// Retrieve watchlists.
    /**
     * @param[in] includeIndices  @c true to include market indices, @c false otherwise
     * @return  list of watchlists
     */
    virtual QStringList watchlists( bool includeIndices = true ) const;

    // ========================================================================
    // Methods
    // ========================================================================

    /// Open database connection.
    /**
     * @return  connection to database
     */
    virtual QSqlDatabase openDatabaseConnection() const override {return _Mybase::openDatabaseConnection();}

    /// Open database connection for symbol.
    /**
     * @param[in]  symbol
     * @return  connection to database
     */
    virtual QSqlDatabase openDatabaseConnection( const QString& symbol ) const;

    /// Remove filter.
    /**
     * @param[in] name  filter name
     */
    virtual void removeFilter( const QString& name );

    /// Remove watchlist.
    /**
     * @param[in] name  watchlist name
     */
    virtual void removeWatchlist( const QString& name );

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
    virtual bool processData( const QJsonObject& obj );

protected:

    double optionTradeCost_;                        ///< Option trade cost.
    QString optionCalcMethod_;                      ///< Option calc method.

    QString optionAnalysisWatchLists_;              ///< Watchlists to use for option analysis.
    QString optionAnalysisFilter_;                  ///< Filter to use for option analysis.

    double numTradingDays_;                         ///< Number of trading days.
    double numDays_;                                ///< Number of days.

    QString palette_;                               ///< Application palette.
    QColor paletteHighlight_;                       ///< Application palette highlight color.

    QDateTime now_;                                 ///< Current date time (or empty to use actual).

    QStringList configs_;                           ///< Configuration key values.

    // ========================================================================
    // Properties
    // ========================================================================

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

    /// Add account to database.
    /**
     * @param[in] stamp  timestamp
     * @param[in] obj  data
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool addAccount( const QDateTime& stamp, const QJsonObject& obj );

    /// Add account balances to database.
    /**
     * @param[in] stamp  timestamp
     * @param[in] accountId  account id
     * @param[in] type  balance type
     * @param[in] obj  data
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool addAccountBalances( const QDateTime& stamp, const QString& accountId, const QString& type, const QJsonObject& obj );

    /// Add market hours to database.
    /**
     * @param[in] obj  data
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool addMarketHours( const QJsonObject& obj );

    /// Add product type to database.
    /**
     * @param[in] type  type
     * @param[in] description  description
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool addProductType( const QString& type, const QString& description );

    /// Add session hours to database.
    /**
     * @param[in] date  date
     * @param[in] marketType  market type
     * @param[in] product  product
     * @param[in] sessionHoursType  session hours type
     * @param[in] obj  data
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool addSessionHours( const QDate& date, const QString& marketType, const QString& product, const QString& sessionHoursType, const QJsonObject& obj );

    /// Add treasury bill rate to database.
    /**
     * @param[in] stamp  timestamp
     * @param[in] obj  data
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool addTreasuryBillRate( const QDateTime& stamp, const QJsonObject& obj );

    /// Add treasury yield curve rate to database.
    /**
     * @param[in] stamp  timestamp
     * @param[in] obj  data
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool addTreasuryYieldCurveRate( const QDateTime& stamp, const QJsonObject& obj );

private:

    static QMutex instanceMutex_;
    static _Myt *instance_;

    /// Constructor.
    AppDatabase();

    /// Destructor.
    ~AppDatabase();

    /// Read settings from database.
    void readSettings();

    /// Parse account balances.
    bool parseAccountBalances( const QDateTime& stamp, const QString& accountId, const QJsonObject& obj );

    /// Parse session hours.
    bool parseSessionHours( const QDate& date, const QString& marketType, const QString& product, const QJsonObject& obj );

    /// Find symbol database.
    SymbolDatabase *findSymbol( const QString& symbol );

    /// Check for session hours.
    bool checkSessionHours( const QDateTime& dt, const QString& marketType, const QString& product, bool *isExtended ) const;

    /// Check for extended hours.
    bool isExtendedHours( const QString& sessionHoursType ) const;

    // not implemented
    AppDatabase( const _Myt& ) = delete;

    // not implemented
    AppDatabase( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // APPDB_H
