/**
 * @file binomialcalc.h
 * Binomial calculator (template class).
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

#ifndef BINOMIALCALC_H
#define BINOMIALCALC_H

#include "abstractevcalc.h"

#include "util/coxrossrubinstein.h"
#include "util/equalprobbinomial.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Binomial calculator (template class).
/**
 * @tparam C  option pricing method
 * @tparam VI  implied volatility calculation method
 */
template <class C, class VI = NewtonRaphson>
class BinomialCalculator : public AbstractExpectedValueCalculator<C, VI>
{
    using _Myt = BinomialCalculator<C, VI>;
    using _Mybase = AbstractExpectedValueCalculator<C, VI>;

public:

    /// Table model type.
    using table_model_type = typename _Mybase::table_model_type;

    /// Item model type.
    using item_model_type = typename _Mybase::item_model_type;

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] underlying  underlying price (i.e. mark)
     * @param[in] chains  chains to evaluate
     * @param[in] results  results
     */
    BinomialCalculator( double underlying, const table_model_type *chains, item_model_type *results ) :
        _Mybase( underlying, chains, results ) {}

    /// Destructor.
    ~BinomialCalculator() {}

protected:

    /// Option pricing method type.
    using pricing_method_type = typename _Mybase::pricing_method_type;

    // ========================================================================
    // Methods
    // ========================================================================

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
    virtual AbstractOptionPricing *createPricingMethod( double S, double r, double b, double sigma, double T, bool european = false ) const override
    {
        return new pricing_method_type( S, r, b, sigma, T, DEPTH, european );
    }

    /// Factory method for creation of Option Pricing Methods.
    /**
     * @warning
     * Passed in @c vector classes @a divTimes and @a divYields are assumed to have equal sizes.
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
    virtual AbstractOptionPricing *createPricingMethod( double S, double r, double b, double sigma, double T, const std::vector<double>& divTimes, const std::vector<double>& divYields, bool european = false ) const override
    {
        // for this mode we should have no dividend already passed in
        assert( b == r );

        return new pricing_method_type( S, r, b, sigma, T, DEPTH, divTimes, divYields, european );
    }

private:

    static constexpr int DEPTH = 256;

    // not implemented
    BinomialCalculator( const _Myt& ) = delete;

    // not implemented
    BinomialCalculator( const _Myt&& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // BINOMIALCALC_H
