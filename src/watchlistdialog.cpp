/**
 * @file watchlistdialog.cpp
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

#include "db/appdb.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStringList>
#include <QVBoxLayout>

///////////////////////////////////////////////////////////////////////////////////////////////////
WatchlistDialog::WatchlistDialog( QWidget *parent, Qt::WindowFlags f ) :
    _Mybase( parent, f ),
    db_( AppDatabase::instance() )
{
    // remove the question mark button
    setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    // init
    initialize();
    createLayout();
    translate();

    // retrieve watchlists
    foreach ( const QString& name, db_->watchlists() )
    {
        const QStringList symbols( db_->watchlist( name ) );

        // create item
        QListWidgetItem *item( new QListWidgetItem( name ) );
        item->setData( Qt::UserRole, symbols.join( "\n" ) );
        item->setFlags( item->flags() | Qt::ItemIsEditable ); // allow edit of item

        watchlist_->addItem( item );
    }

    // select first item
    selectItem( 0 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSize WatchlistDialog::sizeHint() const
{
    return QSize( 800, 600 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WatchlistDialog::translate()
{
    setWindowTitle( tr( "Watchlist Editor" ) );

    watchlistLabel_->setText( tr( "Watchlists:" ) );

    createList_->setText( tr( "New" ) );
    copyList_->setText( tr( "Copy" ) );
    renameList_->setText( tr( "Rename" ) );
    deleteList_->setText( tr( "Delete" ) );

    symbolsLabel_->setText( tr( "Symbols (one per line):" ) );

    okay_->setText( tr( "Okay" ) );
    cancel_->setText( tr( "Cancel" ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WatchlistDialog::onButtonClicked()
{
    // create new watchlist
    if ( createList_ == sender() )
    {
        const QString nameTemplate( "New Watchlist %1" );

        // generate unique watchlist name
        for ( int i( 1 ); ; ++i )
        {
            const QString name( nameTemplate.arg( i ) );

            const QList<QListWidgetItem*> items( watchlist_->findItems( name, Qt::MatchFixedString ) );

            if ( items.size() )
                continue;

            QListWidgetItem *item( new QListWidgetItem( name ) );
            item->setFlags( item->flags() | Qt::ItemIsEditable ); // allow edit of item

            watchlist_->addItem( item );

            // select item
            selectItem( watchlist_->count() );
            break;
        }
    }

    // copy watchlist
    else if ( copyList_ == sender() )
    {
        const QListWidgetItem *origItem( selectedItem() );

        if ( !origItem )
            return;

        const QString nameTemplate( "%1 (Copy %2)" );

        // generate unique name
        for ( int i( 1 ); ; ++i )
        {
            const QString name( nameTemplate.arg( origItem->text() ).arg( i ) );

            const QList<QListWidgetItem*> items( watchlist_->findItems( name, Qt::MatchFixedString ) );

            if ( items.size() )
                continue;

            QListWidgetItem *item( new QListWidgetItem( name ) );
            item->setData( Qt::UserRole, origItem->data( Qt::UserRole ) );
            item->setFlags( item->flags() | Qt::ItemIsEditable ); // allow edit of item

            watchlist_->addItem( item );
            break;
        }
    }

    // rename watchlist
    else if ( renameList_ == sender() )
    {
        QListWidgetItem *item( selectedItem() );

        if ( item )
            watchlist_->editItem( item );
    }

    // delete selected watchlist
    else if ( deleteList_ == sender() )
    {
        QListWidgetItem *item( selectedItem() );

        if ( item )
        {
            const int row( watchlist_->row( item ) );

            // remove
            watchlist_->takeItem( row );

            // select new item
            selectItem( row );

            delete item;
        }
    }

    // okay
    else if ( okay_ == sender() )
    {
        saveForm();
        accept();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WatchlistDialog::onItemSelectionChanged()
{
    const QListWidgetItem *item( selectedItem() );

    copyList_->setEnabled( item );
    renameList_->setEnabled( item );
    deleteList_->setEnabled( item );

    symbolsLabel_->setEnabled( item );
    symbols_->setEnabled( item );

    if ( !item )
    {
        symbols_->clear();
        return;
    }

    symbols_->setPlainText( item->data( Qt::UserRole ).toString() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WatchlistDialog::onTextChanged()
{
    QListWidgetItem *item( selectedItem() );

    if ( !item )
        return;

    item->setData( Qt::UserRole, symbols_->toPlainText() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WatchlistDialog::initialize()
{
    // watchlist label
    watchlistLabel_ = new QLabel( this );

    // watchlist
    watchlist_ = new QListWidget( this );
    watchlist_->setSelectionMode( QListWidget::SingleSelection );

    QListWidget::EditTriggers triggers( watchlist_->editTriggers() );
    triggers.setFlag( QListWidget::SelectedClicked, true );

    watchlist_->setEditTriggers( triggers );

    connect( watchlist_, &QListWidget::itemSelectionChanged, this, &_Myt::onItemSelectionChanged );

    // create list
    createList_ = new QPushButton( this );

    connect( createList_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    // copy list
    copyList_ = new QPushButton( this );

    connect( copyList_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    // rename list
    renameList_ = new QPushButton( this );

    connect( renameList_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    // delete list
    deleteList_ = new QPushButton( this );

    connect( deleteList_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    // symbols label
    symbolsLabel_ = new QLabel( this );

    // symbols
    symbols_ = new QPlainTextEdit( this );

    connect( symbols_, &QPlainTextEdit::textChanged, this, &_Myt::onTextChanged );

    // okay
    okay_ = new QPushButton( this );
    okay_->setDefault( true );

    connect( okay_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    // cancel
    cancel_ = new QPushButton( this );

    connect( cancel_, &QPushButton::clicked, this, &_Myt::reject );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WatchlistDialog::createLayout()
{
    QHBoxLayout *watchlistButtons( new QHBoxLayout );
    watchlistButtons->addWidget( createList_ );
    watchlistButtons->addWidget( copyList_ );
    watchlistButtons->addWidget( renameList_ );
    watchlistButtons->addWidget( deleteList_ );

    QVBoxLayout *watchlist( new QVBoxLayout );
    watchlist->addWidget( watchlistLabel_ );
    watchlist->addWidget( watchlist_, 1 );
    watchlist->addLayout( watchlistButtons );

    QVBoxLayout *symbols( new QVBoxLayout );
    symbols->addWidget( symbolsLabel_ );
    symbols->addWidget( symbols_, 1 );

    QHBoxLayout *top( new QHBoxLayout );
    top->addLayout( watchlist );
    top->addLayout( symbols );

    QHBoxLayout *buttons( new QHBoxLayout );
    buttons->addStretch();
    buttons->addWidget( cancel_ );
    buttons->addWidget( okay_ );

    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->addLayout( top, 1 );
    form->addLayout( buttons );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QListWidgetItem *WatchlistDialog::selectedItem() const
{
    const QList<QListWidgetItem*> items( watchlist_->selectedItems() );

    QListWidgetItem *item( nullptr );

    if ( 1 == items.size() )
        item = items.at( 0 );

    return item;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WatchlistDialog::selectItem( int index )
{
    if ( watchlist_->count() )
        watchlist_->setCurrentRow( qMin( index, watchlist_->count()-1 ) );

    onItemSelectionChanged();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WatchlistDialog::saveForm()
{
    const QStringList oldLists( db_->watchlists() );

    // retrieve new watchlist names
    QStringList newLists;

    for ( int i( watchlist_->count() ); i--; )
        newLists.append( watchlist_->item( i )->text() );

    // remove deleted lists from db
    foreach ( const QString& name, oldLists )
        if ( !newLists.contains( name ) )
            db_->removeWatchlist( name );

    // insert/update modified lists
    for ( int i( watchlist_->count() ); i--; )
    {
        const QListWidgetItem *item( watchlist_->item( i ) );
        const QString name( item->text() );

        // generate list of symbols from input
        const QStringList symbols( generateList( item->data( Qt::UserRole ).toString() ) );

        // skip save when exists already and not different
        if ( oldLists.contains( name ) )
            if ( symbols == db_->watchlist( name ) )
                continue;

        // save!
        db_->setWatchlist( name, symbols );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QStringList WatchlistDialog::generateList( const QString& data )
{
    QStringList result;

    // generate list
    foreach ( const QString& d, data.split( "\n" ) )
    {
        const QString str( d.trimmed() );

        if ( !str.isEmpty() )
            result.append( str );
    }

    // remove duplicates
    result.removeDuplicates();

    // sort by name
    result.sort();

    return result;
}
