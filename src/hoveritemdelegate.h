/**
 * @file hoveritemdelegate.h
 * Item delegate for row hovering.
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

#ifndef HOVERITEMDELEGATE_H
#define HOVERITEMDELEGATE_H

#include <QStyledItemDelegate>

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Item delegate for row hovering.
class HoverItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    using _Myt = HoverItemDelegate;
    using _Mybase = QStyledItemDelegate;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] parent  parent
     */
    HoverItemDelegate( QObject *parent = nullptr );

    /// Destructor.
    virtual ~HoverItemDelegate();

    // ========================================================================
    // Methods
    // ========================================================================

    /// Paint method.
    /**
     * @param[in,out] painter  painter
     * @param[in] option  style option
     * @param[in] index  index
     */
    virtual void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override;

public slots:

    /// Set hover region.
    /**
     * @param[in] row  item row
     * @param[in] from  column start
     * @param[in] to  column end
     */
    virtual void setHoverRegion( int row, int from, int to );

    /// Clear hover region.
    virtual void clearHoverRegion();

private:

    int row_;

    int from_;
    int to_;

    // not implemented
    HoverItemDelegate( const _Myt& ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // HOVERITEMDELEGATE_H
