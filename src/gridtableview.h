/**
 * @file gridtableview.h
 * Grid table view widget.
 *
 * Based on code from https://github.com/eyllanesc/stackoverflow/tree/master/questions/46469720
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

#ifndef GRIDTABLEVIEW_H
#define GRIDTABLEVIEW_H

#include <QTableView>

class QStandardItemModel;
class GridTableHeaderView;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Grid table view widget.
class GridTableView : public QTableView
{
    Q_OBJECT

    using _Myt = GridTableView;
    using _Mybase = QTableView;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in,out] parent  parent
     */
    explicit GridTableView( QWidget *parent = nullptr );

    /// Destructor.
    virtual ~GridTableView();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve grid header view.
    /**
     * @param[in] orientation  which orientation to retrieve
     * @return  pointer to header view
     */
    virtual GridTableHeaderView *gridHeaderView( Qt::Orientation orientation ) const;

    /// Set grid header view.
    /**
     * @param[in] orientation  which orientation to set
     * @param[in] levels  number of levels
     */
    virtual void setGridHeaderView( Qt::Orientation orientation, int levels );

private slots:

    /// Slot for header section pressed.
    void onHeaderSectionPressed( const QPoint& pos, Qt::MouseButton button, int beginSection, int endSection );

private:

    // not implemented
    GridTableView( const _Myt& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt& ) = delete;

};

#endif // GRIDTABLEVIEW_H
