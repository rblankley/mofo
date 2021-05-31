/**
 * @file tableheaderitem.cpp
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

#include "tableheaderitem.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
TableHeaderItem::TableHeaderItem( int row, int column, _Myt *parent ) :
    parentItem_( parent ),
    row_( row ),
    column_( column )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TableHeaderItem::TableHeaderItem( _Myt *parent ) :
    TableHeaderItem( 0, 0, parent )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TableHeaderItem::~TableHeaderItem()
{
    clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TableHeaderItem *TableHeaderItem::insertChild( int row, int col )
{
    _Myt *child( new _Myt( row, col, this ) );
    childItems_.insert( RowColumnPair( row, col ), child );
    return child;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TableHeaderItem *TableHeaderItem::child( int row, int col )
{
    ChildItemsHashTable::const_iterator it( childItems_.constFind( RowColumnPair( row, col ) ) );

    if (  childItems_.constEnd() != it )
        return it.value();

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TableHeaderItem::setData( const QVariant& data, int role )
{
    itemData_.insert( role, data );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant TableHeaderItem::data( int role ) const
{
    ItemDataHashTable::const_iterator it( itemData_.constFind( role ) );

    if ( itemData_.constEnd() != it )
        return it.value();

    return QVariant();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TableHeaderItem::clear()
{
    for ( _Myt *item : childItems_ )
        if ( item )
        {
            item->clear();
            delete item;
        }

    childItems_.clear();
}
