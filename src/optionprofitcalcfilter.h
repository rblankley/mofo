/**
 * @file optionprofitcalcfilter.h
 * Filter for stock option profit calculators.
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

#ifndef OPTIONPROFITCALCFILTER_H
#define OPTIONPROFITCALCFILTER_H

#include <QByteArray>

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Filter for stock option profit calculators.
class OptionProfitCalculatorFilter
{
    using _Myt = OptionProfitCalculatorFilter;

public:

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

    /// Option strategies.
    enum OptionTradingStrategyFilter
    {
        SINGLE = 0x0001,                            ///< Single options (CSP and CC).
        VERTICAL = 0x0002,                          ///< Verticals.
        CALENDAR = 0x0004,                          ///< Calendar trades.
        STRANGLE = 0x0008,                          ///< Strangles.
        STRADDLE = 0x0010,                          ///< Straddles.
        BUTTERFLY = 0x0020,                         ///< Butterflies.
        CONDOR = 0x0040,                            ///< Iron Condor.
        DIAGONAL = 0x0080,                          ///< Diagonals.
        COLLAR = 0x0100,                            ///< Collar trades.

        ALL_STRATEGIES = 0xffff,                    ///< All trading strategies.
    };

    /// Volatility.
    enum VolatilityFilter
    {
        HV_LTE_VI = 0x1,                            ///< Historical volatility less than or equal to implied volatility.
        HV_GT_VI = 0x2,                             ///< Historical volatility greater than implied volatility.

        ALL_VOLATILITY = HV_LTE_VI | HV_GT_VI,      ///< All volatilities.
    };

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    OptionProfitCalculatorFilter();

    /// Constructor.
    /**
     * @param[in] state  saved state
     */
    OptionProfitCalculatorFilter( const QByteArray& state );

    /// Destructor.
    virtual ~OptionProfitCalculatorFilter();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Set minimum investment amount.
    /**
     * @param[in] value  amount
     */
    virtual void setMinInvestAmount( double value ) {minInvestAmount_ = value;}

    /// Retrieve minimum investment amount.
    /**
     * @return  amount
     */
    virtual double minInvestAmount() const {return minInvestAmount_;}

    /// Set maximum investment amount.
    /**
     * @param[in] value  amount
     */
    virtual void setMaxInvestAmount( double value ) {maxInvestAmount_ = value;}

    /// Retrieve maximum investment amount.
    /**
     * @return  amount
     */
    virtual double maxInvestAmount() const {return maxInvestAmount_;}

    /// Set maximum loss amount.
    /**
     * @param[in] value  amount
     */
    virtual void setMaxLossAmount( double value ) {maxLossAmount_ = value;}

    /// Retrieve maximum loss amount.
    /**
     * @return  amount
     */
    virtual double maxLossAmount() const {return maxLossAmount_;}

    /// Set minimum gain amount.
    /**
     * @param[in] value  amount
     */
    virtual void setMinGainAmount( double value ) {minGainAmount_ = value;}

    /// Retrieve minimum gain amount.
    /**
     * @return  amount
     */
    virtual double minGainAmount() const {return minGainAmount_;}

    /// Set minimum bid size.
    /**
     * @param[in] value  size
     */
    virtual void setMinBidSize( int value ) {minBidSize_ = value;}

    /// Retrieve minimum bid size.
    /**
     * @return  size
     */
    virtual int minBidSize() const {return minBidSize_;}

    /// Set minimum ask size.
    /**
     * @param[in] value  size
     */
    virtual void setMinAskSize( int value ) {minAskSize_ = value;}

    /// Retrieve minimum ask size.
    /**
     * @return  size
     */
    virtual int minAskSize() const {return minAskSize_;}

    /// Set minimum probability of profit.
    /**
     * @param[in] value  percentage
     */
    virtual void setMinProbProfit( double value ) {minProbProfit_ = value;}

    /// Retrieve minimum probability of profit.
    /**
     * @return  percentage
     */
    virtual double minProbProfit() const {return minProbProfit_;}

    /// Set maximum probability of profit.
    /**
     * @param[in] value  percentage
     */
    virtual void setMaxProbProfit( double value ) {maxProbProfit_ = value;}

    /// Retrieve maximum probability of profit.
    /**
     * @return  percentage
     */
    virtual double maxProbProfit() const {return maxProbProfit_;}

    /// Set minimum days to expiration.
    /**
     * @param[in] value  DTE
     */
    virtual void setMinDaysToExpiry( int value ) {minDaysToExpiry_ = value;}

    /// Retrieve minimum days to expiration.
    /**
     * @return  DTE
     */
    virtual int minDaysToExpiry() const {return minDaysToExpiry_;}

    /// Set maximum days to expiration.
    /**
     * @param[in] value  DTE
     */
    virtual void setMaxDaysToExpiry( int value ) {maxDaysToExpiry_ = value;}

    /// Retrieve maximum days to expiration.
    /**
     * @return  DTE
     */
    virtual int maxDaysToExpiry() const {return maxDaysToExpiry_;}

    /// Set minimum ROI.
    /**
     * @param[in] value  percent
     */
    virtual void setMinReturnOnInvestment( double value ) {minReturnOnInvestment_ = value;}

    /// Retrieve minimum ROI.
    /**
     * @return  percent
     */
    virtual double minReturnOnInvestment() const {return minReturnOnInvestment_;}

    /// Set maximum ROI.
    /**
     * @param[in] value  percent
     */
    virtual void setMaxReturnOnInvestment( double value ) {maxReturnOnInvestment_ = value;}

    /// Retrieve maximum ROI.
    /**
     * @return  percent
     */
    virtual double maxReturnOnInvestment() const {return maxReturnOnInvestment_;}

    /// Set minimum ROI over time.
    /**
     * @param[in] value  percent
     */
    virtual void setMinReturnOnInvestmentTime( double value ) {minReturnOnInvestmentTime_ = value;}

    /// Retrieve minimum ROI over time.
    /**
     * @return  percent
     */
    virtual double minReturnOnInvestmentTime() const {return minReturnOnInvestmentTime_;}

    /// Set maximum ROI over time.
    /**
     * @param[in] value  percent
     */
    virtual void setMaxReturnOnInvestmentTime( double value ) {maxReturnOnInvestmentTime_ = value;}

    /// Retrieve maximum ROI over time.
    /**
     * @return  percent
     */
    virtual double maxReturnOnInvestmentTime() const {return maxReturnOnInvestmentTime_;}

    /// Set maximum bid/ask spread percent.
    /**
     * @param[in] value  percent
     */
    virtual void setMaxSpreadPercent( double value ) {maxSpreadPercent_ = value;}

    /// Retrieve maximum bid/ask spread percent.
    /**
     * @return  percent
     */
    virtual double maxSpreadPercent() const {return maxSpreadPercent_;}

    /// Set minimum volatility.
    /**
     * @param[in] value  volatility
     */
    virtual void setMinVolatility( double value ) {minVolatility_ = value;}

    /// Retrieve minimum volatility.
    /**
     * @return  volatility
     */
    virtual double minVolatility() const {return minVolatility_;}

    /// Set maximum volatility.
    /**
     * @param[in] value  volatility
     */
    virtual void setMaxVolatility( double value ) {maxVolatility_ = value;}

    /// Retrieve maximum volatility.
    /**
     * @return  volatility
     */
    virtual double maxVolatility() const {return maxVolatility_;}

    /// Set option type filtering.
    /**
     * @param[in] value  option type(s)
     */
    virtual void setOptionTypeFilter( OptionTypeFilter value ) {optionTypes_ = value;}

    /// Retrieve option type filtering.
    /**
     * @return  option type(s)
     */
    virtual OptionTypeFilter optionTypeFilter() const {return optionTypes_;}

    /// Set option trading strategy filtering.
    /**
     * @param[in] value  option trading strategies
     */
    virtual void setOptionTradingStrategyFilter( OptionTradingStrategyFilter value ) {optionTradingStrats_ = value;}

    /// Retrieve option trading strategy filtering.
    /**
     * @return  option trading strategies
     */
    virtual OptionTradingStrategyFilter optionTradingStrategyFilter() const {return optionTradingStrats_;}

    /// Set option volatility filtering.
    /**
     * @param[in] value  volatility filter
     */
    virtual void setVolatilityFilter( VolatilityFilter value ) {volatility_ = value;}

    /// Retrieve option volatility filtering.
    /**
     * @return  volatility filter
     */
    virtual VolatilityFilter volatilityFilter() const {return volatility_;}

    /// Set vertical depth (for vertical analysis).
    /**
     * @param[in] value  depth
     */
    virtual void setVerticalDepth( int value ) {vertDepth_ = value;}

    /// Retrieve vertical depth (for vertical analysis)
    /**
     * @return  depth
     */
    virtual int verticalDepth() const {return vertDepth_;}

    // ========================================================================
    // Methods
    // ========================================================================

    /// Save filter state.
    /**
     * @return  filter state
     */
    virtual QByteArray saveState() const;

    /// Restore filter state.
    /**
     * @param[in] state  filter state
     */
    virtual void restoreState( const QByteArray& state );

