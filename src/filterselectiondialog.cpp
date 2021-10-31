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
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

///////////////////////////////////////////////////////////////////////////////////////////////////
FilterSelectionDialog::FilterSelectionDialog( QWidget *parent, Qt::WindowFlags f ) :
    _Mybase( parent, f ),
    watchListsVisible_( false )
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
bool FilterSelectionDialog::isWatchListsVisible() const
{
    return watchListsVisible_;
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
QString FilterSelectionDialog::watchLists() const
{
    return watchLists_->text();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilterSelectionDialog::setDefaultFilter( const QString& value )
{
    const int i( filters_->findText( value ) );

    if ( 0 < i )
        filters_->setCurrentIndex( i );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilterSelectionDialog::setDefaultWatchLists( const QString& value )
{
    watchLists_->setText( value );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilterSelectionDialog::setWatchListsVisible( bool value )
{
    watchListsVisible_ = value;

    watchListsLabel_->setVisible( watchListsVisible_ );
    watchLists_->setVisible( watchListsVisible_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSize FilterSelectionDialog::sizeHint() const
{
    return QSize( 250, watchListsVisible_ ? 150 : 100 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilterSelectionDialog::translate()
{
    setWindowTitle( tr( "Choose Filter" ) );

    watchListsLabel_->setText( tr( "Enter watchlists (comma separated):" ) );

    filtersLabel_->setText( tr( "Select a filter:" ) );

    okay_->setText( tr( "Okay" ) );
    cancel_->setText( tr( "Cancel" ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilterSelectionDialog::initialize()
{
    // watchlists
    watchListsLabel_ = new QLabel( this );
    watchListsLabel_->setVisible( watchListsVisible_ );

    watchLists_ = new QLineEdit( this );
    watchLists_->setVisible( watchListsVisible_ );

    // filters
    filtersLabel_ = new QLabel( this );

    filters_ = new QComboBox( this );

    // okay
    okay_ = new QPushButton( this );

    connect( okay_, &QPushButton::clicked, this, &_Myt::accept );

    // cancel
    cancel_ = new QPushButton( this );

    connect( cancel_, &QPushButton::clicked, this, &_Myt::reject );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilterSelectionDialog::createLayout()
{
    QHBoxLayout *buttons( new QHBoxLayout() );
    buttons->addStretch();
    buttons->addWidget( cancel_ );
    buttons->addWidget( okay_ );

    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->addWidget( watchListsLabel_ );
    form->addWidget( watchLists_ );
    form->addWidget( filtersLabel_ );
    form->addWidget( filters_ );
    form->addStretch();
    form->addLayout( buttons );
}
