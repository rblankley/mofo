/**
 * @file expectedvaluecalc.h
 * Profit calculator based on expected value.
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

#ifndef EXPECTEDVALUECALC_H
#define EXPECTEDVALUECALC_H

#include "../optionprofitcalc.h"

class AbstractOptionPricing;

enum class OptionType;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Profit calculator based on expected value.
class ExpectedValueCalculator : public OptionProfitCalculator
{
    using _Myt = ExpectedValueCalculator;
    using _Mybase = OptionProfitCalculator;

public:

    // ========================================================================
    // DTOR
    // ========================================================================

    /// Destructor.
    ~ExpectedValueCalculator();

    // ========================================================================
    // Methods
    // ========================================================================

    /// Analyze option chain.
    /**
     * @param[out] strat  trading strategy
     */
    virtual void analyze( item_model_type::Strategy strat ) override;

protected:

    // ========================================================================
    // CTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] underlying  underlying price (i.e. mark)
     * @param[in] chains  chains to evaluate
     * @param[in] results  results
     */
    ExpectedValueCalculator( double underlying, const table_model_type *chains, item_model_type *results );

    // ========================================================================
    // Methods
    // ========================================================================

    /// Analyze single.
    /**
     * @param[in] call  process calls
     * @param[in] put  process puts
     */
    virtual void analyzeSingle( bool call, bool put ) const;

    /// Analyze vertical bear calls.
    virtual void analyzeVertBearCalls() const;

    /// Analyze vertical bull put.
    virtual void analyzeVertBullPuts() const;

    /// Generate greeks.
    /**
     * @return  @c true if greeks valid, @c false otherwise
     */
    virtual bool generateGreeks();

    /// Generate probability curve.
    /**
     * @return  @c true if curve is valid, @c false otherwise
     */
    virtual bool generateProbCurve();

    /// Calculate implied volatility.
    /**
     * @tparam T  option pricing class
     * @param[in,out] pricing  option pricing
     * @param[in] type  option type
     * @param[in] X  strike price
     * @param[in] price  option price
     * @param[out] okay  @c true if calculation okay, @c false otherwise
     * @return  implied volatility of @p pricing
     */
    virtual double calcImplVol( AbstractOptionPricing *pricing, OptionType type, double X, double price, bool *okay = nullptr ) const = 0;

    /// Factory method for creation of Option Pricing Methods.
    /**
     * @param[in] S  underlying (spot) price
     * @param[in] r  risk-free interest rate
     * @param[in] b  cost-of-carry rate of holding underlying
     * @param[in] sigma  volatility of underlying
     * @param[in] T  time to expiration (years)
     * @param[in] european  @c true for european style option (exercise at expiry only), @c false for american style (exercise any time)
     * @return  pointer to pricing method
     */
    virtual AbstractOptionPricing *createPricingMethod( double S, double r, double b, double sigma, double T, bool european = false ) const = 0;

    /// Factory method for creation of Option Pricing Methods.
    /**
     * @warning
     * Passed in @c vector classes @p divTimes and @p divYields are assumed to have equal sizes.
     * @param[in] S  underlying (spot) price
     * @param[in] r  risk-free interest rate
     * @param[in] b  cost-of-carry rate of holding underlying
     * @param[in] sigma  volatility of underlying
     * @param[in] T  time to expiration (years)
     * @param[in] divTimes  dividend times
     * @param[in] divYields  dividend yields
     * @param[in] european  @c true for european style option (exercise at expiry only), @c false for american style (exercise any time)
     * @return  pointer to pricing method
     */
    virtual AbstractOptionPricing *createPricingMethod( double S, double r, double b, double sigma, double T, const std::vector<double>& divTimes, const std::vector<double>& divYields, bool european = false ) const = 0;

    /// Factory method for destruction of Option Pricing Methods.
    /**
     * @param[in] doomed  pricing method to destroy
     */
    virtual void destroyPricingMethod( AbstractOptionPricing *doomed ) const = 0;

private:

    struct Greeks
    {
        double bid;
        double bidvi;

        double ask;
        double askvi;

        double spread;
        double spreadPercent;

        double mark;
        double markvi;

        double timeToExpiry;
        double riskFreeRate;

        double price;
        double vi;

        double delta;
        double gamma;
        double theta;
        double vega;
        double rho;

        double marketPrice;
    };

    using OptionGreeks = QMap<double, Greeks>;

    OptionGreeks greeksCall_;
    OptionGreeks greeksPut_;

    struct ProbCurve
    {
        double min;
        double minvi;

        double max;
        double maxvi;

        double price;
        double vi;

        double delta;
    };

    using OptionProbCurve = QMap<double, ProbCurve>;

    OptionProbCurve probCurveCall_;
    OptionProbCurve probCurvePut_;

    QList<double> asc_;
    QList<double> desc_;

    QMap<double, double> probCurve_;

    double underlyingMin_;
    double underlyingMax_;

    /// Analyze single covered call.
    void analyzeSingleCall( int row ) const;

    /// Analyze single cash secured put.
    void analyzeSinglePut( int row ) const;

    /// Analyze vertical bear call.
    void analyzeVertBearCall( int rowLong, int rowShort ) const;

    /// Analyze vertical bull put.
    void analyzeVertBullPut( int rowLong, int rowShort ) const;

    /// Generate greeks.
    /**
     * Parse chain row data and calculate vi and greeks.
     */
    bool generateGreeks( int row, double strike, bool isCall );

    /// Calculate probability curve.
    /**
     * Iterate over probability curve data and adjust min/max vi for fitting.
     */
    bool calcProbCurve( OptionProbCurve& curve, const QList<double>& direction, bool isCall ) const;

    /// Calculate probability curve price.
    /**
     * Iterate over probability curve data and adjust price and vi for fitting.
     */
    bool calcProbCurvePrices( OptionProbCurve& curve, const QList<double>& direction, bool isCall );

    /// Generate probability curve.
    /**
     * Parse greeks and create probability data.
     */
    bool generateProbCurve( double strike, bool isCall );

    /// Generate probability curve prom call/put parity.
    /**
     * Parse greeks and create probability data.
     */
    bool generateProbCurveParity( double strike, bool isCall );

    /// Calculate greeks for option.
    bool calcGreeks( AbstractOptionPricing *o, double theoOptionValue, double strike, bool isCall, Greeks& result ) const;

    /// Calculate probability of ITM.
    double calcProbInTheMoney( double price, bool isCall ) const;

    /// Calculate expected loss.
    double calcExpectedLoss( double multiplier, double priceMin, double priceMax, double costBasis, double totalProb, bool isCall ) const;

    /// Populate result model greeks.
    static void populateResultModelGreeks( const Greeks& g, item_model_type::ColumnValueMap &result );

    /// Populate result model greeks.
    static void populateResultModelGreeksSpread( const Greeks& glong, const Greeks& gshort, item_model_type::ColumnValueMap &result );

    // not implemented
    ExpectedValueCalculator( const _Myt& ) = delete;

    // not implemented
    ExpectedValueCalculator( const _Myt&& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // EXPECTEDVALUECALC_H
