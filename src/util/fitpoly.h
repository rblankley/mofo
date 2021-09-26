/**
 * @file fitpoly.h
 * Best fit methods for polynomial generation.
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

#ifndef FITPOLY_H
#define FITPOLY_H

#include <vector>

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Vector of values.
struct vector2
{
    double x;                                       ///< X Coordinate.
    double y;                                       ///< Y Coordinate.
};

/// Fit second order polynomial.
/**
 * @param[in] v  values
 * @param[out] x2  second order coefficient
 * @param[out] x1  first order coefficient
 * @param[out] x0  constant value
 */
void fit_polynomial( const std::vector<vector2>& v, double& x2, double& x1, double& x0 );

/// Fit linear.
/**
 * @param[in] v  values
 * @param[out] x2  second order coefficient
 * @param[out] x1  first order coefficient
 * @param[out] x0  constant value
 */
void fit_linear( const std::vector<vector2>& v, double& x2, double& x1, double& x0 );

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // FITPOLY_H
