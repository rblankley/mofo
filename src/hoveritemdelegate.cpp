/**
 * @file hoveritemdelegate.cpp
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

#include "hoveritemdelegate.h"

#include <QApplication>
#include <QPainter>

///////////////////////////////////////////////////////////////////////////////////////////////////
HoverItemDelegate::HoverItemDelegate( QObject *parent ) :
    _Mybase( parent ),
    row_( -1 )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
HoverItemDelegate::~HoverItemDelegate()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void HoverItemDelegate::paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    QStyleOptionViewItem opt( option );
    initStyleOption( &opt, index );

    if (( row_ == index.row() ) && ( from_ <= index.column() ) && ( index.column() <= to_ ))
        opt.state |= QStyle::State_MouseOver;
    else
        opt.state &= ~QStyle::State_MouseOver;

    // ---- //

    const QWidget *w( opt.widget );
    const QStyle *style( w ? w->style() : QApplication::style() );

    // highlight background
    if ( QStyle::State_MouseOver & opt.state )
        opt.backgroundBrush = opt.palette.brush( QPalette::Highlight );

    painter->save();
    style->drawControl( QStyle::CE_ItemViewItem, &opt, painter, w );
    painter->restore();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void HoverItemDelegate::setHoverRegion( int row, int from, int to )
{
    row_ = row;
    from_ = from;
    to_ = to;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void HoverItemDelegate::clearHoverRegion()
{
    row_ = -1;
}
