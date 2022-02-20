/**
 * @file symboldetailsdialog.cpp
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

#include "collapsiblesplitter.h"
#include "common.h"
#include "fundamentalsviewerwidget.h"
#include "symboldetailsdialog.h"
#include "symbolimplvolwidget.h"
#include "symbolpricehistorywidget.h"

#include "db/appdb.h"
#include "db/symboldbs.h"

#include <QHBoxLayout>
#include <QTabWidget>

const QString SymbolDetailsDialog::STATE_GROUP_NAME( "symbolDetails" );

static const QString GEOMETRY( "geometry" );

///////////////////////////////////////////////////////////////////////////////////////////////////
SymbolDetailsDialog::SymbolDetailsDialog( const QString symbol, double price, QWidget *parent, Qt::WindowFlags f ) :
    _Mybase( parent, f ),
    symbol_( symbol ),
    price_( price )
{
    // remove the question mark button
    setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    // init
    initialize();
    createLayout();
    translate();

    // restore states
    restoreState( this );
    restoreState( splitter_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SymbolDetailsDialog::~SymbolDetailsDialog()
{
    // save states
    saveState( this );
    saveState( splitter_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSize SymbolDetailsDialog::sizeHint() const
{
    // default size
    return QSize( 1800, 900 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDetailsDialog::translate()
{
    const QString desc( SymbolDatabases::instance()->description( symbol() ) );

    QString title( symbol() + " " + tr( "Details" ) );

    if ( desc.length() )
        title += " - " + desc;

    setWindowTitle( title );

    tabs_->setTabText( 0, tr( "Details" ) );
    tabs_->setTabText( 1, tr( "Volatility" ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDetailsDialog::initialize()
{
    tabs_ = new QTabWidget( this );

    // details
    splitter_ = new CollapsibleSplitter( Qt::Horizontal );
    splitter_->setButtonLocation( Qt::TopEdge );
    splitter_->setHandleWidth( SPLITTER_WIDTH );
    splitter_->setObjectName( "underlying" );

    splitter_->addWidget( priceHistory_ = new SymbolPriceHistoryWidget( symbol(), splitter_ ) );
    splitter_->addWidget( fundamentals_ = new FundamentalsViewerWidget( symbol(), price_, splitter_ ) );

    tabs_->addTab( splitter_, QString() );

    // impl vol
    implVol_ = new SymbolImpliedVolatilityWidget( symbol(), price_ );

    tabs_->addTab( implVol_, QString() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDetailsDialog::createLayout()
{
    QHBoxLayout *form( new QHBoxLayout( this ) );
    form->setContentsMargins( QMargins() );
    form->addWidget( tabs_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDetailsDialog::saveState( QDialog *w ) const
{
    if ( w )
        AppDatabase::instance()->setWidgetState( AppDatabase::Dialog, STATE_GROUP_NAME, GEOMETRY, w->saveGeometry() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDetailsDialog::saveState( QSplitter *w ) const
{
    if ( w )
        AppDatabase::instance()->setWidgetState( AppDatabase::Splitter, STATE_GROUP_NAME, w->objectName(), w->saveState() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDetailsDialog::restoreState( QDialog *w ) const
{
    if ( w )
        w->restoreGeometry( AppDatabase::instance()->widgetState( AppDatabase::Dialog, STATE_GROUP_NAME, GEOMETRY ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDetailsDialog::restoreState( QSplitter *w ) const
{
    if ( w )
        splitter_->restoreState( AppDatabase::instance()->widgetState( AppDatabase::Splitter, STATE_GROUP_NAME, w->objectName() ) );
}
