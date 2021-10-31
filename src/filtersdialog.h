/**
 * @file filtersdialog.h
 * Dialog for adding/removing filters.
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

#ifndef FILTERSDIALOG_H
#define FILTERSDIALOG_H

#include <QDialog>
#include <QStringList>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QPlainTextEdit;
class QPushButton;
class QTimer;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Dialog for adding/removing filters.
class FiltersDialog : public QDialog
{
    Q_OBJECT

    using _Myt = FiltersDialog;
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
    FiltersDialog( QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

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

protected:

    QLabel *filtersLabel_;
    QListWidget *filters_;

    QPushButton *createFilter_;
    QPushButton *editFilter_;
    QPushButton *renameFilter_;
    QPushButton *removeFilter_;

    QPushButton *okay_;

private slots:

    /// Close any open peristent editor.
    void closePersistentEditor();

    /// Slot for button clicked.
    void onButtonClicked();

    /// Slot for item changed.
    void onItemChanged( QListWidgetItem *item );

    /// Slot for item double clicked.
    void onItemDoubleClicked( QListWidgetItem *item );

    /// Slot for item selection changed.
    void onItemSelectionChanged();

private:

    QString currentFilterName_;

    QTimer *closeEditorTimer_;

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    /// Retrieve selected item.
    QListWidgetItem *selected() const;

    /// Select item.
    void selectItem( int index );

    // not implemented
    FiltersDialog( const _Myt& ) = delete;

    // not implemented
    FiltersDialog( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // FILTERSDIALOG_H
