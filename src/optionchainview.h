/**
 * @file optionchainview.h
 * Grid table view for option chains.
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

#ifndef OPTIONCHAINVIEW_H
#define OPTIONCHAINVIEW_H

#include "gridtableview.h"

class GridTableView;
class HoverItemDelegate;
class OptionChainTableModel;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Grid table view for option chains.
class OptionChainView : public GridTableView
{
    Q_OBJECT

    using _Myt = OptionChainView;
    using _Mybase = GridTableView;

signals:

    /// Set hover region.
    /**
     * @param[in] row  item row
     * @param[in] from  column start
     * @param[in] to  column end
     */
    void setHoverRegion( int row, int from, int to );

    /// Clear hover region.
    void clearHoverRegion();

public:

    /// Model type.
    using model_type = OptionChainTableModel;

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] model  item model
     * @param[in] parent  parent object
     */
    OptionChainView( model_type *model, QWidget *parent = nullptr );

    /// Destructor.
    virtual ~OptionChainView();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve option chain title.
    /**
     * @return  option chain title
     */
    virtual QString title() const;

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
     * @param[in,out] e  event
     */
    virtual void leaveEvent( QEvent *e ) override;

    /// Mouse move event.
    /**
     * @param[in,out] e  event
     */
    virtual void mouseMoveEvent( QMouseEvent *e ) override;

    /// Show event.
    /**
     * @param[in,out] e  event
     */
    virtual void showEvent( QShowEvent *e ) override;

private slots:

    /// Slot for header section moved.
    void onHeaderSectionMoved( int logicalIndex, int oldVisualIndex, int newVisualIndex );

    /// Slot for header section pressed.
    void onHeaderSectionPressed( const QPoint& pos, Qt::MouseButton button, int from, int to );

    /// Slot for header section resized.
    void onHeaderSectionResized( int logicalIndex, int oldSize, int newSize );

private:

    static const QString STATE_GROUP_NAME;
    static const QString STATE_NAME;

    static constexpr int DEFAULT_WIDTH = 75;
    static constexpr int DEFAULT_HEIGHT = 20;

    static constexpr int STRIKE_COLUMN_WIDTH = 100;

    model_type *model_;

    HoverItemDelegate *itemDelegate_;

    QString currentState_;

    int prevRow_;
    int prevColFrom_;
    int prevColTo_;

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
    OptionChainView( const _Myt& ) = delete;

    // not implemented
    OptionChainView( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // OPTIONCHAINVIEW_H
