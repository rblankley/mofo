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

#include "common.h"
#include "symboldetailsdialog.h"
#include "symbolpricehistorywidget.h"

#include <QVBoxLayout>

///////////////////////////////////////////////////////////////////////////////////////////////////
SymbolDetailsDialog::SymbolDetailsDialog( const QString symbol, QWidget *parent, Qt::WindowFlags f ) :
    _Mybase( parent, f ),
    symbol_( symbol )
{
    // remove the question mark button
    setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    // init
    initialize();
    createLayout();
    translate();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSize SymbolDetailsDialog::sizeHint() const
{
    return QSize( 1800, 900 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDetailsDialog::translate()
{
    setWindowTitle( symbol() + " " + tr( "Details" ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDetailsDialog::initialize()
{
    priceHistory_ = new SymbolPriceHistoryWidget( symbol(), this );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolDetailsDialog::createLayout()
{
    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->addWidget( priceHistory_ );
}
