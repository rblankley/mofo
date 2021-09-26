/**
 * @file fitpoly.cpp
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
 *
 * @note
 * This code adapted from libmetaoptions - A collection of option-related functions.
 * Copyright (C) 2000-2004 B. Augestad, bjorn.augestad@gmail.com
 */

#include "common.h"
#include "fitpoly.h"

#include <algorithm>
#include <cmath>

///////////////////////////////////////////////////////////////////////////////////////////////////
void fit_polynomial( const std::vector<vector2>& v, double& x2, double& x1, double& x0 )
{
    //const size_t N( v.size() );
    const int N( v.size() );

    //size_t i, j, k;
    int i, j, k;

    // generate augmented matrix
    double B[3][4] = {0.0};

    for (k = 0; k < 3; k++)
    {
        for (i = 0; i < N; i++)
        {
            for (j = 0; j < 3; j++)
                B[k][j] += pow(v[i].x, j + k);

            B[k][3] += v[i].y*pow(v[i].x, k);
        }
    }

    // invert matrix
    for(k = 0; k < 3; k++)
    {
        // divide row by B[k][k]
        double q = B[k][k];

        for(i = 0; i < 4; i++)
            B[k][i] /= q;

        // zero out column B[][k]
        for(j = 0; j < 3; j++)
        {
            if(j == k)
                continue;
            double m = B[j][k];
            for(i = 0; i < 4; i++)
                B[j][i] -= m*B[k][i];
        }
    }

    x2 = B[2][3];
    x1 = B[1][3];
    x0 = B[0][3];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void fit_linear( const std::vector<vector2>& v, double& x2, double& x1, double& x0 )
{
    const size_t N( v.size() );

    // fail: infinitely many lines passing through this single point
    if ( N < 2 )
        return;

    double sumX( 0 );
    double sumY( 0 );
    double sumXY( 0 );
    double sumX2( 0 );

    for ( size_t i( 0 ); i < N; ++i )
    {
        sumX += v[i].x;
        sumY += v[i].y;
        sumXY += v[i].x * v[i].y;
        sumX2 += pow( v[i].x, 2 );
    }

    const double xMean( sumX / N );
    const double yMean( sumY / N );
    const double denominator( sumX2 - sumX * xMean );

    x2 = 0.0;
    x1 = (sumXY - sumX * yMean) / denominator;
    x0 = yMean - x1 * xMean;
}
