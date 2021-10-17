/**
 * @file filterselectiondialog.h
 * Dialog for selecting a filter.
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

#ifndef FILTERSELECTIONDIALOG_H
#define FILTERSELECTIONDIALOG_H

#include <QDialog>

class QComboBox;
class QLabel;
class QPushButton;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Dialog for selecting a filter.
class FilterSelectionDialog : public QDialog
{
    Q_OBJECT

    using _Myt = FilterSelectionDialog;
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
    FilterSelectionDialog( QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve if filters exist.
    /**
     * @return  @c true if filters exist, @c false otherwise
     */
    virtual bool filtersExist() const;

    /// Retrieve selected filter.
    /**
     * @return  filter
     */
    virtual QString selected() const;

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
    QComboBox *filters_;

    QPushButton *okay_;
    QPushButton *cancel_;

private slots:

    /// Slot for button clicked.
    void onButtonClicked();

private:

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    // not implemented
    FilterSelectionDialog( const _Myt& ) = delete;

    // not implemented
    FilterSelectionDialog( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // FILTERSELECTIONDIALOG_H
