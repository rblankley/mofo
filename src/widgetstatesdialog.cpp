/**
 * @file widgetstatesdialog.cpp
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
#include "widgetstatesdialog.h"

#include "db/appdb.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QStringList>
#include <QVBoxLayout>

const QString WidgetStatesDialog::STATE_GROUP_NAME( "widgetStates" );

static const QString GEOMETRY( "geometry" );

///////////////////////////////////////////////////////////////////////////////////////////////////
WidgetStatesDialog::WidgetStatesDialog( QWidget *parent, Qt::WindowFlags f ) :
    _Mybase( parent, f ),
    db_( AppDatabase::instance() )
{
    // remove the question mark button
    setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    // init
    initialize();
    createLayout();
    translate();

    // map of group name to display name
    QMap<QString, QString> groupNames;
    groupNames["optionChainView"] = tr( "Option Chains" );
    groupNames["optionTradingView"] = tr( "Option Analysis Results" );

    // populate group names combo
    foreach ( const QString& name, db_->widgetGroupNames( AppDatabase::HeaderView ) )
    {
        QString displayName( "_" + name );

        if ( groupNames.contains( name ) )
            displayName = groupNames[name];

        groupName_->addItem( displayName, name );
    }

    // select first item
    onCurrentIndexChanged( 0 );

    // restore states
    restoreState( this );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
WidgetStatesDialog::~WidgetStatesDialog()
{
    // save existing items
    saveForm();

    // save states
    saveState( this );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSize WidgetStatesDialog::sizeHint() const
{
    return QSize( 800, 600 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WidgetStatesDialog::translate()
{
    setWindowTitle( tr( "Layout Editor" ) );

    groupNameLabel_->setText( tr( "Select a View:" ) );

    statesLabel_->setText( tr( "Layouts:" ) );

    copyState_->setText( tr( "Copy" ) );
    renameState_->setText( tr( "Rename" ) );
    deleteState_->setText( tr( "Delete" ) );

    okay_->setText( tr( "Okay" ) );
    cancel_->setText( tr( "Cancel" ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WidgetStatesDialog::onButtonClicked()
{
    // copy state
    if ( copyState_ == sender() )
    {
        const QListWidgetItem *origItem( selectedItem() );

        if ( !origItem )
            return;

        const QString nameTemplate( "%1 (Copy %2)" );

        // generate unique name
        for ( int i( 1 ); ; ++i )
        {
            const QString name( nameTemplate.arg( origItem->text() ).arg( i ) );

            const QList<QListWidgetItem*> items( states_->findItems( name, Qt::MatchFixedString ) );

            if ( items.size() )
                continue;

            QListWidgetItem *item( new QListWidgetItem( name ) );
            item->setData( Qt::UserRole, origItem->data( Qt::UserRole ) );
            item->setFlags( item->flags() | Qt::ItemIsEditable ); // allow edit of item

            states_->addItem( item );
            break;
        }
    }

    // rename state
    else if ( renameState_ == sender() )
    {
        QListWidgetItem *item( selectedItem() );

        if ( item )
            states_->editItem( item );
    }

    // delete selected state
    else if ( deleteState_ == sender() )
    {
        QListWidgetItem *item( selectedItem() );

        if ( item )
        {
            const int row( states_->row( item ) );

            // remove
            states_->takeItem( row );

            // select new item
            selectItem( row );

            delete item;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WidgetStatesDialog::onCurrentIndexChanged( int index )
{
    Q_UNUSED( index )

    // save existing items
    saveForm();

    // clear out existing items
    states_->clear();

    // populate with new items
    currentGroupName_ = groupName_->currentData().toString();

    foreach ( const QString& name, db_->widgetStates( AppDatabase::HeaderView, currentGroupName_ ) )
    {
        // create item
        QListWidgetItem *item( new QListWidgetItem( name ) );
        item->setData( Qt::UserRole, db_->widgetState( AppDatabase::HeaderView, currentGroupName_, name ) );
        item->setFlags( item->flags() | Qt::ItemIsEditable ); // allow edit of item

        states_->addItem( item );
    }

    // select first item
    selectItem( 0 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WidgetStatesDialog::onItemSelectionChanged()
{
    const QListWidgetItem *item( selectedItem() );

    copyState_->setEnabled( item );
    renameState_->setEnabled( item );
    deleteState_->setEnabled( item );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WidgetStatesDialog::initialize()
{
    // group name label
    groupNameLabel_ = new QLabel( this );

    // group name
    groupName_ = new QComboBox( this );

    connect( groupName_, static_cast<void(QComboBox::*)(int)>( &QComboBox::currentIndexChanged ), this, &_Myt::onCurrentIndexChanged );

    // states label
    statesLabel_ = new QLabel( this );

    // states
    states_ = new QListWidget( this );
    states_->setSelectionMode( QListWidget::SingleSelection );

    QListWidget::EditTriggers triggers( states_->editTriggers() );
    triggers.setFlag( QListWidget::SelectedClicked, true );

    states_->setEditTriggers( triggers );

    connect( states_, &QListWidget::itemSelectionChanged, this, &_Myt::onItemSelectionChanged );

    // copy state
    copyState_ = new QPushButton( this );

    connect( copyState_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    // rename state
    renameState_ = new QPushButton( this );

    connect( renameState_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    // delete state
    deleteState_ = new QPushButton( this );

    connect( deleteState_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    // okay
    okay_ = new QPushButton( this );
    okay_->setDefault( true );

    connect( okay_, &QPushButton::clicked, this, &_Myt::accept );

    // cancel
    cancel_ = new QPushButton( this );
    cancel_->setVisible( false );

    connect( cancel_, &QPushButton::clicked, this, &_Myt::reject );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WidgetStatesDialog::createLayout()
{
    QHBoxLayout *stateButtons( new QHBoxLayout );
    stateButtons->addWidget( copyState_ );
    stateButtons->addWidget( renameState_ );
    stateButtons->addWidget( deleteState_ );

    QHBoxLayout *buttons( new QHBoxLayout );
    buttons->addStretch();
    buttons->addWidget( cancel_ );
    buttons->addWidget( okay_ );

    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->addWidget( groupNameLabel_ );
    form->addWidget( groupName_ );
    form->addWidget( statesLabel_ );
    form->addWidget( states_, 1 );
    form->addLayout( stateButtons );
    form->addLayout( buttons );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QListWidgetItem *WidgetStatesDialog::selectedItem() const
{
    const QList<QListWidgetItem*> items( states_->selectedItems() );

    QListWidgetItem *item( nullptr );

    if ( 1 == items.size() )
        item = items.at( 0 );

    return item;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WidgetStatesDialog::selectItem( int index )
{
    if ( states_->count() )
        states_->setCurrentRow( qMin( index, states_->count()-1 ) );

    onItemSelectionChanged();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WidgetStatesDialog::saveForm()
{
    // nothing to do
    if ( currentGroupName_.isEmpty() )
        return;

    const QStringList oldStates( db_->widgetStates( AppDatabase::HeaderView, currentGroupName_ ) );

    // retrieve new states
    QStringList newStates;

    for ( int i( states_->count() ); i--; )
        newStates.append( states_->item( i )->text() );

    // remove deleted states from db
    foreach ( const QString& name, oldStates )
        if ( !newStates.contains( name ) )
            db_->removeWidgetState( AppDatabase::HeaderView, currentGroupName_, name );

    // insert/update modified states
    for ( int i( states_->count() ); i--; )
    {
        const QListWidgetItem *item( states_->item( i ) );
        const QString name( item->text() );

        // skip save when exists already
        if ( oldStates.contains( name ) )
            continue;

        // save!
        db_->setWidgetState( AppDatabase::HeaderView, currentGroupName_, name, item->data( Qt::UserRole ).toByteArray() );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WidgetStatesDialog::saveState( QDialog *w ) const
{
    if ( w )
        AppDatabase::instance()->setWidgetState( AppDatabase::Dialog, STATE_GROUP_NAME, GEOMETRY, w->saveGeometry() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WidgetStatesDialog::restoreState( QDialog *w ) const
{
    if ( w )
        w->restoreGeometry( AppDatabase::instance()->widgetState( AppDatabase::Dialog, STATE_GROUP_NAME, GEOMETRY ) );
}
