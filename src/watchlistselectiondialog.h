/**
 * @file watchlistselectiondialog.h
 * Dialog for selecting watchlists.
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

#ifndef WATCHLISTSELECTIONDIALOG_H
#define WATCHLISTSELECTIONDIALOG_H

#include <QDialog>
#include <QMap>

class QCheckBox;
class QLabel;
class QPushButton;
class QVBoxLayout;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Dialog for selecting watchlists.
class WatchlistSelectionDialog : public QDialog
{
    Q_OBJECT

    using _Myt = WatchlistSelectionDialog;
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
    WatchlistSelectionDialog( QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    /// Destructor.
    virtual ~WatchlistSelectionDialog();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve if watchlists exist.
    /**
     * @return  @c true if watchlists exist, @c false otherwise
     */
    virtual bool watchlistsExist() const;

    /// Retrieve selected watchlists.
    /**
     * @return  watchlists
     */
    virtual QString selected() const;

    /// Set selected watchlists
    /**
     * @param[in] value  watchlists
     */
    virtual void setSelected( const QString& value );

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

private:

    static const QString STATE_GROUP_NAME;

    using WatchlistCheckBoxMap = QMap<QString, QCheckBox*>;

    WatchlistCheckBoxMap boxes_;

    QVBoxLayout *boxesLayout_;

    // ---- //

    QLabel *watchListsLabel_;

    QPushButton *editWatchLists_;

    QPushButton *okay_;
    QPushButton *cancel_;

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    /// Generate watchlist check boxes.
    void generateBoxes();

    /// Save dialog state.
    void saveState( QDialog *w ) const;

    /// Restore dialog state.
    void restoreState( QDialog *w ) const;

    // not implemented
    WatchlistSelectionDialog( const _Myt& ) = delete;

    // not implemented
    WatchlistSelectionDialog( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // WATCHLISTSELECTIONDIALOG_H
