/**
 * @file watchlistdialog.h
 * Dialog for editing watchlists.
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

#ifndef WATCHLISTDIALOG_H
#define WATCHLISTDIALOG_H

#include <QDialog>
#include <QMap>
#include <QString>

class AppDatabase;

class QLabel;
class QListWidget;
class QListWidgetItem;
class QPlainTextEdit;
class QPushButton;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Dialog for editing watchlists.
class WatchlistDialog : public QDialog
{
    Q_OBJECT

    using _Myt = WatchlistDialog;
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
    WatchlistDialog( QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    /// Destructor.
    virtual ~WatchlistDialog();

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

    /// Slot for item selection changed.
    void onItemSelectionChanged();

    /// Slot for text changed.
    void onTextChanged();

private:

    static const QString STATE_GROUP_NAME;

    AppDatabase *db_;

    // ---- //

    QLabel *watchlistLabel_;
    QListWidget *watchlist_;

    QPushButton *createList_;
    QPushButton *copyList_;
    QPushButton *renameList_;
    QPushButton *deleteList_;

    QLabel *symbolsLabel_;
    QPlainTextEdit *symbols_;

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

    /// Save dialog state.
    void saveState( QDialog *w ) const;

    /// Restore dialog state.
    void restoreState( QDialog *w ) const;

    /// Generate list from text.
    static QStringList generateList( const QString& data );

    // not implemented
    WatchlistDialog( const _Myt& ) = delete;

    // not implemented
    WatchlistDialog( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // WATCHLISTDIALOG_H
