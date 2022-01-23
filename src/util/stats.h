/**
 * @file stats.h
 * Statistical methods utility object.
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

#ifndef STATS_H
#define STATS_H

#include <QVector>

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Statistical methods utility object.
class Stats
{
    using _Myt = Stats;

public:

    // ========================================================================
    // Static Methods
    // ========================================================================

    /// Calculate mean.
    /**
     * @param[in] data  data to evaluate
     * @return  mean
     */
    static double calcMean( const QVector<double>& data );

    /// Calculate standard deviation.
    /**
     * @param[in] data  data to evaluate
     * @return  standard deviation
     */
    static double calcStdDeviation( const QVector<double>& data );

private:

    // not implemented
    Stats() = delete;

    // not implemented
    Stats( const _Myt& ) = delete;

    // not implemented
    Stats( const _Myt&& ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // STATS_H
