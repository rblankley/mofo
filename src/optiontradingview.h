/**
 * @file optiontradingview.h
 * Grid table view for option trading analysis.
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

#ifndef OPTIONTRADINGVIEW_H
#define OPTIONTRADINGVIEW_H

#include "gridtableview.h"

class GridTableView;
class HoverItemDelegate;
class OptionTradingItemModel;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Grid table view for option trading analysis.
class OptionTradingView : public GridTableView
{
    Q_OBJECT

    using _Myt = OptionTradingView;
    using _Mybase = GridTableView;

signals:

    /// Clear hover region.
    void clearHoverRegion();

    /// Signal for table item pressed.
    /**
     * @param[in] pos  position of button pressed
     * @param[in] button  button pressed
     * @param[in] row  row of item pressed
     * @param[in] column  column of item pressed
     */
    void itemPressed( const QPoint& pos, Qt::MouseButton button, int row, int column );

    /// Set hover region.
    /**
     * @param[in] row  item row
     * @param[in] from  column start
     * @param[in] to  column end
     */
    void setHoverRegion( int row, int from, int to );

public:

    /// Model type.
    using model_type = OptionTradingItemModel;

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] model  item model
     * @param[in] parent  parent object
     */
    OptionTradingView( model_type *model, QWidget *parent = nullptr );

    /// Destructor.
    virtual ~OptionTradingView();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve item model.
    /**
     * @return  item model
     */
    virtual model_type *model() const {return model_;}

    // ========================================================================
    // Methods
    // ========================================================================

    /// Translate strings.
    virtual void translate();

protected:

    // ========================================================================
    // Events
    // ========================================================================

    /// Leave event.
    /**
     * @param[in,out] event  event
     */
    virtual void leaveEvent( QEvent *event ) override;

    /// Mouse move event.
    /**
     * @param[in,out] event  event
     */
    virtual void mouseMoveEvent( QMouseEvent *event ) override;

    /// Mouse release event.
    /**
     * @param[in,out] event  event
     */
    virtual void mouseReleaseEvent( QMouseEvent *event ) override;

private slots:

    /// Slot for header section moved.
    void onHeaderSectionMoved( int logicalIndex, int oldVisualIndex, int newVisualIndex );

    /// Slot for header section pressed.
    void onHeaderSectionPressed( const QPoint& pos, Qt::MouseButton button, int from, int to );

    /// Slot for header section resized.
    void onHeaderSectionResized( int logicalIndex, int oldSize, int newSize );

    /// Slot for table item pressed.
    void onItemPressed( const QPoint& pos, Qt::MouseButton button, int row, int column );

private:

    static const QString STATE_GROUP_NAME;
    static const QString STATE_NAME;

    static constexpr int DEFAULT_WIDTH = 75;
    static constexpr int DEFAULT_HEIGHT = 20;

    model_type *model_;

    HoverItemDelegate *itemDelegate_;

    QString currentState_;

    int prevRow_;

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    /// Retrieve column header text.
    QString columnHeaderText( int column ) const;

    /// Save header view state.
    void saveHeaderState( const QHeaderView *view, const QString& name = STATE_NAME );

    /// Restore header view state.
    void restoreHeaderState( QHeaderView *view, const QString& name = STATE_NAME );

    /// Reset header view state.
    void resetHeaderState( QHeaderView *view );

    // not implemented
    OptionTradingView( const _Myt& ) = delete;

    // not implemented
    OptionTradingView( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // OPTIONTRADINGVIEW_H
