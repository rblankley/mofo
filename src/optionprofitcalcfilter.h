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

#include <db/optiontradingitemmodel.h>

#include <QByteArray>

class FundamentalsTableModel;
class OptionChainTableModel;
class QuoteTableModel;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Filter for stock option profit calculators.
class OptionProfitCalculatorFilter
{
    using _Myt = OptionProfitCalculatorFilter;

public:

    /// Option types.
    enum OptionTypeFilter
    {
        ITM_CALLS = 0x1,                                    ///< In the money calls.
        OTM_CALLS = 0x2,                                    ///< Out of the money calls.
        ITM_PUTS = 0x4,                                     ///< In the money puts.
        OTM_PUTS = 0x8,                                     ///< Out of the money puts.

        ONLY_CALLS = ITM_CALLS | OTM_CALLS,                 ///< Only call options.
        ONLY_PUTS = ITM_PUTS | OTM_PUTS,                    ///< Only put options.
        ALL_OPTION_TYPES = ONLY_CALLS | ONLY_PUTS,          ///< All options.
    };

    /// Option strategies.
    enum OptionTradingStrategyFilter
    {
        SINGLE = 0x0001,                                    ///< Single options (CSP and CC).
        VERTICAL = 0x0002,                                  ///< Verticals.
        CALENDAR = 0x0004,                                  ///< Calendar trades.
        STRANGLE = 0x0008,                                  ///< Strangles.
        STRADDLE = 0x0010,                                  ///< Straddles.
        BUTTERFLY = 0x0020,                                 ///< Butterflies.
        CONDOR = 0x0040,                                    ///< Iron Condor.
        DIAGONAL = 0x0080,                                  ///< Diagonals.
        COLLAR = 0x0100,                                    ///< Collar trades.

        ALL_STRATEGIES = 0xffff,                            ///< All trading strategies.
    };

    /// Price.
    enum PriceFilter
    {
        THEO_LTE_MARKET = 0x1,                              ///< Theoretical option price less than or equal to market price.
        THEO_GT_MARKET = 0x2,                               ///< Theoretical option price greater than market price.

        ALL_PRICES = THEO_LTE_MARKET | THEO_GT_MARKET,      ///< All prices.
    };

    /// Volatility.
    enum VolatilityFilter
    {
        HV_LTE_VI = 0x1,                                    ///< Historical volatility less than or equal to implied volatility.
        HV_GT_VI = 0x2,                                     ///< Historical volatility greater than implied volatility.

        ALL_VOLATILITY = HV_LTE_VI | HV_GT_VI,              ///< All volatilities.
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

    /// Set minimum underlying price.
    /**
     * @param[in] value  amount
     */
    virtual void setMinUnderlyingPrice( double value ) {minUnderlyingPrice_ = value;}

    /// Retrieve minimum underlying price.
    /**
     * @return  amount
     */
    virtual double minUnderlyingPrice() const {return minUnderlyingPrice_;}

    /// Set maximum underlying price.
    /**
     * @param[in] value  amount
     */
    virtual void setMaxUnderlyingPrice( double value ) {maxUnderlyingPrice_ = value;}

    /// Retrieve maximum underlying price.
    /**
     * @return  amount
     */
    virtual double maxUnderlyingPrice() const {return maxUnderlyingPrice_;}

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

    /// Set minimum probability of ITM.
    /**
     * @param[in] value  percentage
     */
    virtual void setMinProbITM( double value ) {minProbITM_ = value;}

    /// Retrieve minimum probability of ITM.
    /**
     * @return  percentage
     */
    virtual double minProbITM() const {return minProbITM_;}

    /// Set maximum probability of ITM.
    /**
     * @param[in] value  percentage
     */
    virtual void setMaxProbITM( double value ) {maxProbITM_ = value;}

    /// Retrieve maximum probability of ITM.
    /**
     * @return  percentage
     */
    virtual double maxProbITM() const {return maxProbITM_;}

    /// Set minimum probability of OTM.
    /**
     * @param[in] value  percentage
     */
    virtual void setMinProbOTM( double value ) {minProbOTM_ = value;}

    /// Retrieve minimum probability of OTM.
    /**
     * @return  percentage
     */
    virtual double minProbOTM() const {return minProbOTM_;}

    /// Set maximum probability of OTM.
    /**
     * @param[in] value  percentage
     */
    virtual void setMaxProbOTM( double value ) {maxProbOTM_ = value;}

    /// Retrieve maximum probability of OTM.
    /**
     * @return  percentage
     */
    virtual double maxProbOTM() const {return maxProbOTM_;}

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

    /// Set minimum dividend amount.
    /**
     * @param[in] value  amount
     */
    virtual void setMinDividendAmount( int value ) {minDividendAmount_ = value;}

    /// Retrieve minimum dividend amount.
    /**
     * @return  amount
     */
    virtual int minDividendAmount() const {return minDividendAmount_;}

    /// Set maximum dividend amount.
    /**
     * @param[in] value  amount
     */
    virtual void setMaxDividendAmount( int value ) {maxDividendAmount_ = value;}

