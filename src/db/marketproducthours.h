/**
 * @file marketproducthours.h
 * Market hours by product type.
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

#ifndef MARKETPRODUCTHOURS_H
#define MARKETPRODUCTHOURS_H

#include <QDateTime>

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Market hours by product type.
struct MarketProductHours
{
    QDateTime preMarketStart;
    QDateTime preMarketEnd;
    QDateTime regularMarketStart;
    QDateTime regularMarketEnd;
    QDateTime postMarketStart;
    QDateTime postMarketEnd;
};

/// Comparison operator.
/**
 * @param[in] lhs  object to compare
 * @param[in] rhs  object to compare
 * @return  @c true if equal, @c false otherwise
 */
inline bool operator == ( const MarketProductHours& lhs, const MarketProductHours& rhs )
{
    return (( lhs.preMarketStart == rhs.preMarketStart ) && ( lhs.preMarketEnd == rhs.preMarketEnd ) &&
        ( lhs.regularMarketStart == rhs.regularMarketStart ) && ( lhs.regularMarketEnd == rhs.regularMarketEnd ) &&
        ( lhs.postMarketStart == rhs.postMarketStart ) && ( lhs.postMarketEnd == rhs.postMarketEnd ));
}


///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MARKETPRODUCTHOURS_H
