/**
 * @file stats.cpp
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

#include "stats.h"

#include <cmath>

///////////////////////////////////////////////////////////////////////////////////////////////////
double Stats::calcMean( const QVector<double>& data )
{
    double sum( 0.0 );

    foreach ( double v, data )
        sum += v;

    return sum / data.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double Stats::calcStdDeviation( const QVector<double>& data )
{
    const double mean( calcMean( data ) );

    double variance( 0.0 );

    foreach ( double v, data )
        variance += pow( v - mean, 2 );

    return sqrt( variance / data.size() );
}

