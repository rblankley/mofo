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

#include "sqldb.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Symbol History Database.
class SymbolDatabase : public SqlDatabase
{
    Q_OBJECT

    using _Myt = SymbolDatabase;
    using _Mybase = SqlDatabase;

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

    /// Retrieve historical volatility.
    /**
     * @param[in] dt  date to retrieve
     * @param[in] depth  depth in days
     * @return  historical volatility
     */
    virtual double historicalVolatility( const QDateTime& dt, int depth ) const;

    /// Retrieve last quote history processed stamp.
    /**
     * @return  stamp of last quote history processed
     */
    virtual QDateTime lastQuoteHistoryProcessed() const;

    /// Retrieve quote history date range.
    /**
     * @param[out] start  start date
     * @param[out] end  end date
     */
    virtual void quoteHistoryDateRange( QDate& start, QDate& end ) const;

    /// Retrieve symbol.
    /**
     * @return  stock symbol
     */
    virtual QString symbol() const {return symbol_;}

public slots:

    // ========================================================================
    // Methods
    // ========================================================================

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

    /// Add option to database.
    /**
     * @param[in] obj  data
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool addOption( const QJsonObject& obj );

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

private:

    /// Calculate historical volatility (standard deviation).
    void calcHistoricalVolatility();

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
