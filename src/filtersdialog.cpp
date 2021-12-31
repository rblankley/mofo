/**
 * @file filtersdialog.cpp
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
#include "filtereditordialog.h"
#include "filtersdialog.h"

#include "db/appdb.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

///////////////////////////////////////////////////////////////////////////////////////////////////
FiltersDialog::FiltersDialog( QWidget *parent, Qt::WindowFlags f ) :
    _Mybase( parent, f ),
    db_( AppDatabase::instance() ),
    cancelVisible_( false )
{
    // remove the question mark button
    setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    // init
    initialize();
    createLayout();
    translate();

    // populate!
    foreach ( const QString& name, db_->filters() )
    {
        QListWidgetItem *item( new QListWidgetItem( name ) );
        item->setData( Qt::UserRole, name );
        item->setFlags( item->flags() | Qt::ItemIsEditable ); // allow edit of item

        filters_->addItem( item );
    }

    // select first item
    selectItem( 0 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString FiltersDialog::selected() const
{
    QListWidgetItem *item( selectedItem() );

    if ( item )
        return item->text();

    return QString();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FiltersDialog::setCancelButtonVisible( bool value )
{
    cancel_->setVisible( (cancelVisible_ = value) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FiltersDialog::setSelected( const QString& value )
{
    // find and select passed in item
    for ( int i( filters_->count() ); i--; )
        if ( value == filters_->item( i )->text() )
        {
            selectItem( i );
            break;
        }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSize FiltersDialog::sizeHint() const
{
    return QSize( 500, 600 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FiltersDialog::translate()
{
    setWindowTitle( tr( "Filters" ) );

    filtersLabel_->setText( tr( "Filters:" ) );

    createFilter_->setText( tr( "New" ) );
    editFilter_->setText( tr( "Edit" ) );
    copyFilter_->setText( tr( "Copy" ) );
    renameFilter_->setText( tr( "Rename" ) );
    deleteFilter_->setText( tr( "Delete" ) );

    okay_->setText( tr( "Okay" ) );
    cancel_->setText( tr( "Cancel" ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FiltersDialog::closePersistentEditor()
{
    QListWidgetItem *item( selectedItem() );

    if (( item ) && ( filters_->isPersistentEditorOpen( item ) ))
        filters_->closePersistentEditor( item );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FiltersDialog::onButtonClicked()
{
    // create new filter
    if ( createFilter_ == sender() )
    {
        const QString nameTemplate( "New Filter %1" );

        // generate unique name
        for ( int i( 1 ); ; ++i )
        {
            const QString name( nameTemplate.arg( i ) );

            const QList<QListWidgetItem*> items( filters_->findItems( name, Qt::MatchFixedString ) );

            if ( items.size() )
                continue;

            QListWidgetItem *item( new QListWidgetItem( name ) );
            item->setFlags( item->flags() | Qt::ItemIsEditable ); // allow edit of item

            filters_->addItem( item );

            // create
            db_->setFilter( name );

            // select item
            selectItem( filters_->count() );
            break;
        }
    }

    // edit selected filter
    else if ( editFilter_ == sender() )
    {
        FilterEditorDialog d( currentFilterName_, db_->filter( currentFilterName_ ), this );

        if ( QDialog::Accepted == d.exec() )
            db_->setFilter( currentFilterName_, d.filterValue() );
    }

    // copy selected filter
    else if ( copyFilter_ == sender() )
    {
        QListWidgetItem *origItem( selectedItem() );

        if ( !origItem )
            return;

        const QString nameTemplate( "%1 (Copy %2)" );

        // generate unique name
        for ( int i( 1 ); ; ++i )
        {
            const QString name( nameTemplate.arg( origItem->text() ).arg( i ) );

            const QList<QListWidgetItem*> items( filters_->findItems( name, Qt::MatchFixedString ) );

            if ( items.size() )
                continue;

            QListWidgetItem *item( new QListWidgetItem( name ) );
            item->setFlags( item->flags() | Qt::ItemIsEditable ); // allow edit of item

            filters_->addItem( item );

            // create
            const QByteArray f( db_->filter( origItem->text() ) );

            db_->setFilter( name, f );
            break;
        }
    }

    // rename selected filter
    else if ( renameFilter_ == sender() )
    {
        QListWidgetItem *item( selectedItem() );

        if ( item )
            filters_->editItem( item );
    }

    // delete selected filter
    else if ( deleteFilter_ == sender() )
    {
        QListWidgetItem *item( selectedItem() );

        if ( item )
        {
            const int row( filters_->row( item ) );

            // remove
            filters_->takeItem( row );

            db_->removeFilter( item->text() );

            // select new item
            selectItem( row );

            delete item;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FiltersDialog::onItemChanged( QListWidgetItem *item )
{
    if ( !item )
        return;

    // update filter name
    const QString name( item->text() );

    if (( currentFilterName_.length() ) && ( name.length() ) && ( currentFilterName_ != name ))
    {
        // check this name does not exist already
        const QList<QListWidgetItem*> items( filters_->findItems( name, Qt::MatchExactly ) );

        if ( 1 == items.size() )
        {
            const QByteArray value( db_->filter( currentFilterName_ ) );

            db_->removeFilter( currentFilterName_ );
            db_->setFilter( name, value );

            currentFilterName_ = name;

            return;
        }
    }

    // revert filter name
    item->setText( currentFilterName_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FiltersDialog::onItemDoubleClicked( QListWidgetItem *item )
{
    if ( !item )
        return;

    FilterEditorDialog d( item->text(), db_->filter( item->text() ), this );

    if ( QDialog::Accepted == d.exec() )
        db_->setFilter( item->text(), d.filterValue() );

    // force close any editor
    closeEditorTimer_->start( 0 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FiltersDialog::onItemSelectionChanged()
{
    QListWidgetItem *item( selectedItem() );

    editFilter_->setEnabled( item );
    copyFilter_->setEnabled( item );
    renameFilter_->setEnabled( item );
    deleteFilter_->setEnabled( item );

    currentFilterName_ = (item ? item->text() : QString());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FiltersDialog::initialize()
{
    // filters label
    filtersLabel_ = new QLabel( this );

    // filters
    filters_ = new QListWidget( this );
    filters_->setSelectionMode( QListWidget::SingleSelection );

    QListWidget::EditTriggers triggers( filters_->editTriggers() );
    triggers.setFlag( QListWidget::SelectedClicked, true );

    filters_->setEditTriggers( triggers );

    connect( filters_, &QListWidget::itemChanged, this, &_Myt::onItemChanged );
    connect( filters_, &QListWidget::itemDoubleClicked, this, &_Myt::onItemDoubleClicked );
    connect( filters_, &QListWidget::itemSelectionChanged, this, &_Myt::onItemSelectionChanged );

    // create filter
    createFilter_ = new QPushButton( this );

    connect( createFilter_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    // edit filter
    editFilter_ = new QPushButton( this );
    editFilter_->setEnabled( false );

    connect( editFilter_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    // copy filter
    copyFilter_ = new QPushButton( this );
    copyFilter_->setEnabled( false );

    connect( copyFilter_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    // rename filter
    renameFilter_ = new QPushButton( this );
    renameFilter_->setEnabled( false );

    connect( renameFilter_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    // delete filter
    deleteFilter_ = new QPushButton( this );
    deleteFilter_->setEnabled( false );

    connect( deleteFilter_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    // okay
    okay_ = new QPushButton( this );
    okay_->setDefault( true );

    connect( okay_, &QPushButton::clicked, this, &_Myt::accept );

    // cancel
    cancel_ = new QPushButton( this );
    cancel_->setVisible( cancelVisible_ );

    connect( cancel_, &QPushButton::clicked, this, &_Myt::reject );

    // close editor timer
    closeEditorTimer_ = new QTimer( this );
    closeEditorTimer_->setSingleShot( true );

    connect( closeEditorTimer_, &QTimer::timeout, this, &_Myt::closePersistentEditor );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FiltersDialog::createLayout()
{
    QVBoxLayout *filters( new QVBoxLayout );
    filters->addWidget( filtersLabel_ );
    filters->addWidget( filters_, 1 );

    QHBoxLayout *filterButtons( new QHBoxLayout );
    filterButtons->addWidget( createFilter_ );
    filterButtons->addWidget( editFilter_ );
    filterButtons->addWidget( copyFilter_ );
    filterButtons->addWidget( renameFilter_ );
    filterButtons->addWidget( deleteFilter_ );

    QHBoxLayout *buttons( new QHBoxLayout );
    buttons->addStretch();
    buttons->addWidget( cancel_ );
    buttons->addWidget( okay_ );

    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->addLayout( filters, 1 );
    form->addLayout( filterButtons );
    form->addItem( new QSpacerItem( 16, 16 ) );
    form->addLayout( buttons );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QListWidgetItem *FiltersDialog::selectedItem() const
{
    const QList<QListWidgetItem*> items( filters_->selectedItems() );

    QListWidgetItem *item( nullptr );

    if ( 1 == items.size() )
        item = items.at( 0 );

    return item;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FiltersDialog::selectItem( int index )
{
    if ( filters_->count() )
        filters_->setCurrentRow( qMin( index, filters_->count()-1 ) );

    onItemSelectionChanged();
}
