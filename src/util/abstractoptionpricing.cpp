/**
 * @file abstractoptionpricing.cpp
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

#include "abstractoptionpricing.h"

#include <cmath>

///////////////////////////////////////////////////////////////////////////////////////////////////
AbstractOptionPricing::AbstractOptionPricing( double S, double r, double b, double sigma, double T ) :
    S_( S ),
    r_( r ),
    b_( b ),
    sigma_( sigma ),
    T_( T )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double AbstractOptionPricing::calcImplVolSeedValue( double X ) const
{
    return sqrt( fabs( log( S_ / X ) + r_ * T_ ) * (2.0 / T_) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AbstractOptionPricing::copy( const _Myt& other )
{
    S_ = other.S_;
    r_ = other.r_;
    b_ = other.b_;
    sigma_ = other.sigma_;
    T_ = other.T_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AbstractOptionPricing::move( const _Myt&& other )
{
    S_ = std::move( other.S_ );
    r_ = std::move( other.r_ );
    b_ = std::move( other.b_ );
    sigma_ = std::move( other.sigma_ );
    T_ = std::move( other.T_ );
}
