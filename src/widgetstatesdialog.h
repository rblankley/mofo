/**
 * @file widgetstatesdialog.h
 * Dialog for editing widget states (layouts).
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

#ifndef WIDGETSTATESDIALOG_H
#define WIDGETSTATESDIALOG_H

#include <QDialog>

class AppDatabase;

class QComboBox;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Dialog for editing widget states (layouts).
class WidgetStatesDialog : public QDialog
{
    Q_OBJECT

    using _Myt = WidgetStatesDialog;
    using _Mybase = QDialog;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in,out] parent  parent widget
     * @param[in] f  window flags
     */
    WidgetStatesDialog( QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    /// Destructor.
    ~WidgetStatesDialog();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve size hint.
    /**
     * @return  size hint
     */
    virtual QSize sizeHint() const;

    // ========================================================================
    // Methods
    // ========================================================================

    /// Translate strings.
    virtual void translate();

private slots:

    /// Slot for button clicked.
    void onButtonClicked();

    /// Slot for current index changed.
    void onCurrentIndexChanged( int index );

    /// Slot for item selection changed.
    void onItemSelectionChanged();

private:

    AppDatabase *db_;

    QString currentGroupName_;

    // ---- //

    QLabel *groupNameLabel_;
    QComboBox *groupName_;

    QLabel *statesLabel_;
    QListWidget *states_;

    QPushButton *copyState_;
    QPushButton *renameState_;
    QPushButton *deleteState_;

    QPushButton *okay_;
    QPushButton *cancel_;

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    /// Retrieve selected item.
    QListWidgetItem *selectedItem() const;

    /// Select item.
    void selectItem( int index );

    /// Save to database.
    void saveForm();

    // not implemented
    WidgetStatesDialog( const _Myt& ) = delete;

    // not implemented
    WidgetStatesDialog( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // WIDGETSTATESDIALOG_H
