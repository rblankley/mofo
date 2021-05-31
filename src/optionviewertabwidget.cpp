/**
 * @file optionviewertabwidget.cpp
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

#include "abstractdaemon.h"
#include "common.h"
#include "optionviewertabwidget.h"
#include "optionviewerwidget.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionViewerTabWidget::OptionViewerTabWidget( QWidget *parent ) :
    _Mybase( parent )
{
    // init
    initialize();
    createLayout();
    translate();

    connect( AbstractDaemon::instance(), &AbstractDaemon::optionChainUpdated, this, &_Myt::onOptionChainUpdated );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionViewerTabWidget::~OptionViewerTabWidget()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionViewerTabWidget::translate()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionViewerTabWidget::createUnderlying( const QString& symbol )
{
    bool shown( false );

    QWidget *w( findUnderlying( symbol, shown ) );

    if ( !w )
    {
        w = new OptionViewerWidget( symbol, this );
        w->setObjectName( symbol );

        w->setVisible( false );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QWidget *OptionViewerTabWidget::findUnderlying( const QString& symbol, bool& shown )
{
    QWidget *w( nullptr );

    // locate existing tab with symbol
    for ( int i( count() ); i--; )
        if ( symbol == tabText( i ) )
        {
            w = widget( i );
            shown = true;

            break;
        }

    // otherwise try to find child widget
    if ( !w )
        w = findChild<OptionViewerWidget*>( symbol );

    return w;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionViewerTabWidget::onOptionChainUpdated( const QString& symbol, const QList<QDate>& expiryDates, bool background )
{
    Q_UNUSED( expiryDates )

    // ignore background requests
    if ( background )
        return;

    bool shown( false );

    QWidget *w( findUnderlying( symbol, shown ) );

    if ( w )
    {
        // does not exist; create a new tab
        if ( !shown )
        {
            const int idx( addTab( w, symbol ) );

            // make sure tab is shown
            setCurrentIndex( idx );
        }

        w->setVisible( true );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionViewerTabWidget::onTabCloseRequested( int index )
{
    QWidget *w( widget( index ) );

    removeTab( index );
    w->deleteLater();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionViewerTabWidget::initialize()
{
    setMovable( true );
    setTabsClosable( true );
    setTabPosition( QTabWidget::East );

    connect( this, &QTabWidget::tabCloseRequested, this, &_Myt::onTabCloseRequested );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionViewerTabWidget::createLayout()
{
}

