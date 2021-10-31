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
class QLineEdit;
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

    /// Check watch lists visible.
    /**
     * @return  @c true if visible, @c false otherwise
     */
    virtual bool isWatchListsVisible() const;

    /// Retrieve selected filter.
    /**
     * @return  filter
     */
    virtual QString selected() const;

    /// Retrieve watch lists.
    /**
     * @return  watch lists
     */
    virtual QString watchLists() const;

    /// Set default filter for selection.
    /**
     * @param[in] value  default filter
     */
    virtual void setDefaultFilter( const QString& value );

    /// Set default watch lists.
    /**
     * @param[in] value  default watch lists
     */
    virtual void setDefaultWatchLists( const QString& value );

    /// Set watch lists visible or not.
    /**
     * @param[in] value  @c true if visible, @c false otherwise
     */
    virtual void setWatchListsVisible( bool value );

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

    bool watchListsVisible_;

    QLabel *watchListsLabel_;
    QLineEdit *watchLists_;

    QLabel *filtersLabel_;
    QComboBox *filters_;

    QPushButton *okay_;
    QPushButton *cancel_;

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
