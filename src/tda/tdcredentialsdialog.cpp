/**
 * @file tdcredentialsdialog.cpp
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
#include "tdcredentialsdialog.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

static const QString DEV_TDA_SITE( "https://developer.tdameritrade.com/user/me/apps" );

///////////////////////////////////////////////////////////////////////////////////////////////////
TDCredentialsDialog::TDCredentialsDialog( QWidget *parent, Qt::WindowFlags f ) :
    _Mybase( parent, f )
{
    // remove the question mark button
    setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    // init
    initialize();
    createLayout();
    translate();

    // validate
    validateForm();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString TDCredentialsDialog::callbackUrl() const
{
    return callbackUrl_->text();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString TDCredentialsDialog::consumerId() const
{
    return consumerId_->text();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDCredentialsDialog::setCallbackUrl( const QString& value )
{
    callbackUrl_->setText( value );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDCredentialsDialog::setConsumerId( const QString& value )
{
    consumerId_->setText( value );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSize TDCredentialsDialog::sizeHint() const
{
    return QSize( 350, 200 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDCredentialsDialog::translate()
{
    const QString href( "<a href=\"%1\">%1</a>" );

    setWindowTitle( tr( "TD Ameritrade Credentials" ) );

    consumerIdLabel_->setText( tr( "Consumer Id:" ) );
    consumerId_->setToolTip( tr( "Enter your consumer key from the TDA site followed by '@AMER.OAUTHAP'." ) );

    callbackUrlLabel_->setText( tr( "Callback URL:" ) );
    callbackUrl_->setToolTip( tr( "Enter your callback url from the TDA site." ) );

    tdaDeveloperInfo_->setText( tr( "For more information on TDA for Developers and setting up credentials please visit:" ) );
    tdaLink_->setText( href.arg( DEV_TDA_SITE ) );

    okay_->setText( tr( "Okay" ) );
    cancel_->setText( tr( "Cancel" ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool TDCredentialsDialog::validateForm()
{
    bool result( true );

    if ( consumerId_->text().isEmpty() )
        result = false;

    if ( callbackUrl_->text().isEmpty() )
        result = false;

    okay_->setEnabled( result );

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDCredentialsDialog::initialize()
{
    // consumer id
    consumerIdLabel_ = new QLabel( this );

    consumerId_ = new QLineEdit( this );

    connect( consumerId_, &QLineEdit::textChanged, this, &_Myt::validateForm );

    // callback url
    callbackUrlLabel_ = new QLabel( this );

    callbackUrl_ = new QLineEdit( this );

    connect( callbackUrl_, &QLineEdit::textChanged, this, &_Myt::validateForm );

    // tda information
    tdaDeveloperInfo_ = new QLabel( this );
    tdaDeveloperInfo_->setAlignment( Qt::AlignCenter );

    tdaLink_ = new QLabel( this );
    tdaLink_->setAlignment( Qt::AlignCenter );
    tdaLink_->setOpenExternalLinks( true );

    // okay
    okay_ = new QPushButton( this );
    okay_->setDefault( true );

    connect( okay_, &QPushButton::clicked, this, &_Myt::accept );

    // cancel
    cancel_ = new QPushButton( this );

    connect( cancel_, &QPushButton::clicked, this, &_Myt::reject );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDCredentialsDialog::createLayout()
{
    QFormLayout *creds( new QFormLayout );
    creds->addRow( consumerIdLabel_, consumerId_ );
    creds->addRow( callbackUrlLabel_, callbackUrl_ );

    QHBoxLayout *buttons( new QHBoxLayout() );
    buttons->addStretch();
    buttons->addWidget( cancel_ );
    buttons->addWidget( okay_ );

    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->addLayout( creds );
    form->addStretch();
    form->addWidget( tdaDeveloperInfo_ );
    form->addWidget( tdaLink_ );
    form->addStretch();
    form->addLayout( buttons );
}
