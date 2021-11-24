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

#include "optionprofitcalcfilter.h"

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

    /// Filter type.
    using filter_type = OptionProfitCalculatorFilter;

    // ========================================================================
    // DTOR
    // ========================================================================

    /// Destructor.
    virtual ~OptionProfitCalculator();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve filter.
    /**
     * @return  filter
     */
    virtual filter_type filter() const {return f_;}

    /// Set cost basis.
    /**
     * @param[in] value  amount
     */
    virtual void setCostBasis( double value ) {costBasis_ = value;}

    /// Set equity trading cost (i.e. buy or sell underlying).
    /**
     * For example, how much it costs to acquire 100 shares of XYZ.
     * @param[in] value  trade cost
     */
    virtual void setEquityTradeCost( double value ) {equityTradeCost_ = value;}

    /// Set filter.
    /**
     * @param[in] value  filter
     */
    virtual void setFilter( const filter_type& value ) {f_ = value;}

    /// Set option trading cost.
    /**
     * @param[in] value  trade cost
     */
    virtual void setOptionTradeCost( double value ) {optionTradeCost_ = value;}

    // ========================================================================
    // Methods
    // ========================================================================

    /// Analyze option chain.
    /**
     * @param[out] strat  trading strategy
     */
    virtual void analyze( item_model_type::Strategy strat ) = 0;

    // ========================================================================
    // Static Methods
    // ========================================================================

    /// Create option profic calculator.
    /**
     * Factory method to create a calculator from configuration settings.
     * @param[in] underlying  underlying price (i.e. mark)
     * @param[in] chains  chains to evaluate
     * @param[in] results  results
     * @return  calculator
     */
    static _Myt *create( double underlying, const table_model_type *chains, item_model_type *results );

    /// Destroy option profic calculator.
    /**
     * Factory method to destroy allocated calculator.
     * @param[in] calc  calculator
     */
    static void destroy( _Myt *calc );

protected:

    bool valid_;                                    ///< Chain is valid.

    int daysToExpiry_;                              ///< Days to expiration.

    double underlying_;                             ///< Underlying price.

    double totalDivAmount_;                         ///< Total dividend amount (expected).
    double totalDivYield_;                          ///< Total dividend yield (expected).

    const table_model_type *chains_;                ///< Chains for analysis.

    item_model_type *results_;                      ///< Results.

    // ---- //

    std::vector<double> divTimes_;                  ///< Dividend times.
    std::vector<double> div_;                       ///< Dividend yields.

    double costBasis_;                              ///< Cost basis (used for call options).

    double equityTradeCost_;                        ///< Cost to trade a stock (equity).
    double optionTradeCost_;                        ///< Cost to trade an option.

    filter_type f_;                                 ///< Filter.

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
    // Properties
    // ========================================================================

    /// Check if row is filtered out.
    /**
     * @param[in] row  table row
     * @param[in] isCall  @c true if call option, @c false otherwise
     * @return  @c true if filtered out, @c false otherwise
     */
    virtual bool isFilteredOut( int row, bool isCall ) const;

    /// Check if row is non-standard option.
    /**
     * @param[in] row  table row
     * @return  @c true if non-standard, @c false otherwise
     */
    virtual bool isNonStandard( int row ) const;

    // ========================================================================
    // Methods
    // ========================================================================

    /// Add result to item model.
    /**
     * @param[in] result
     */
    virtual void addRowToItemModel( const item_model_type::ColumnValueMap& result ) const;

    /// Populate result model with put/call information.
    /**
     * @param[in] row  row
     * @param[in] isCall  @c true if call option type, @c false otherwise
     * @param[in,out] result  result
     */
    virtual void populateResultModelSingle( int row, bool isCall, item_model_type::ColumnValueMap& result ) const;

    /// Populate result model with put/call information.
    /**
     * @param[in] rowLong  long option row
     * @param[in] rowShort  short option row
     * @param[in] isCall  @c true if call option type, @c false otherwise
     * @param[in,out] result  result
     */
    virtual void populateResultModelVertical( int rowLong, int rowShort, bool isCall, item_model_type::ColumnValueMap& result ) const;

private:

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