    /// Retrieve maximum dividend amount.
    /**
     * @return  amount
     */
    virtual int maxDividendAmount() const {return maxDividendAmount_;}

    /// Set minimum dividend yield.
    /**
     * @param[in] value  percent
     */
    virtual void setMinDividendYield( int value ) {minDividendYield_ = value;}

    /// Retrieve minimum dividend yield.
    /**
     * @return  percent
     */
    virtual int minDividendYield() const {return minDividendYield_;}

    /// Set maximum dividend yield.
    /**
     * @param[in] value  percent
     */
    virtual void setMaxDividendYield( int value ) {maxDividendYield_ = value;}

    /// Retrieve maximum dividend yield.
    /**
     * @return  percent
     */
    virtual int maxDividendYield() const {return maxDividendYield_;}

    /// Set minimum ROR.
    /**
     * @param[in] value  percent
     */
    virtual void setMinReturnOnRisk( double value ) {minReturnOnRisk_ = value;}

    /// Retrieve minimum ROR.
    /**
     * @return  percent
     */
    virtual double minReturnOnRisk() const {return minReturnOnRisk_;}

    /// Set maximum ROR.
    /**
     * @param[in] value  percent
     */
    virtual void setMaxReturnOnRisk( double value ) {maxReturnOnRisk_ = value;}

    /// Retrieve maximum ROR.
    /**
     * @return  percent
     */
    virtual double maxReturnOnRisk() const {return maxReturnOnRisk_;}

    /// Set minimum ROR over time.
    /**
     * @param[in] value  percent
     */
    virtual void setMinReturnOnRiskTime( double value ) {minReturnOnRiskTime_ = value;}

    /// Retrieve minimum ROR over time.
    /**
     * @return  percent
     */
    virtual double minReturnOnRiskTime() const {return minReturnOnRiskTime_;}

    /// Set maximum ROR over time.
    /**
     * @param[in] value  percent
     */
    virtual void setMaxReturnOnRiskTime( double value ) {maxReturnOnRiskTime_ = value;}

    /// Retrieve maximum ROR over time.
    /**
     * @return  percent
     */
    virtual double maxReturnOnRiskTime() const {return maxReturnOnRiskTime_;}

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

    /// Set minimum EV.
    /**
     * @param[in] value  percent
     */
    virtual void setMinExpectedValue( double value ) {minExpectedValue_ = value;}

    /// Retrieve minimum EV.
    /**
     * @return  percent
     */
    virtual double minExpectedValue() const {return minExpectedValue_;}

    /// Set maximum EV.
    /**
     * @param[in] value  percent
     */
    virtual void setMaxExpectedValue( double value ) {maxExpectedValue_ = value;}

    /// Retrieve maximum EV.
    /**
     * @return  percent
     */
    virtual double maxExpectedValue() const {return maxExpectedValue_;}

    /// Set minimum EV ROI.
    /**
     * @param[in] value  percent
     */
    virtual void setMinExpectedValueReturnOnInvestment( double value ) {minExpectedValueReturnOnInvestment_ = value;}

    /// Retrieve minimum EV ROI.
    /**
     * @return  percent
     */
    virtual double minExpectedValueReturnOnInvestment() const {return minExpectedValueReturnOnInvestment_;}

    /// Set maximum EV ROI.
    /**
     * @param[in] value  percent
     */
    virtual void setMaxExpectedValueReturnOnInvestment( double value ) {maxExpectedValueReturnOnInvestment_ = value;}

    /// Retrieve maximum EV ROI.
    /**
     * @return  percent
     */
    virtual double maxExpectedValueReturnOnInvestment() const {return maxExpectedValueReturnOnInvestment_;}

    /// Set minimum EV ROI over time.
    /**
     * @param[in] value  percent
     */
    virtual void setMinExpectedValueReturnOnInvestmentTime( double value ) {minExpectedValueReturnOnInvestmentTime_ = value;}

    /// Retrieve minimum EV ROI over time.
    /**
     * @return  percent
     */
    virtual double minExpectedValueReturnOnInvestmentTime() const {return minExpectedValueReturnOnInvestmentTime_;}

    /// Set maximum EV ROI over time.
    /**
     * @param[in] value  percent
     */
    virtual void setMaxExpectedValueReturnOnInvestmentTime( double value ) {maxExpectedValueReturnOnInvestmentTime_ = value;}

    /// Retrieve maximum EV ROI over time.
    /**
     * @return  percent
     */
    virtual double maxExpectedValueReturnOnInvestmentTime() const {return maxExpectedValueReturnOnInvestmentTime_;}

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

    /// Set option price filtering.
    /**
     * @param[in] value  price filter
     */
    virtual void setPriceFilter( PriceFilter value ) {price_ = value;}

    /// Retrieve option price filtering.
    /**
     * @return  price filter
     */
    virtual PriceFilter priceFilter() const {return price_;}

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

    /// Set advanced filters.
    /**
     * @param[in] value  advanced filter list
     */
    virtual void setAdvancedFilters( const QStringList& value ) {advancedFilters_ = value;}

