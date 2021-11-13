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
#include "filtersdialog.h"
#include "filterselectiondialog.h"
#include "watchlistselectiondialog.h"

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
    watchListsDialog_->setVisible( watchListsVisible_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSize FilterSelectionDialog::sizeHint() const
{
    return QSize( 350, watchListsVisible_ ? 200 : 150 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilterSelectionDialog::translate()
{
    setWindowTitle( tr( "Choose Filter" ) );

    watchListsLabel_->setText( tr( "Enter watchlists (comma separated):" ) );
    watchListsDialog_->setText( "..." );

    filtersLabel_->setText( tr( "Select a filter:" ) );
    filtersDialog_->setText( "..." );

    okay_->setText( tr( "Okay" ) );
    cancel_->setText( tr( "Cancel" ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilterSelectionDialog::onButtonClicked()
{
    // watchlists
    if ( watchListsDialog_ == sender() )
    {
        WatchlistSelectionDialog d( this );
        d.setSelected( watchLists_->text() );

        if ( QDialog::Accepted == d.exec() )
            watchLists_->setText( d.selected() );
    }

    // filters
    else if ( filtersDialog_ == sender() )
    {
        // save off existing selection
        const QString s( filters_->currentData().toString() );

        // edit
        FiltersDialog d( this );
        d.setSelected( s );
        d.setCancelButtonVisible( true );

        // prompt
        const int rc( d.exec() );

        // remove existing filters and add new ones
        while ( 1 < filters_->count() )
            filters_->removeItem( filters_->count() - 1 );

        foreach ( const QString& f, AppDatabase::instance()->filters() )
            filters_->addItem( f, f );

        // set back to existing selection; or the selected filter if they accepted the dialog
        int i;

        if ( 0 <= (i = filters_->findData( (QDialog::Accepted == rc) ? d.selected() : s )) )
            filters_->setCurrentIndex( i );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilterSelectionDialog::initialize()
{
    // watchlists
    watchListsLabel_ = new QLabel( this );
    watchListsLabel_->setVisible( watchListsVisible_ );

    watchLists_ = new QLineEdit( this );
    watchLists_->setVisible( watchListsVisible_ );

    watchListsDialog_ = new QPushButton( this );
    watchListsDialog_->setVisible( watchListsVisible_ );

    connect( watchListsDialog_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    // filters
    filtersLabel_ = new QLabel( this );

    filters_ = new QComboBox( this );

    filtersDialog_ = new QPushButton( this );

    connect( filtersDialog_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    // okay
    okay_ = new QPushButton( this );
    okay_->setDefault( true );

    connect( okay_, &QPushButton::clicked, this, &_Myt::accept );

    // cancel
    cancel_ = new QPushButton( this );

    connect( cancel_, &QPushButton::clicked, this, &_Myt::reject );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilterSelectionDialog::createLayout()
{
    QHBoxLayout *watchLists( new QHBoxLayout );
    watchLists->setContentsMargins( QMargins() );
    watchLists->addWidget( watchLists_, 1 );
    watchLists->addWidget( watchListsDialog_ );

    QHBoxLayout *filters( new QHBoxLayout );
    filters->setContentsMargins( QMargins() );
    filters->addWidget( filters_, 1 );
    filters->addWidget( filtersDialog_ );

    QHBoxLayout *buttons( new QHBoxLayout() );
    buttons->addStretch();
    buttons->addWidget( cancel_ );
    buttons->addWidget( okay_ );

    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->addWidget( watchListsLabel_ );
    form->addLayout( watchLists );
    form->addWidget( filtersLabel_ );
    form->addLayout( filters );
    form->addStretch();
    form->addLayout( buttons );
}
