/**
 * @file optionprofitcalc.h
 * Stock option profit calculator.
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

#ifndef OPTIONPROFITCALC_H
#define OPTIONPROFITCALC_H

#include "db/optiontradingitemmodel.h"

class OptionChainTableModel;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Stock option profit calculator.
class OptionProfitCalculator
{
    using _Myt = OptionProfitCalculator;

public:

    /// Table model type.
    using table_model_type = OptionChainTableModel;

    /// Item model type.
    using item_model_type = OptionTradingItemModel;

    /// Option types.
    enum OptionTypeFilter
    {
        ITM_CALLS = 0x1,                            ///< In the money calls.
        OTM_CALLS = 0x2,                            ///< Out of the money calls.
        ITM_PUTS = 0x4,                             ///< In the money puts.
        OTM_PUTS = 0x8,                             ///< Out of the money puts.

        ONLY_CALLS = ITM_CALLS | OTM_CALLS,         ///< Only call options.
        ONLY_PUTS = ITM_PUTS | OTM_PUTS,            ///< Only put options.
        ALL_OPTION_TYPES = ONLY_CALLS | ONLY_PUTS,  ///< All options.
    };

    /// Volatility.
    enum VolatilityFilter
    {
        HV_LT_VI = 0x1,                             ///< Historical volatility less than or equal to implied volatility.
        HV_GT_VI = 0x2,                             ///< Historical volatility greater than implied volatility.

        ALL_VOLATILITY = HV_LT_VI | HV_GT_VI,       ///< All volatilities.
    };

    // ========================================================================
    // DTOR
    // ========================================================================

    /// Destructor.
    virtual ~OptionProfitCalculator();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Set maximum investment amount.
    /**
     * @param[in] value  amount
     */
    virtual void setMaxInvestAmount( double value ) {maxInvestAmount_ = value;}

    /// Set minimum investment amount.
    /**
     * @param[in] value  amount
     */
    virtual void setMinInvestAmount( double value ) {minInvestAmount_ = value;}

    /// Set minimum return on investment.
    /**
     * @param[in] value  minimum return percentage
     */
    virtual void setMinReturnOnInvestment( double value ) {minReturnOnInvestment_ = value;}

    /// Set maximum volatility.
    /**
     * @param[in] value  amount
     */
    virtual void setMaxVolatility( double value ) {maxVolatility_ = value;}

    /// Set minimum volatility.
    /**
     * @param[in] value  amount
     */
    virtual void setMinVolatility( double value ) {minVolatility_ = value;}

    /// Set filter for option types.
    /**
     * @param[in] value  option types
     */
    virtual void setOptionTypeFilter( OptionTypeFilter value ) {optionTypes_ = value;}

    /// Set filter for volatility.
    /**
     * @param[in] value  volatility
     */
    virtual void setVolatilityFilter( VolatilityFilter value ) {volatility_ = value;}

    /// Set option trading cost.
    /**
     * @param[in] value  trade cost
     */
    virtual void setOptionTradeCost( double value ) {optionTradeCost_ = value;}

    /// Set depth for verical option analysis.
    /**
     * @param[in] value  depth
     */
    virtual void setVerticalAnalysisDepth( int value ) {vertDepth_ = value;}

    // ========================================================================
    // Methods
    // ========================================================================

    /// Analyze option chain.
    /**
     * @param[out] strat  trading strategy
     */
    virtual void analyze( item_model_type::Strategy strat ) = 0;

protected:

    bool valid_;                                    ///< Chain is valid.

    int daysToExpiry_;                              ///< Days to expiration.

    double underlying_;                             ///< Underlying price.

    const table_model_type *chains_;                ///< Chains for analysis.

    item_model_type *results_;                      ///< Results.

    // ---- //

    double minInvestAmount_;                        ///< Minimum investment amount.
    double maxInvestAmount_;                        ///< Maximum investment amount.

    double maxLossAmount_;                          ///< Maximum loss amount.

    double minReturnOnInvestment_;                  ///< Minimum return on investment.
    double minSpreadPercent_;                       ///< Minimum spread bid/ask amount.

    double minVolatility_;                          ///< Minimum volatility.
    double maxVolatility_;                          ///< Maximum volatility.

    OptionTypeFilter optionTypes_;                  ///< Option type filter.
    VolatilityFilter volatility_;                   ///< Volatility filter.

    double optionTradeCost_;                        ///< Cost to trade an option.

    int vertDepth_;                                 ///< Depth for vertical analysis.

    double spreadFilterCutOff_;                     ///< Max bid and min ask.

    // ========================================================================
    // CTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] underlying  underlying price (i.e. mark)
     * @param[in] chains  chains to evaluate
     * @param[in] results  results
     */
    OptionProfitCalculator( double underlying, const table_model_type *chains, item_model_type *results );

    // ========================================================================
    // Methods
    // ========================================================================

    /// Populate result model with put/call information.
    /**
     * @param[in] row  row
     * @param[in] strike  strike price
     * @param[in] isCall  @c true if call option type, @c false otherwise
     * @param[in] result  result
     */
    virtual void populateResultModel( int row, double strike, bool isCall, item_model_type::ColumnValueMap& result ) const;

private:

    enum {DEFAULT_VERT_DEPTH = 5};

    /// Days to expiration.
    double calcDaysToExpiry() const;

    // not implemented
    OptionProfitCalculator( const _Myt& ) = delete;

    // not implemented
    OptionProfitCalculator( const _Myt&& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // OPTIONPROFITCALC_H