    /// Retrieve advanced filters.
    /**
     * @return  advanced filter list
     */
    virtual QStringList advancedFilters() const {return advancedFilters_;}

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

    /// Check data against filter.
    /**
     * @param[in] quote  quote
     * @param[in] fundamentals  fundamentals data
     * @return  @c true if passes filter, @c false otherwise
     */
    virtual bool check( const QuoteTableModel *quote, const FundamentalsTableModel *fundamentals ) const;

    /// Check data against filter.
    /**
     * @param[in] chain  chain information
     * @param[in] row  row to check
     * @param[in] isCall  @c true to check call, @c false to check put
     * @return  @c true if passes filter, @c false otherwise
     */
    virtual bool check( const OptionChainTableModel *chain, int row, bool isCall ) const;

    /// Check data against filter.
    /**
     * @param[in] trade  trade information
     * @return  @c true if passes filter, @c false otherwise
     */
    virtual bool check( const OptionTradingItemModel::ColumnValueMap& trade ) const;

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

    static constexpr double MIN_SPREAD_AMOUNT = 0.05;           ///< Minimum spread amount required to apply filter.

    double minInvestAmount_;                                    ///< Minimum investment amount.
    double maxInvestAmount_;                                    ///< Maximum investment amount.

    double minUnderlyingPrice_;                                 ///< Minimum underlying (equity spot) price.
    double maxUnderlyingPrice_;                                 ///< Maximum underlying (equity spot) price.

    double maxLossAmount_;                                      ///< Maximum loss amount.
    double minGainAmount_;                                      ///< Minimum gain amount.

    int minBidSize_;                                            ///< Minimum bid size.
    int minAskSize_;                                            ///< Minimum ask size.

    double minProbITM_;                                         ///< Minimum probability for ending in the money.
    double maxProbITM_;                                         ///< Maximum probability for ending in the money.

    double minProbOTM_;                                         ///< Minimum probability for ending out of money.
    double maxProbOTM_;                                         ///< Maximum probability for ending out of money.

    double minProbProfit_;                                      ///< Minimum probability of profit.
    double maxProbProfit_;                                      ///< Maximum probability of profit.

    int minDaysToExpiry_;                                       ///< Minimum days to expiration.
    int maxDaysToExpiry_;                                       ///< Maximum days to expiration.

    double minDividendAmount_;                                  ///< Minimum expected dividend amount.
    double maxDividendAmount_;                                  ///< Maximum expected dividend amount.

    double minDividendYield_;                                   ///< Minimum expected dividend yield.
    double maxDividendYield_;                                   ///< Maximum expected dividend yield.

    double minReturnOnRisk_;                                    ///< Minimum return on risk.
    double maxReturnOnRisk_;                                    ///< Maximum return on risk.

    double minReturnOnRiskTime_;                                ///< Minimum return on risk over time.
    double maxReturnOnRiskTime_;                                ///< Maximum return on risk over time.

    double minReturnOnInvestment_;                              ///< Minimum return on investment.
    double maxReturnOnInvestment_;                              ///< Maximum return on investment.

    double minReturnOnInvestmentTime_;                          ///< Minimum return on investment over time.
    double maxReturnOnInvestmentTime_;                          ///< Maximum return on investment over time.

    double minExpectedValue_;                                   ///< Minimum expected value amount.
    double maxExpectedValue_;                                   ///< Maximum expected value amount.

    double minExpectedValueReturnOnInvestment_;                 ///< Minimum expected value as return on investment.
    double maxExpectedValueReturnOnInvestment_;                 ///< Maximum expected value as return on investment.

    double minExpectedValueReturnOnInvestmentTime_;             ///< Minimum expected value as return on investment over time.
    double maxExpectedValueReturnOnInvestmentTime_;             ///< Maximum expected value as return on investment over time.

    double maxSpreadPercent_;                                   ///< Maximum spread bid/ask amount.

    double minVolatility_;                                      ///< Minimum volatility.
    double maxVolatility_;                                      ///< Maximum volatility.

    QStringList advancedFilters_;                               ///< Advanced filters.

    OptionTypeFilter optionTypes_;                              ///< Option type filter.
    OptionTradingStrategyFilter optionTradingStrats_;           ///< Option trading strategies.

    PriceFilter price_;                                         ///< Price filter.

    VolatilityFilter volatility_;                               ///< Volatility filter.

    int vertDepth_;                                             ///< Depth for vertical analysis.

private:

    static constexpr int DEFAULT_VERT_DEPTH = 3;

    mutable const QuoteTableModel *q_;
    mutable const FundamentalsTableModel *f_;

    mutable const OptionChainTableModel *oc_;
    mutable int ocr_;

    mutable const OptionTradingItemModel::ColumnValueMap *t_;

    /// Check advanced filters.
    bool checkAdvancedFilters() const;

    /// Retrieve table data value.
    QVariant tableData( const QString& t, int col ) const;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // OPTIONPROFITCALCFILTER_H
