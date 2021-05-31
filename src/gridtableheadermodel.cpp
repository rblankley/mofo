/**
 * @file gridtableheadermodel.cpp
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

#include "gridtableheadermodel.h"
#include "tableheaderitem.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
GridTableHeaderModel::GridTableHeaderModel( int row, int column, QObject *parent ) :
    _Mybase( parent ),
    rootItem_( new item_type ),
    row_( row ),
    column_( column )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
GridTableHeaderModel::~GridTableHeaderModel()
{
    delete rootItem_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QModelIndex GridTableHeaderModel::index( int row, int column, const QModelIndex& parent ) const
{
    if ( !hasIndex( row, column, parent ) )
        return QModelIndex();

    item_type *parentItem;

    if ( !parent.isValid() )
        parentItem = rootItem_;
    else
        parentItem = static_cast<item_type*>( parent.internalPointer() );

    item_type *childItem( parentItem->child( row, column ) );

    if ( !childItem )
        childItem = parentItem->insertChild( row, column );

    return createIndex( row, column, childItem );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int GridTableHeaderModel::rowCount( const QModelIndex& parent ) const
{
    Q_UNUSED( parent )
    return row_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int GridTableHeaderModel::columnCount( const QModelIndex& parent ) const
{
    Q_UNUSED( parent )
    return column_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Qt::ItemFlags GridTableHeaderModel::flags( const QModelIndex& index ) const
{
    return (index.isValid() ? QAbstractTableModel::flags( index ) : Qt::NoItemFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant GridTableHeaderModel::data( const QModelIndex& index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();
    else if (( row_ <= index.row() ) || ( index.row() < 0 ) || ( column_ <= index.column() ) || ( index.column() < 0 ))
        return QVariant();

    const item_type *item( static_cast<item_type*>( index.internalPointer() ) );

    return item->data( role );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool GridTableHeaderModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    if ( !index.isValid() )
        return false;

    item_type *item( static_cast<item_type*>( index.internalPointer() ) );

    if ( ColumnSpanRole == role )
    {
        int span( value.toInt() );

        if ( 0 < span )
        {
            const int col( index.column() );

            if ( column_ <= (col + span - 1) )
                span = column_ - col;

            item->setData( span, ColumnSpanRole );
        }
    }
    else if ( RowSpanRole == role )
    {
        int span( value.toInt() );

        if ( 0 < span )
        {
            const int row( index.row() );

            if ( row_ < (row + span - 1) )
                span = column_ - row;

            item->setData( span, RowSpanRole );
        }
    }
    else
    {
        item->setData( value, role );
    }

    return true;
}
