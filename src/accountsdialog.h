/**
 * @file accountsdialog.h
 * Accounts dialog.
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

#ifndef ACCOUNTSDIALOG_H
#define ACCOUNTSDIALOG_H

#include <QDialog>
#include <QMap>

class AccountNicknameWidget;

class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Accounts dialog.
class AccountsDialog : public QDialog
{
    Q_OBJECT

    using _Myt = AccountsDialog;
    using _Mybase = QDialog;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] parent  parent object
     */
    AccountsDialog( QWidget *parent = nullptr );

    /// Destructor.
    virtual ~AccountsDialog();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve size hint.
    /**
     * @return  size hint
     */
    virtual QSize sizeHint() const override;

    // ========================================================================
    // Methods
    // ========================================================================

    /// Translate strings.
    virtual void translate();

private slots:

    /// Slot for button clicked.
    void onButtonClicked();

    /// Slot for default changed.
    void onDefaultChanged( bool newValue );

private:

    static const QString STATE_GROUP_NAME;

    using AccountListMap = QMap<AccountNicknameWidget*, QListWidgetItem*>;

    AccountListMap w_;

    QLabel *accountLabel_;
    QLabel *typeLabel_;
    QLabel *nicknameLabel_;

    QListWidget *accounts_;

    QPushButton *okay_;
    QPushButton *cancel_;

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    /// Create account item.
    AccountNicknameWidget *createAccountItem();

    /// Save dialog state.
    void saveState( QDialog *w ) const;

    /// Restore dialog state.
    void restoreState( QDialog *w ) const;

    // not implemented
    AccountsDialog( const _Myt& other ) = delete;

    // not implemented
    AccountsDialog( const _Myt&& other ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt& rhs ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt&& rhs ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // ACCOUNTSDIALOG_H
