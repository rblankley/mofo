/**
 * @file itemmodel.cpp
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

#include "common.h"
#include "itemmodel.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
ItemModel::ItemModel( int rows, int columns, QObject *parent ) :
    _Mybase( rows, columns, parent ),
    m_( QMutex::Recursive ),
    columnIsCurrency_( columns, false )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ItemModel::~ItemModel()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant ItemModel::data( int row, int col, int role ) const
{
    QMutexLocker guard( &m_ );
    return index( row, col ).data( role );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant ItemModel::data0( int col, int role ) const
{
    QMutexLocker guard( &m_ );
    return index( 0, col ).data( role );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool ItemModel::isColumnCurrency( int col ) const
{
    return columnIsCurrency_[col];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ItemModel::removeAllRows()
{
    QMutexLocker guard( &m_ );

    if ( !rowCount() )
        return;

    removeRows( 0, rowCount() );
}