protected:

    double minInvestAmount_;                                ///< Minimum investment amount.
    double maxInvestAmount_;                                ///< Maximum investment amount.

    double maxLossAmount_;                                  ///< Maximum loss amount.
    double minGainAmount_;                                  ///< Minimum gain amount.

    int minBidSize_;                                        ///< Minimum bid size.
    int minAskSize_;                                        ///< Minimum ask size.

    double minProbProfit_;                                  ///< Minimum probability of profit.
    double maxProbProfit_;                                  ///< Maximum probability of profit.

    int minDaysToExpiry_;                                   ///< Minimum days to expiration.
    int maxDaysToExpiry_;                                   ///< Maximum days to expiration.

    double minReturnOnInvestment_;                          ///< Minimum return on investment.
    double maxReturnOnInvestment_;                          ///< Maximum return on investment.

    double minReturnOnInvestmentTime_;                      ///< Minimum return on investment over time.
    double maxReturnOnInvestmentTime_;                      ///< Maximum return on investment over time.

    double maxSpreadPercent_;                               ///< Maximum spread bid/ask amount.

    double minVolatility_;                                  ///< Minimum volatility.
    double maxVolatility_;                                  ///< Maximum volatility.

    OptionTypeFilter optionTypes_;                          ///< Option type filter.
    OptionTradingStrategyFilter optionTradingStrats_;       ///< Option trading strategies.

    VolatilityFilter volatility_;                           ///< Volatility filter.

    int vertDepth_;                                         ///< Depth for vertical analysis.

private:

    enum {DEFAULT_VERT_DEPTH = 3};

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // OPTIONPROFITCALCFILTER_H
