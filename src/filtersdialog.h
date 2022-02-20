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

class AppDatabase;

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

    /// Destructor.
    virtual ~FiltersDialog();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Check if cancel button is visible.
    /**
     * @return  @c true if visible, @c false otherwise
     */
    virtual bool isCancelButtonVisible() const {return cancelVisible_;}

    /// Retrieve selected filter.
    /**
     * @return  currently selected filter
     */
    virtual QString selected() const;

    /// Set cancel button visible.
    /**
     * @param[in] value  @c true for visible, @c false otherwise
     */
    virtual void setCancelButtonVisible( bool value );

    /// Set currently selected filter.
    /**
     * @param[in] value  selected filter
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

    static const QString STATE_GROUP_NAME;

    AppDatabase *db_;

    QString currentFilterName_;

    QTimer *closeEditorTimer_;

    bool cancelVisible_;

    // ---- //

    QLabel *filtersLabel_;
    QListWidget *filters_;

    QPushButton *createFilter_;
    QPushButton *editFilter_;
    QPushButton *copyFilter_;
    QPushButton *renameFilter_;
    QPushButton *deleteFilter_;

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

    /// Save dialog state.
    void saveState( QDialog *w ) const;

    /// Restore dialog state.
    void restoreState( QDialog *w ) const;

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
