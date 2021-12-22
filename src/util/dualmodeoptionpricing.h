/**
 * @file dualmodeoptionpricing.h
 * Dual mode (European and American) option pricing.
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

#ifndef DUALMODEOPTIONPRICING_H
#define DUALMODEOPTIONPRICING_H

#include "blackscholes.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Dual mode (European and American) option pricing.
class DualModeOptionPricing : public BlackScholes
{
    using _Myt = DualModeOptionPricing;
    using _Mybase = BlackScholes;

public:

    // ========================================================================
    // Properties
    // ========================================================================

    /// Check for european style option.
    /**
     * @return  @c true if european, @c false otherwise
     */
    virtual bool isEuropean() const override {return european_;}

    /// Set american style option.
    /**
     * @param[in] value  @c true if american, @c false otherwise
     */
    virtual void setAmerican( bool value ) {setEuropean( !value );}

    /// Set european style option.
    /**
     * @param[in] value  @c true if european, @c false otherwise
     */
    virtual void setEuropean( bool value ) {european_ = value;}

protected:

    bool european_;                                 ///< Flag for european style option when @c true, or american style (early exercise) when @c false.

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    DualModeOptionPricing() {}

    /// Constructor.
    /**
     * @param[in] S  underlying price
     * @param[in] r  risk-free interest rate
     * @param[in] b  cost-of-carry rate of holding underlying
     * @param[in] sigma  volatility of underlying
     * @param[in] T  time to expiration (years)
     * @param[in] european  @c true for european style option (exercise at expiry only), @c false for american style (exercise any time)
     */
    DualModeOptionPricing( double S, double r, double b, double sigma, double T, bool european = false ) :
        _Mybase( S, r, b, sigma, T ),
        european_( european ) {}

    /// Constructor.
    /**
     * @param[in] other  object to copy
     */
    DualModeOptionPricing( const _Myt& other ) : _Mybase() {copy( other );}

    /// Constructor.
    /**
     * @param[in] other  object to move
     */
    DualModeOptionPricing( const _Myt&& other ) : _Mybase() {move( std::move( other ) );}

    // ========================================================================
    // Methods
    // ========================================================================

    /// Copy object.
    /**
     * @param[in] other  object to copy
     * @return  reference to this
     */
    void copy( const _Myt& other )
    {
        _Mybase::copy( other );
        european_ = other.european_;
    }

    /// Move object.
    /**
     * @param[in] other  object to move
     * @return  reference to this
     */
    void move( const _Myt&& other )
    {
        _Mybase::move( std::move( other ) );
        european_ = std::move( other.european_ );
    }

};


///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // DUALMODEOPTIONPRICING_H
