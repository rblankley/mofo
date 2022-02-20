/**
 * @file accountnicknamewidget.h
 * Widget for editing account nickname.
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

#ifndef ACCOUNTNICKNAMEWIDGET_H
#define ACCOUNTNICKNAMEWIDGET_H

#include <QWidget>

class QCheckBox;
class QLabel;
class QLineEdit;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Widget for editing account nickname.
class AccountNicknameWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString accountId READ accountId WRITE setAccountId )
    Q_PROPERTY( bool default READ isDefault WRITE setDefault NOTIFY defaultChanged )
    Q_PROPERTY( QString nickname READ nickname WRITE setNickname )
    Q_PROPERTY( QString type READ type WRITE setType )

    using _Myt = AccountNicknameWidget;
    using _Mybase = QWidget;

signals:

    /// Signal for default changed.
    /**
     * @param[in] newValue  new value
     */
    void defaultChanged( bool newValue );

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] parent  parent object
     */
    AccountNicknameWidget( QWidget *parent = nullptr );

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve account id.
    /**
     * @return  account id
     */
    virtual QString accountId() const;

    /// Retrieve if account is default
    /**
     * @return  @c true if default, @c false otherwise
     */
    virtual bool isDefault() const;

    /// Retrieve nickname
    /**
     * @return  nickname
     */
    virtual QString nickname() const;

    /// Set account id.
    /**
     * @param[in] value  account id
     */
    virtual void setAccountId( const QString& value );

    /// Set default account.
    /**
     * @param[in] value  @c true for default account, @c false otherwise
     */
    virtual void setDefault( bool value );

    /// Set nickname.
    /**
     * @param[in] value  nickname
     */
    virtual void setNickname( const QString& value );

    /// Set type.
    /**
     * @param[in] value  type
     */
    virtual void setType( const QString& value );

    /// Retrieve type.
    /**
     * @return  type
     */
    virtual QString type() const;

    // ========================================================================
    // Methods
    // ========================================================================

    /// Translate strings.
    virtual void translate();

private:

    QCheckBox *default_;

    QLabel *accountId_;
    QLabel *type_;

    QLineEdit *nickname_;

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    // not implemented
    AccountNicknameWidget( const _Myt& other ) = delete;

    // not implemented
    AccountNicknameWidget( const _Myt&& other ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt& rhs ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt&& rhs ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // ACCOUNTNICKNAMEWIDGET_H
