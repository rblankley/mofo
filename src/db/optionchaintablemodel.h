/**
 * @file optionchaintablemodel.h
 * Table model for option chains.
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

#ifndef OPTIONCHAINTABLEMODEL_H
#define OPTIONCHAINTABLEMODEL_H

#include "sqltablemodel.h"

#include <QColor>

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Table model for option chains.
class OptionChainTableModel : public SqlTableModel
{
    Q_OBJECT

    using _Myt = OptionChainTableModel;
    using _Mybase = SqlTableModel;

public:

    /// Column index values.
    enum ColumnIndex
    {
        STAMP,
        UNDERLYING,
        EXPIRY_DATE,
        // CALL
        CALL_SYMBOL,
        CALL_DESC,
        CALL_BID_ASK_SIZE,
        CALL_BID_PRICE,
        CALL_BID_SIZE,
        CALL_ASK_PRICE,
        CALL_ASK_SIZE,
        CALL_LAST_PRICE,
        CALL_LAST_SIZE,
        CALL_BREAK_EVEN_PRICE,
        CALL_INTRINSIC_VALUE,
        CALL_OPEN_PRICE,
        CALL_HIGH_PRICE,
        CALL_LOW_PRICE,
        CALL_CLOSE_PRICE,
        CALL_CHANGE,
        CALL_PERCENT_CHANGE,
        CALL_TOTAL_VOLUME,
        CALL_QUOTE_TIME,
        CALL_TRADE_TIME,
        CALL_MARK,
        CALL_MARK_CHANGE,
        CALL_MARK_PERCENT_CHANGE,
        CALL_EXCHANGE_NAME,
        CALL_VOLATILITY,
        CALL_DELTA,
        CALL_GAMMA,
        CALL_THETA,
        CALL_VEGA,
        CALL_RHO,
        CALL_TIME_VALUE,
        CALL_OPEN_INTEREST,
        CALL_IS_IN_THE_MONEY,
        CALL_THEO_OPTION_VALUE,
        CALL_THEO_VOLATILITY,
        CALL_IS_MINI,
        CALL_IS_NON_STANDARD,
        CALL_IS_INDEX,
        CALL_IS_WEEKLY,
        CALL_IS_QUARTERLY,
        CALL_EXPIRY_DATE,
        CALL_EXPIRY_TYPE,
        CALL_DAYS_TO_EXPIRY,
        CALL_LAST_TRADING_DAY,
        CALL_MULTIPLIER,
        CALL_SETTLEMENT_TYPE,
        CALL_DELIVERABLE_NOTE,
        // STRIKE
        STRIKE_PRICE,
        // PUT
        PUT_SYMBOL,
        PUT_DESC,
        PUT_BID_ASK_SIZE,
        PUT_BID_PRICE,
        PUT_BID_SIZE,
        PUT_ASK_PRICE,
        PUT_ASK_SIZE,
        PUT_LAST_PRICE,
        PUT_LAST_SIZE,
        PUT_BREAK_EVEN_PRICE,
        PUT_INTRINSIC_VALUE,
        PUT_OPEN_PRICE,
        PUT_HIGH_PRICE,
        PUT_LOW_PRICE,
        PUT_CLOSE_PRICE,
        PUT_CHANGE,
        PUT_PERCENT_CHANGE,
        PUT_TOTAL_VOLUME,
        PUT_QUOTE_TIME,
        PUT_TRADE_TIME,
        PUT_MARK,
        PUT_MARK_CHANGE,
        PUT_MARK_PERCENT_CHANGE,
        PUT_EXCHANGE_NAME,
        PUT_VOLATILITY,
        PUT_DELTA,
        PUT_GAMMA,
        PUT_THETA,
        PUT_VEGA,
        PUT_RHO,
        PUT_TIME_VALUE,
        PUT_OPEN_INTEREST,
        PUT_IS_IN_THE_MONEY,
        PUT_THEO_OPTION_VALUE,
        PUT_THEO_VOLATILITY,
        PUT_IS_MINI,
        PUT_IS_NON_STANDARD,
        PUT_IS_INDEX,
        PUT_IS_WEEKLY,
        PUT_IS_QUARTERLY,
        PUT_EXPIRY_DATE,
        PUT_EXPIRY_TYPE,
        PUT_DAYS_TO_EXPIRY,
        PUT_LAST_TRADING_DAY,
        PUT_MULTIPLIER,
        PUT_SETTLEMENT_TYPE,
        PUT_DELIVERABLE_NOTE,

        _NUM_COLUMNS,

        _CALL_COLUMNS_BEGIN = CALL_SYMBOL,
        _CALL_COLUMNS_END = CALL_DELIVERABLE_NOTE,
        _CALL_WIDTH = _CALL_COLUMNS_END - _CALL_COLUMNS_BEGIN + 1,
        _PUT_COLUMNS_BEGIN = PUT_SYMBOL,
        _PUT_COLUMNS_END = PUT_DELIVERABLE_NOTE,
        _PUT_WIDTH = _PUT_COLUMNS_END - _PUT_COLUMNS_BEGIN + 1,
    };

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] symbol  underlying symbol
     * @param[in] expiryDate  expiration date
     * @param[in] stamp  date time
     * @param[in] parent  parent object
     */
    OptionChainTableModel( const QString& symbol, const QDate& expiryDate, const QDateTime& stamp = QDateTime(), QObject *parent = nullptr );

    /// Destructor.
    virtual ~OptionChainTableModel();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve table data.
    /**
     * @param[in] row  row
     * @param[in] col  column
     * @param[in] role  role
     * @return  data
     */
    virtual QVariant data( int row, int col, int role = Qt::DisplayRole ) const override {return _Mybase::data( row, col, role );}

    /// Retrieve data for role.
    /**
     * @param[in] index  index
     * @param[in] role  role
     * @return  data value
     */
    virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override;

    /// Retrieve expiration date.
    /**
     * @return  expiration date
     */
    virtual QDate expirationDate() const {return expiryDate_;}

    /// Retrieve flags.
    /**
     * @param[in] index  index
     * @return  flags
     */
    virtual Qt::ItemFlags flags( const QModelIndex& index ) const override;

    /// Check if column is call option.
    /**
     * @param[in] col  column
     * @return  @c true if call option, @c false otherwise
     */
    virtual bool isColumnCallOption( ColumnIndex col ) const {return (( _CALL_COLUMNS_BEGIN <= col ) && ( col <= _CALL_COLUMNS_END ));}

    /// Check if column is put option.
    /**
     * @param[in] col  column
     * @return  @c true if put option, @c false otherwise
     */
    virtual bool isColumnPutOption( ColumnIndex col ) const {return (( _PUT_COLUMNS_BEGIN <= col ) && ( col <= _PUT_COLUMNS_END ));}

    /// Retrieve mapped call/put column.
    /**
     * @param[in] col  column index
     * @return  mapped column index
     */
    virtual ColumnIndex mappedColumn( ColumnIndex col ) const;

    /// Retrieve symbol.
    /**
     * @return  symbol
     */
    virtual QString symbol() const {return symbol_;}

    /// Retrieve table data.
    /**
     * @param[in] row  row
     * @param[in] col  column
     * @param[in] role  role
     * @return  data
     */
    virtual QVariant tableData( int row, ColumnIndex col, int role = Qt::DisplayRole ) const {return _Mybase::data( row, col, role );}

protected:

    QString symbol_;                                ///< Underlying symbol.
    QDate expiryDate_;                              ///< Option expiration date.

private:

    QMap<int, int> bidAskSize_;

    QColor inTheMoneyColor_;
    QColor strikeColor_;

    QColor textColor_;

    // not implemented
    OptionChainTableModel( const _Myt& ) = delete;

    // not implemented
    OptionChainTableModel( const _Myt&& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // OPTIONCHAINTABLEMODEL_H
