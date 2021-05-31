/**
 * @file abstractoptionpricing.h
 * Abstract Option Pricing.
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

#ifndef ABSTRACTOPTIONPRICING_H
#define ABSTRACTOPTIONPRICING_H

#include "optiontype.h"

#include <cmath>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Abstract Option Pricing.
class AbstractOptionPricing
{
    using _Myt = AbstractOptionPricing;

public:

    // ========================================================================
    // Properties
    // ========================================================================

    /// Check for american style option.
    /**
     * @return  @c true if american, @c false otherwise
     */
    virtual bool isAmerican() const {return !isEuropean();}

    /// Check for european style option.
    /**
     * @return  @c true if european, @c false otherwise
     */
    virtual bool isEuropean() const = 0;

    /// Compute option price.
    /**
     * @param[in] type  option type
     * @param[in] X  strike price
     * @return  option price
     */
    virtual double optionPrice( OptionType type, double X ) const = 0;

    /// Set new volatility.
    /**
     * @param[in] value  volatility of underlying
     */
    virtual void setSigma( double value ) {sigma_ = value;}

    /// Retrieve volatility.
    /**
     * @return  volatility of underlying
     */
    virtual double sigma() const {return sigma_;}

    // ========================================================================
    // Methods
    // ========================================================================

    /// Calculate the Manaster and Koehler seed value.
    /**
     * @param[in] X  strike price
     * @return  seed value for implied volatility
     */
    virtual double calcImplVolSeedValue( double X ) const;

protected:

    double S_;                                      ///< underlying price
    double r_;                                      ///< risk-free interest rate
    double b_;                                      ///< cost-of-carry rate of holding underlying
    double sigma_;                                  ///< volatility of underlying
    double T_;                                      ///< time to expiration (years)

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    AbstractOptionPricing() {}

    /// Constructor.
    /**
     * @param[in] S  underlying price
     * @param[in] r  risk-free interest rate
     * @param[in] b  cost-of-carry rate of holding underlying
     * @param[in] sigma  volatility of underlying
     * @param[in] T  time to expiration (years)
     */
    AbstractOptionPricing( double S, double r, double b, double sigma, double T );

    /// Constructor.
    /**
     * @param[in] other  object to copy
     */
    AbstractOptionPricing( const _Myt& other ) {copy( other );}

    /// Constructor.
    /**
     * @param[in] other  object to move
     */
    AbstractOptionPricing( const _Myt&& other ) {move( std::move( other ) );}

    /// Destructor.
    virtual ~AbstractOptionPricing() {}

    // ========================================================================
    // Methods
    // ========================================================================

    /// Copy object.
    /**
     * @param[in] rhs  object to copy
     * @return  reference to this
     */
    void copy( const _Myt& other );

    /// Move object.
    /**
     * @param[in] rhs  object to move
     * @return  reference to this
     */
    void move( const _Myt&& other );

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // ABSTRACTOPTIONPRICING_H
