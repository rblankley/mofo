/**
 * @file abstractevcalc.h
 * Abstract expected value calculator (template class).
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

#ifndef ABSTRACTEVCALC_H
#define ABSTRACTEVCALC_H

#include "expectedvaluecalc.h"

#include "util/altbisection.h"
#include "util/newtonraphson.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Abstract expected value calculator (template class).
/**
 * @tparam C  option pricing method
 * @tparam VI  implied volatility calculation method
 */
template <class C, class VI = NewtonRaphson>
class AbstractExpectedValueCalculator : public ExpectedValueCalculator
{
    using _Myt = AbstractExpectedValueCalculator<C, VI>;
    using _Mybase = ExpectedValueCalculator;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] underlying  underlying price (i.e. mark)
     * @param[in] chains  chains to evaluate
     * @param[in] results  results
     */
    AbstractExpectedValueCalculator( double underlying, const table_model_type *chains, item_model_type *results ) :
        _Mybase( underlying, chains, results ) {}

    /// Destructor.
    ~AbstractExpectedValueCalculator() {}

protected:

    /// Option pricing method type.
    using pricing_method_type = C;

    /// Implied volatility calculation method type.
    using implied_volatility_method_type = VI;

    // ========================================================================
    // Methods
    // ========================================================================

    /// Calculate implied volatility.
    /**
     * @param[in,out] pricing  option pricing
     * @param[in] type  option type
     * @param[in] X  strike price
     * @param[in] price  option price
     * @param[out] okay  @c true if calculation okay, @c false otherwise
     * @return  implied volatility of @a pricing
     */
    virtual double calcImplVol( AbstractOptionPricing *pricing, OptionType type, double X, double price, bool *pokay = nullptr ) const override
    {
        bool okay;

        // primary method
        double vi = implied_volatility_method_type::calcImplVol( dynamic_cast<pricing_method_type*>( pricing ), type, X, price, &okay );

        // alt method for VI calculation (if applicable)
        if (( !okay ) && ( !std::is_same<AlternativeBisection, implied_volatility_method_type>::value ))
            vi = AlternativeBisection::calcImplVol( dynamic_cast<pricing_method_type*>( pricing ), type, X, price, &okay );

        if ( pokay )
            *pokay = okay;

        return vi;
    }

    /// Factory method for destruction of Option Pricing Methods.
    /**
     * @param[in] doomed  pricing method to destroy
     */
    virtual void destroyPricingMethod( AbstractOptionPricing *doomed ) const override
    {
        if ( doomed )
            delete dynamic_cast<pricing_method_type*>( doomed );
    }

private:

    // not implemented
    AbstractExpectedValueCalculator( const _Myt& ) = delete;

    // not implemented
    AbstractExpectedValueCalculator( const _Myt&& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // ABSTRACTEVCALC_H
