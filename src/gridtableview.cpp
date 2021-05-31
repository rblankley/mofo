/**
 * @file gridtableview.cpp
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

#include "gridtableview.h"
#include "gridtableheaderview.h"

#include <QStandardItemModel>

///////////////////////////////////////////////////////////////////////////////////////////////////
GridTableView::GridTableView( QWidget *parent ) :
    _Mybase( parent )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
GridTableView::~GridTableView()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GridTableView::setGridHeaderView( Qt::Orientation orientation, int levels )
{
    GridTableHeaderView *header;

    if ( Qt::Horizontal == orientation )
        setHorizontalHeader( (header = new GridTableHeaderView( orientation, levels, model()->columnCount(), this )) );
    else if ( Qt::Vertical == orientation )
        setVerticalHeader( (header = new GridTableHeaderView( orientation, model()->rowCount(), levels, this )) );
    else
    {
        return;
    }

    connect( header, &GridTableHeaderView::sectionPressed, this, &_Myt::onHeaderSectionPressed );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
GridTableHeaderView *GridTableView::gridHeaderView( Qt::Orientation orientation ) const
{
    GridTableHeaderView *header( nullptr );

    if ( Qt::Horizontal == orientation )
        header = qobject_cast<GridTableHeaderView*>( horizontalHeader() );
    else if ( Qt::Vertical == orientation )
        header = qobject_cast<GridTableHeaderView*>( verticalHeader() );

    return header;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GridTableView::onHeaderSectionPressed( const QPoint& pos, Qt::MouseButton button, int beginSection, int endSection )
{
    Q_UNUSED( pos );

    if ( Qt::LeftButton != button )
        return;

    const QAbstractItemView::SelectionMode oldSelectionMode( selectionMode() );

    // no selection allowed
    if ( QAbstractItemView::NoSelection == oldSelectionMode )
        return;

    const bool isHor( sender() == horizontalHeader() );
    const bool isVert( sender() == verticalHeader() );

    // only row selection allowed
    if (( isHor ) && ( QAbstractItemView::SelectRows != selectionBehavior() ))
        return;
    // only column selection allowed
    else if (( isVert ) && ( QAbstractItemView::SelectColumns != selectionBehavior() ))
        return;

    // select rows
    clearSelection();
    setSelectionMode( QAbstractItemView::MultiSelection );

    if ( isHor )
    {
        for ( int i( beginSection ); i <= endSection; ++i )
            selectColumn( i );
    }
    else if ( isVert )
    {
        for ( int i( beginSection ); i <= endSection; ++i )
            selectRow( i );
    }

    setSelectionMode( oldSelectionMode );
}

