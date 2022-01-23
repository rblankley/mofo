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

#include <QColor>

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Item model for stock option trading.
class OptionTradingItemModel : public ItemModel
{
    Q_OBJECT

    using _Myt = OptionTradingItemModel;
    using _Mybase = ItemModel;

public:

    /// Map of column values.
    using ColumnValueMap = QMap<int, QVariant>;

    /// Column index values.
    enum ColumnIndex
    {
        STAMP,                                      ///< Stamp.
        UNDERLYING,                                 ///< Underlying symbol.
        UNDERLYING_PRICE,                           ///< Underlying share price (mark).
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

        DIV_AMOUNT,
        DIV_YIELD,

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
        PROBABILITY_PROFIT,

        INVESTMENT_OPTION_PRICE,                    ///< Calculated buy/sell price based on adjusted bid/ask.
        INVESTMENT_OPTION_PRICE_VS_THEO,

        INVESTMENT_AMOUNT,
        PREMIUM_AMOUNT,
        MAX_GAIN,
        MAX_LOSS,

        ROR,                                        ///< Return on risk.
        ROR_TIME,

        ROI,                                        ///< Return on investment.
        ROI_TIME,

        EXPECTED_VALUE,                             ///< Expected value.
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

    /// Retrieve column description.
    /**
     * @param[in] col  column
     * @return  description
     */
    virtual QString columnDescription( int col ) const override;

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
    static double calcError( const QVariant& col0, const QVariant& col1, bool &valid );

    /// Calculate error color.
    static QColor calcErrorColor( const QVariant& col0, const QVariant& col1, const QColor& orig );

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
