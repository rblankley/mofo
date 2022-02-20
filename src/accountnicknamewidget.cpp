/**
 * @file accountnicknamewidget.cpp
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

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

///////////////////////////////////////////////////////////////////////////////////////////////////
AccountNicknameWidget::AccountNicknameWidget( QWidget *parent ) :
    _Mybase( parent )
{
    // init
    initialize();
    createLayout();
    translate();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString AccountNicknameWidget::accountId() const
{
    return accountId_->text();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AccountNicknameWidget::isDefault() const
{
    return default_->isChecked();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString AccountNicknameWidget::nickname() const
{
    return nickname_->text();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AccountNicknameWidget::setAccountId( const QString& value )
{
    accountId_->setText( value );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AccountNicknameWidget::setDefault( bool value )
{
    default_->setChecked( value );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AccountNicknameWidget::setNickname( const QString& value )
{
    nickname_->setText( value );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AccountNicknameWidget::setType( const QString& value )
{
    type_->setText( value );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString AccountNicknameWidget::type() const
{
    return type_->text();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AccountNicknameWidget::translate()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AccountNicknameWidget::initialize()
{
    default_ = new QCheckBox( this );

    connect( default_, &QCheckBox::toggled, this, &_Myt::defaultChanged );

    accountId_ = new QLabel( this );

    type_ = new QLabel( this );

    nickname_ = new QLineEdit( this );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AccountNicknameWidget::createLayout()
{
    QHBoxLayout *form( new QHBoxLayout( this ) );
    form->setContentsMargins( QMargins() );
    form->addWidget( default_ );
    form->addWidget( accountId_, 2 );
    form->addWidget( type_, 1 );
    form->addWidget( nickname_, 2 );
}
