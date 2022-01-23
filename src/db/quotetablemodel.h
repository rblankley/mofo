/**
 * @file quotetablemodel.h
 * Table model for symbol quote.
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

#ifndef QUOTETABLEMODEL_H
#define QUOTETABLEMODEL_H

#include "sqltablemodel.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Table model for for symbol quote.
class QuoteTableModel : public SqlTableModel
{
    Q_OBJECT

    using _Myt = QuoteTableModel;
    using _Mybase = SqlTableModel;

public:

    /// Column index values.
    enum ColumnIndex
    {
        STAMP,
        SYMBOL,
        DESCRIPTION,
        ASSET_MAIN_TYPE,
        ASSET_SUB_TYPE,
        ASSET_TYPE,
        CUSIP,
        BID_ASK_SIZE,
        BID_PRICE,
        BID_SIZE,
        BID_ID,
        BID_TICK,
        ASK_PRICE,
        ASK_SIZE,
        ASK_ID,
        LAST_PRICE,
        LAST_SIZE,
        LAST_ID,
        OPEN_PRICE,
        HIGH_PRICE,
        LOW_PRICE,
        CLOSE_PRICE,
        CHANGE,
        PERCENT_CHANGE,
        TOTAL_VOLUME,
        QUOTE_TIME,
        TRADE_TIME,
        MARK,
        MARK_CHANGE,
        MARK_PERCENT_CHANGE,
        FIFTY_TWO_WEEK_HIGH,
        FIFTY_TWO_WEEK_LOW,
        PERCENT_BELOW_FIFTY_TWO_WEEK_HIGH,
        PERCENT_ABOVE_FIFTY_TWO_WEEK_LOW,
        FIFTY_TWO_WEEK_PRICE_RANGE,
        EXCHANGE,
        EXCHANGE_NAME,
        IS_MARGINABLE,
        IS_SHORTABLE,
        IS_DELAYED,
        VOLATILITY,
        DIGITS,
        NAV,
        PE_RATIO,
        IMPLIED_YIELD,
        DIV_AMOUNT,
        DIV_YIELD,
        DIV_DATE,
        DIV_FREQUENCY,
        SECURITY_STATUS,
        REG_MARKET_LAST_PRICE,
        REG_MARKET_LAST_SIZE,
        REG_MARKET_CHANGE,
        REG_MARKET_PERCENT_CHANGE,
        REG_MARKET_TRADE_TIME,
        // Forex
        TICK,
        TICK_AMOUNT,
        PRODUCT,
        TRADING_HOURS,
        IS_TRADABLE,
        MARKET_MAKER,

        _NUM_COLUMNS,
    };

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] symbol  underlying symbol
     * @param[in] stamp  date time
     * @param[in] parent  parent object
     */
    QuoteTableModel( const QString& symbol, const QDateTime& stamp = QDateTime(), QObject *parent = nullptr );

    /// Destructor.
    virtual ~QuoteTableModel();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve column description.
    /**
     * @param[in] col  column
     * @return  description
     */
    virtual QString columnDescription( int col ) const override;

    /// Retrieve mark.
    /**
     * @return  amount
     */
    virtual double mark() const;

    /// Retrieve symbol.
    /**
     * @return  symbol
     */
    virtual QString symbol() const {return symbol_;}

    /// Retrieve table data.
    /**
     * @param[in] col  column
     * @param[in] role  role
     * @return  data
     */
    virtual QVariant tableData( ColumnIndex col, int role = Qt::DisplayRole ) const {return _Mybase::data0( col, role );}

protected:

    QString symbol_;                                ///< Underlying symbol.

private:

    // not implemented
    QuoteTableModel( const _Myt& ) = delete;

    // not implemented
    QuoteTableModel( const _Myt&& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // QUOTETABLEMODEL_H
