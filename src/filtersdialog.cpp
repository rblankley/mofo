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
#include <QVBoxLayout>

///////////////////////////////////////////////////////////////////////////////////////////////////
FiltersDialog::FiltersDialog( QWidget *parent, Qt::WindowFlags f ) :
    _Mybase( parent, f )
{
    // remove the question mark button
    setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    // init
    initialize();
    createLayout();
    translate();

    // retrieve watchlists
    AppDatabase *db( AppDatabase::instance() );

    // populate!
    foreach ( const QString& name, db->filters() )
    {
        QListWidgetItem *item( new QListWidgetItem( name ) );
        item->setData( Qt::UserRole, name );

        // allow edit of item
        item->setFlags( item->flags() | Qt::ItemIsEditable );

        filters_->addItem( item );
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
    removeFilter_->setText( tr( "Delete" ) );

    okay_->setText( tr( "Okay" ) );
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

            if ( items.isEmpty() )
            {
                QListWidgetItem *item( new QListWidgetItem( name ) );

                // allow edit of item
                item->setFlags( item->flags() | Qt::ItemIsEditable );

                filters_->addItem( item );

                // create
                AppDatabase::instance()->setFilter( name );

                // select item
                selectItem( filters_->count() );
                break;
            }
        }
    }

    // edit selected filter
    else if ( editFilter_ == sender() )
    {
        FilterEditorDialog d( currentFilterName_, AppDatabase::instance()->filter( currentFilterName_ ), this );

        if ( QDialog::Accepted == d.exec() )
            AppDatabase::instance()->setFilter( currentFilterName_, d.filterValue() );
    }

    // remove selected filter
    else if ( removeFilter_ == sender() )
    {
        QListWidgetItem *item( selected() );

        if ( item )
        {
            const int row( filters_->row( item ) );

            // remove
            filters_->takeItem( row );

            AppDatabase::instance()->removeFilter( item->text() );

            // select new item
            selectItem( row );

            delete item;
        }
    }

    // okay
    else if ( okay_ == sender() )
    {
        accept();
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
            const QByteArray value( AppDatabase::instance()->filter( currentFilterName_ ) );

            AppDatabase::instance()->removeFilter( currentFilterName_ );
            AppDatabase::instance()->setFilter( name, value );

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

    FilterEditorDialog d( item->text(), AppDatabase::instance()->filter( item->text() ), this );

    if ( QDialog::Accepted == d.exec() )
        AppDatabase::instance()->setFilter( item->text(), d.filterValue() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FiltersDialog::onItemSelectionChanged()
{
    QListWidgetItem *item( selected() );

    editFilter_->setEnabled( item );
    removeFilter_->setEnabled( item );

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

    // remove filter
    removeFilter_ = new QPushButton( this );
    removeFilter_->setEnabled( false );

    connect( removeFilter_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    // okay
    okay_ = new QPushButton( this );

    connect( okay_, &QPushButton::clicked, this, &_Myt::onButtonClicked );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FiltersDialog::createLayout()
{
    QVBoxLayout *filters( new QVBoxLayout );
    filters->addWidget( filtersLabel_ );
    filters->addWidget( filters_, 1 );

    QHBoxLayout *buttons( new QHBoxLayout );
    buttons->addWidget( createFilter_ );
    buttons->addWidget( editFilter_ );
    buttons->addWidget( removeFilter_ );
    buttons->addStretch();
    buttons->addWidget( okay_ );

    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->addLayout( filters, 1 );
    form->addLayout( buttons );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QListWidgetItem *FiltersDialog::selected() const
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
