/**
 * @file advancedfilterwidget.h
 * Widget for creating an advanced filter.
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

#ifndef ADVANCEDFILTERWIDGET_H
#define ADVANCEDFILTERWIDGET_H

#include <QWidget>

class SqlTableModel;

class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QSpinBox;
class QToolButton;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Widget for creating an advanced filter.
class AdvancedFilterWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString filter READ filter WRITE setFilter )

    using _Myt = AdvancedFilterWidget;
    using _Mybase = QWidget;

signals:

    /// Signal to remove filter item.
    void remove();

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] parent  parent object
     */
    AdvancedFilterWidget( QWidget *parent = nullptr );

    /// Destructor.
    virtual ~AdvancedFilterWidget();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve filter.
    /**
     * @return  filter
     */
    virtual QString filter() const;

    /// Set filter.
    /**
     * @param[in] value  filter
     */
    virtual void setFilter( const QString& value );

    // ========================================================================
    // Methods
    // ========================================================================

    /// Translate strings.
    virtual void translate();

protected:

    // ========================================================================
    // Events
    // ========================================================================

    /// Resize event.
    /**
     * @param[in,out] e  event
     */
    virtual void resizeEvent( QResizeEvent *e ) override;

private slots:

    /// Slot for current index changed.
    void onCurrentIndexChanged( int index );

private:

    QComboBox *table_;

    QComboBox *operand_;

    QComboBox *tableVal_;
    QLineEdit *stringVal_;
    QSpinBox *integerVal_;
    QDoubleSpinBox *doubleVal_;

    QToolButton *remove_;

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    /// Populate combo with table columns.
    template <class T>
    static void populateTableColumns( const QString& tableName, const T& table, const QString& type, QComboBox *w );

    /// Populate combo with table columns.
    static void populateTableColumns( QComboBox *w, const QString& type = QString() );

    /// Retrieve column description.
    template <class T>
    static QString columnDescription( const T& table, int col );

    /// Retrieve charting value description.
    static QString chartingValueDescription( const QString& data );

    /// Translate table columns.
    static void translateTableColumns( QComboBox *w );

    // not implemented
    AdvancedFilterWidget( const _Myt& other ) = delete;

    // not implemented
    AdvancedFilterWidget( const _Myt&& other ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt& rhs ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt&& rhs ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // ADVANCEDFILTERWIDGET_H
