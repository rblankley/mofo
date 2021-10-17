/**
 * @file optiontradingitemmodel.h
 * Item model for stock option trading.
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

#ifndef OPTIONTRADINGITEMMODEL_H
#define OPTIONTRADINGITEMMODEL_H

#include "itemmodel.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Item model for stock option trading.
class OptionTradingItemModel : public ItemModel
{
    Q_OBJECT

    using _Myt = OptionTradingItemModel;
    using _Mybase = ItemModel;

public:

    /// Column index values.
    enum ColumnIndex
    {
        STAMP,                                      ///< Stamp.
        UNDERLYING,                                 ///< Underlying symbol.
        TYPE,                                       ///< Option type CALL or PUT.

        STRATEGY,                                   ///< Trading strategy.
        STRATEGY_DESC,                              ///< Trading strategy description.

        // Option Chain Information
        SYMBOL,
        DESC,
        BID_ASK_SIZE,
        BID_PRICE,
        BID_SIZE,
        ASK_PRICE,
        ASK_SIZE,
        LAST_PRICE,
        LAST_SIZE,
        BREAK_EVEN_PRICE,
        INTRINSIC_VALUE,
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
        EXCHANGE_NAME,
        VOLATILITY,
        DELTA,
        GAMMA,
        THETA,
        VEGA,
        RHO,
        TIME_VALUE,
        OPEN_INTEREST,
        IS_IN_THE_MONEY,
        IS_OUT_OF_THE_MONEY,
        THEO_OPTION_VALUE,
        THEO_VOLATILITY,
        IS_MINI,
        IS_NON_STANDARD,
        IS_INDEX,
        IS_WEEKLY,
        IS_QUARTERLY,
        EXPIRY_DATE,
        EXPIRY_TYPE,
        DAYS_TO_EXPIRY,
        LAST_TRADING_DAY,
        MULTIPLIER,
        SETTLEMENT_TYPE,
        DELIVERABLE_NOTE,
        STRIKE_PRICE,

        // Calculated Fields
        HIST_VOLATILITY,

        TIME_TO_EXPIRY,
        RISK_FREE_INTEREST_RATE,

        CALC_BID_PRICE_VI,
        CALC_ASK_PRICE_VI,
        CALC_MARK_VI,

        CALC_THEO_OPTION_VALUE,                     ///< Theoretical Option Value.
        CALC_THEO_VOLATILITY,
        CALC_DELTA,
        CALC_GAMMA,
        CALC_THETA,
        CALC_VEGA,
        CALC_RHO,

        BID_ASK_SPREAD,
        BID_ASK_SPREAD_PERCENT,

        PROBABILITY_ITM,
        PROBABILITY_OTM,

        INVESTMENT_OPTION_PRICE,                    ///< Calculated buy/sell price based on adjusted bid/ask.
        INVESTMENT_OPTION_PRICE_VS_THEO,

        INVESTMENT_VALUE,
        MAX_GAIN,
        MAX_LOSS,
        ROI,
        ROI_TIME,

        EXPECTED_VALUE,
        EXPECTED_VALUE_ROI,
        EXPECTED_VALUE_ROI_TIME,

        _NUM_COLUMNS,
    };

    /// Option trading strategies.
    enum Strategy
    {
        SINGLE,                                     ///< Single.
        VERT_BULL_PUT,                              ///< Vertical bull put.
        VERT_BEAR_CALL,                             ///< Vertical bear call.
    };

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] parent  parent object
     */
    OptionTradingItemModel( QObject *parent = nullptr );

    /// Destructor.
    virtual ~OptionTradingItemModel();

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

    /// Retrieve flags.
    /**
     * @param[in] index  index
     * @return  flags
     */
    virtual Qt::ItemFlags flags( const QModelIndex& index ) const override;

public slots:

    // ========================================================================
    // Methods
    // ========================================================================

    /// Add row to model.
    /**
     * @param[in] values  values
     */
    virtual void addRow( const ColumnValueMap& values );

private:

    QColor inTheMoneyColor_;
    QColor mixedMoneyColor_;

    QColor textColor_;

    /// Calculate percent error.
    double calcError( int row, ColumnIndex col0, ColumnIndex col1, bool &valid ) const;

    /// Calculate error color.
    QColor calcErrorColor( int row, ColumnIndex col0, ColumnIndex col1, const QColor& orig ) const;

    /// Format value.
    static QString formatValue( const QVariant& value, bool isCurrency = false );

    /// Retrieve strategy text.
    static QString strategyText( Strategy strat );

    // not implemented
    OptionTradingItemModel( const _Myt& ) = delete;

    // not implemented
    OptionTradingItemModel( const _Myt&& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // OPTIONTRADINGITEMMODEL_H
