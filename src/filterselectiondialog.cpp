/**
 * @file filterselectiondialog.cpp
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
#include "filterselectiondialog.h"

#include "db/appdb.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

///////////////////////////////////////////////////////////////////////////////////////////////////
FilterSelectionDialog::FilterSelectionDialog( QWidget *parent, Qt::WindowFlags f ) :
    _Mybase( parent, f )
{
    // remove the question mark button
    setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    // init
    initialize();
    createLayout();
    translate();

    // populate filters
    filters_->addItem( tr( "NONE" ) );
    filters_->addItems( AppDatabase::instance()->filters() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool  FilterSelectionDialog::filtersExist() const
{
    return ( 1 < filters_->count() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString FilterSelectionDialog::selected() const
{
    // nothing selected
    if ( 0 == filters_->currentIndex() )
        return QString();

    return filters_->currentText();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSize FilterSelectionDialog::sizeHint() const
{
    return QSize( 250, 100 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilterSelectionDialog::translate()
{
    setWindowTitle( tr( "Choose Filter" ) );

    filtersLabel_->setText( tr( "Please select a filter:" ) );

    okay_->setText( tr( "Okay" ) );
    cancel_->setText( tr( "Cancel" ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilterSelectionDialog::onButtonClicked()
{
    // okay
    if ( okay_ == sender() )
    {
        accept();
    }

    // cancel
    else if ( cancel_ == sender() )
    {
        reject();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilterSelectionDialog::initialize()
{
    // filters
    filtersLabel_ = new QLabel( this );

    filters_ = new QComboBox( this );

    // okay
    okay_ = new QPushButton( this );

    connect( okay_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    // cancel
    cancel_ = new QPushButton( this );

    connect( cancel_, &QPushButton::clicked, this, &_Myt::onButtonClicked );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilterSelectionDialog::createLayout()
{
    QHBoxLayout *buttons( new QHBoxLayout() );
    buttons->addStretch();
    buttons->addWidget( cancel_ );
    buttons->addWidget( okay_ );

    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->addWidget( filtersLabel_ );
    form->addWidget( filters_ );
    form->addStretch();
    form->addLayout( buttons );
}
