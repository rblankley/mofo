/**
 * @file advancedfilterlistwidget.h
 * Advanced filter(s) list widget.
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

#ifndef ADVANCEDFILTERLISTWIDGET_H
#define ADVANCEDFILTERLISTWIDGET_H

#include <QListWidget>

class AdvancedFilterWidget;

class QListWidgetItem;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Advanced filter(s) list widget.
class AdvancedFilterListWidget : public QListWidget
{
    Q_OBJECT
    Q_PROPERTY( QStringList filters READ filters WRITE setFilters )

    using _Myt = AdvancedFilterListWidget;
    using _Mybase = QListWidget;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] parent  parent object
     */
    AdvancedFilterListWidget( QWidget *parent = nullptr );

    /// Destructor.
    virtual ~AdvancedFilterListWidget();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve filters.
    /**
     * @return  list of filters
     */
    virtual QStringList filters() const;

    /// Set filters.
    /**
     * @param[in] value  filters
     */
    virtual void setFilters( const QStringList& value );

    // ========================================================================
    // Methods
    // ========================================================================

    /// Translate strings.
    virtual void translate();

public slots:

    /// Add filter item to list.
    virtual void addFilterItem();

private slots:

    /// Slot for removing filter item.
    void onRemoveFilterItem();

private:

    using FilterItemMap = QMap<AdvancedFilterWidget*, QListWidgetItem*>;

    FilterItemMap w_;

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    /// Create filter item.
    AdvancedFilterWidget *createFilterItem();

    /// Remove a filter item from list.
    void removeFilterItem( AdvancedFilterWidget *f );

    /// Remove all filter items from list.
    void removeAllFilterItems();

    // not implemented
    AdvancedFilterListWidget( const _Myt& other ) = delete;

    // not implemented
    AdvancedFilterListWidget( const _Myt&& other ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt& rhs ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt&& rhs ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // ADVANCEDFILTERLISTWIDGET_H
