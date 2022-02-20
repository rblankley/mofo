/**
 * @file accountsdialog.cpp
 *
 * @copyright Copyright (C) 2022 Randy Blankley. All rights reserved.
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

#include "accountnicknamewidget.h"
#include "accountsdialog.h"

#include "db/appdb.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>

const QString AccountsDialog::STATE_GROUP_NAME( "accounts" );

static const QString GEOMETRY( "geometry" );

///////////////////////////////////////////////////////////////////////////////////////////////////
AccountsDialog::AccountsDialog( QWidget *parent ) :
    _Mybase( parent )
{
    // remove the question mark button
    setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    // init
    initialize();
    createLayout();
    translate();

    // populate accounts
    const QStringList accounts( AppDatabase::instance()->accounts() );

    foreach ( const QString& account, accounts )
    {
        const QStringList parts( account.split( ';' ) );

        if ( parts.size() < 4 )
            continue;

        AccountNicknameWidget *w( createAccountItem() );

        if ( !w )
            continue;

        w->setDefault( "1" == parts[3] );
        w->setAccountId( parts[0] );
        w->setType( parts[1] );
        w->setNickname( parts[2] );

        connect( w, &AccountNicknameWidget::defaultChanged, this, &_Myt::onDefaultChanged );
    }

    // restore states
    restoreState( this );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
AccountsDialog::~AccountsDialog()
{
    // save states
    saveState( this );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSize AccountsDialog::sizeHint() const
{
    return QSize( 800, 600 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AccountsDialog::translate()
{
    setWindowTitle( tr( "Accounts" ) );

    accountLabel_->setText( tr( "Account Id" ) );
    typeLabel_->setText( tr( "Type" ) );
    nicknameLabel_->setText( tr( "Nickname" ) );

    for ( AccountListMap::const_iterator i( w_.constBegin() ); i != w_.constEnd(); ++i )
        i.key()->translate();

    okay_->setText( tr( "Okay" ) );
    cancel_->setText( tr( "Cancel" ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AccountsDialog::onButtonClicked()
{
    if ( cancel_ == sender() )
        reject();
    else if ( okay_ == sender() )
    {
        QStringList nicknames;

        // create list of nicknames
        for ( AccountListMap::const_iterator i( w_.constBegin() ); i != w_.constEnd(); ++i )
        {
            const AccountNicknameWidget *w( i.key() );

            if ( !w )
                continue;

            nicknames.append( QString( "%1;%2;%3" ).arg(
                w->accountId(),
                w->nickname().replace( ';', ' ' ),
                w->isDefault() ? "1" : "0" ) );
        }

        // save
        AppDatabase::instance()->setAccountNicknames( nicknames );

        accept();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AccountsDialog::onDefaultChanged( bool newValue )
{
    if ( !newValue )
        return;

    // deselect other widgets
    for ( AccountListMap::const_iterator i( w_.constBegin() ); i != w_.constEnd(); ++i )
    {
        AccountNicknameWidget *w( i.key() );

        if ( !w )
            continue;
        else if ( w == sender() )
            continue;

        w->setDefault( false );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AccountsDialog::initialize()
{
    accountLabel_ = new QLabel( this );

    typeLabel_ = new QLabel( this );

    nicknameLabel_ = new QLabel( this );

    accounts_ = new QListWidget( this );

    okay_ = new QPushButton( this );

    connect( okay_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    cancel_ = new QPushButton( this );

    connect( cancel_, &QPushButton::clicked, this, &_Myt::onButtonClicked );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AccountsDialog::createLayout()
{
    QHBoxLayout *header( new QHBoxLayout() );
    header->setContentsMargins( QMargins() );
    header->addItem( new QSpacerItem( 24, 24 ) );
    header->addWidget( accountLabel_, 2 );
    header->addWidget( typeLabel_, 1 );
    header->addWidget( nicknameLabel_, 2 );

    QHBoxLayout *buttons( new QHBoxLayout() );
    buttons->setContentsMargins( QMargins() );
    buttons->addStretch();
    buttons->addWidget( cancel_ );
    buttons->addWidget( okay_ );

    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->addLayout( header );
    form->addWidget( accounts_, 1 );
    form->addLayout( buttons );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
AccountNicknameWidget *AccountsDialog::createAccountItem()
{
    // create widget
    AccountNicknameWidget *w( new AccountNicknameWidget( this ) );

    // create item
    QListWidgetItem *item( new QListWidgetItem( accounts_ ) );
    item->setFlags( Qt::NoItemFlags );
    item->setSizeHint( w->sizeHint() );

    // add
    accounts_->addItem( item );
    accounts_->setItemWidget( item, w );

    // track relationship
    w_[w] = item;

    return w;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AccountsDialog::saveState( QDialog *w ) const
{
    if ( w )
        AppDatabase::instance()->setWidgetState( AppDatabase::Dialog, STATE_GROUP_NAME, GEOMETRY, w->saveGeometry() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AccountsDialog::restoreState( QDialog *w ) const
{
    if ( w )
        w->restoreGeometry( AppDatabase::instance()->widgetState( AppDatabase::Dialog, STATE_GROUP_NAME, GEOMETRY ) );
}
