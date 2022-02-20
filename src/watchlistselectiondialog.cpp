/**
 * @file watchlistselectiondialog.cpp
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
#include "watchlistdialog.h"
#include "watchlistselectiondialog.h"

#include "db/appdb.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

const QString WatchlistSelectionDialog::STATE_GROUP_NAME( "watchlistSelection" );

static const QString GEOMETRY( "geometry" );

///////////////////////////////////////////////////////////////////////////////////////////////////
WatchlistSelectionDialog::WatchlistSelectionDialog( QWidget *parent, Qt::WindowFlags f ) :
    _Mybase( parent, f )
{
    // remove the question mark button
    setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    // init
    initialize();
    createLayout();
    translate();

    // generate boxes
    generateBoxes();

    // restore states
    restoreState( this );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
WatchlistSelectionDialog::~WatchlistSelectionDialog()
{
    // save states
    saveState( this );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool WatchlistSelectionDialog::watchlistsExist() const
{
    return ( 1 < boxes_.size() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString WatchlistSelectionDialog::selected() const
{
    QString result;

    // iterate through watchlist check boxes, build up string of selected watchlists
    for ( WatchlistCheckBoxMap::const_iterator i( boxes_.constBegin() ); i != boxes_.constEnd(); ++i )
        if ( i.value()->isChecked() )
        {
            if ( result.length() )
                result.append( "," );

            result.append( i.key() );
        }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WatchlistSelectionDialog::setSelected( const QString& value )
{
    const QStringList lists( value.split( "," ) );

    // iterate through watchlist check boxes
    for ( WatchlistCheckBoxMap::const_iterator i( boxes_.constBegin() ); i != boxes_.constEnd(); ++i )
    {
        bool found( false );

        // check box if found, otherwise uncheck
        foreach ( const QString& list, lists )
            if ( (found = (list == i.key())) )
                break;

        i.value()->setChecked( found );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSize WatchlistSelectionDialog::sizeHint() const
{
    return QSize( 350, 350 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WatchlistSelectionDialog::translate()
{
    setWindowTitle( tr( "Choose Watchlist(s)" ) );

    watchListsLabel_->setText( tr( "Select one or more watchlists:" ) );

    editWatchLists_->setText( tr( "Edit Watchlists" ) );

    okay_->setText( tr( "Okay" ) );
    cancel_->setText( tr( "Cancel" ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WatchlistSelectionDialog::onButtonClicked()
{
    if ( editWatchLists_ == sender() )
    {
        // save off existing selection
        const QString s( selected() );

        // edit
        WatchlistDialog d( this );

        if ( QDialog::Accepted == d.exec() )
        {
            // regenerate boxes and restore selection
            generateBoxes();
            setSelected( s );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WatchlistSelectionDialog::initialize()
{
    // watchlists
    watchListsLabel_ = new QLabel( this );

    // edit watchlists
    editWatchLists_ = new QPushButton( this );

    connect( editWatchLists_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    // okay
    okay_ = new QPushButton( this );
    okay_->setDefault( true );

    connect( okay_, &QPushButton::clicked, this, &_Myt::accept );

    // cancel
    cancel_ = new QPushButton( this );

    connect( cancel_, &QPushButton::clicked, this, &_Myt::reject );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WatchlistSelectionDialog::createLayout()
{
    QHBoxLayout *buttons( new QHBoxLayout() );
    buttons->addWidget( editWatchLists_ );
    buttons->addStretch();
    buttons->addWidget( cancel_ );
    buttons->addWidget( okay_ );

    // create layout for boxes, will be populated later
    boxesLayout_ = new QVBoxLayout();

    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->addWidget( watchListsLabel_ );
    form->addLayout( boxesLayout_ );
    form->addStretch();
    form->addLayout( buttons );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WatchlistSelectionDialog::generateBoxes()
{
    // remove all existing boxes
    for ( WatchlistCheckBoxMap::const_iterator i( boxes_.constBegin() ); i != boxes_.constEnd(); ++i )
        i.value()->deleteLater();

    boxes_.clear();

    // generate new boxes
    QStringList lists( AppDatabase::instance()->watchlists() );

    foreach ( const QString& list, lists )
    {
        QString text( list );
        text.replace( "&", "&&" );

        QCheckBox *box = new QCheckBox( this );
        box->setText( text );
        box->setCheckable( true );

        boxes_[list] = box;

        // add to layout
        boxesLayout_->addWidget( box );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WatchlistSelectionDialog::saveState( QDialog *w ) const
{
    if ( w )
        AppDatabase::instance()->setWidgetState( AppDatabase::Dialog, STATE_GROUP_NAME, GEOMETRY, w->saveGeometry() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WatchlistSelectionDialog::restoreState( QDialog *w ) const
{
    if ( w )
        w->restoreGeometry( AppDatabase::instance()->widgetState( AppDatabase::Dialog, STATE_GROUP_NAME, GEOMETRY ) );
}
