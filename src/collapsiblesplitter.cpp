/**
 * @file collapsiblesplitter.cpp
 *
 * @copyright Copyright (C) 2022 Randy Blankley. All rights reserved.
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

#include "collapsiblesplitter.h"

#include <QHBoxLayout>
#include <QSplitterHandle>
#include <QToolButton>
#include <QVBoxLayout>

///////////////////////////////////////////////////////////////////////////////////////////////////
CollapsibleSplitter::CollapsibleSplitter( Qt::Orientation orientation, QWidget *parent ) :
    _Mybase( orientation, parent ),
    loc_( Qt::TopEdge )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CollapsibleSplitter::CollapsibleSplitter( QWidget *parent ) :
    _Mybase( parent ),
    loc_( Qt::TopEdge )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CollapsibleSplitter::~CollapsibleSplitter()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CollapsibleSplitter::addWidget( QWidget *widget )
{
    // add
    _Mybase::addWidget( widget );

    const int index( count() - 1 );

    // create button to handle collapsing
    if ( 0 < index )
    {
        const bool buttonTopLeft(( Qt::LeftEdge == buttonLocation() ) || ( Qt::TopEdge == buttonLocation() ));

        QSplitterHandle *h( handle( index ) );
        QToolButton *b( new QToolButton( h ) );

        connect( b, &QToolButton::clicked, this, &_Myt::onButtonClicked );

        // create new layout
        QBoxLayout *layout;

        if ( Qt::Horizontal == orientation() )
        {
            b->setArrowType( Qt::RightArrow );
            layout = new QVBoxLayout( h );
        }
        else if ( Qt::Vertical == orientation() )
        {
            b->setArrowType( Qt::DownArrow );
            layout = new QHBoxLayout( h );
        }
        else
        {
            return;
        }

        // add button to layout
        layout->setContentsMargins( QMargins() );

        if ( buttonTopLeft )
            layout->addWidget( b );
        layout->addStretch();

        if ( !buttonTopLeft )
            layout->addWidget( b );

        // save button index
        buttons_[b] = index;

        while ( sizes_.size() < count() )
            sizes_.append( 0 );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CollapsibleSplitter::onButtonClicked()
{
    QToolButton *b( qobject_cast<QToolButton*>( sender() ) );

    if ( !buttons_.contains( b ) )
        return;

    const int index( buttons_[b] );

    QList<int> widths( sizes() );

    // hide
    if ( widths[index] )
    {
        // save size
        sizes_[index] = widths[index];

        // update arrow
        if ( Qt::Horizontal == orientation() )
            b->setArrowType( Qt::LeftArrow );
        else if ( Qt::Vertical == orientation() )
            b->setArrowType( Qt::UpArrow );

        widths[index] = 0;
    }

    // show
    else
    {
        const int prevSize( sizes_[index] );

        // restore size
        if ( !prevSize )
            widths[index] = 1;
        else
        {
            widths[index] = prevSize;
            widths[index-1] = qMax( 1, widths[index-1] - prevSize );
        }

        // update arrow
        if ( Qt::Horizontal == orientation() )
            b->setArrowType( Qt::RightArrow );
        else if ( Qt::Vertical == orientation() )
            b->setArrowType( Qt::DownArrow );
    }

    // resize
    setSizes( widths );
}
