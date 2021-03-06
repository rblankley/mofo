/**
 * @file gridtableheaderview.h
 * Grid table header view widget.
 *
 * Based on code from https://github.com/eyllanesc/stackoverflow/tree/master/questions/46469720
 * Modified heavily to remove dependance on size hints and emit section pressed signals
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

#ifndef GRIDTABLEHEADERVIEW_H
#define GRIDTABLEHEADERVIEW_H

#include <QHeaderView>

class GridTableHeaderModel;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Grid table header view widget.
class GridTableHeaderView : public QHeaderView
{
    Q_OBJECT

    using _Myt = GridTableHeaderView;
    using _Mybase = QHeaderView;

signals:

    /// Signal for header section pressed.
    /**
     * @param[in] pos  position of button pressed
     * @param[in] button  button pressed
     * @param[in] from  starting index
     * @param[in] to  ending index
     */
    void sectionPressed( const QPoint& pos, Qt::MouseButton button, int from, int to );

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] orientation  header orientation
     * @param[in] rows  number of rows
     * @param[in] columns  number of columns
     * @param[in,out] parent  parent
     */
    GridTableHeaderView( Qt::Orientation orientation, int rows, int columns, QWidget *parent = nullptr );

    /// Destructor.
    virtual ~GridTableHeaderView();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Set cell background color.
    /**
     * @param[in] row  row
     * @param[in] column  column
     * @param[in] color  color
     */
    virtual void setCellBackgroundColor( int row, int column, const QColor& color );

    /// Set cell foreground color.
    /**
     * @param[in] row  row
     * @param[in] column  column
     * @param[in] color  color
     */
    virtual void setCellForegroundColor( int row, int column, const QColor& color );

    /// Set cell label.
    /**
     * @param[in] row  row
     * @param[in] column  column
     * @param[in] label  label text
     */
    virtual void setCellLabel( int row, int column, const QString& label );

    /// Set column width.
    /**
     * @param[in] col  column
     * @param[in] width  width
     */
    virtual void setColumnWidth( int col, int width );

    /// Set row height.
    /**
     * @param[in] row  row
     * @param[in] height  height
     */
    virtual void setRowHeight( int row, int height );

    /// Set span.
    /**
     * @param[in] row  row
     * @param[in] column  column
     * @param[in] rowSpanCount  row span
     * @param[in] columnSpanCount  column span
     */
    virtual void setSpan( int row, int column, int rowSpanCount, int columnSpanCount );

    /// Set span to entire view.
    /**
     * @param[in] row  row
     * @param[in] column  column
     */
    virtual void setSpan( int row, int column );

    /// Retrieve size hint.
    /**
     * @return  size
     */
    virtual QSize sizeHint() const override;

protected:

    // ========================================================================
    // Events
    // ========================================================================

    /// Mouse release event.
    /**
     * @param[in,out] event  event
     */
    virtual void mouseReleaseEvent( QMouseEvent *event ) override;

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve index as position.
    /**
     * @param[in] pos  point
     * @return  index
     */
    virtual QModelIndex indexAt( const QPoint& pos ) const override;

    /// Determine section size from contents.
    /**
     * @param[in] logicalIndex  logical index
     * @return  size
     */
    virtual QSize sectionSizeFromContents( int logicalIndex ) const override;

    /// Retrieve span index.
    /**
     * Compute what index spans into @a index.
     * @param[in] index  index
     * @return  span index
     */
    virtual QModelIndex spanIndex( const QModelIndex& index ) const;

    // ========================================================================
    // Methods
    // ========================================================================

    /// Calculate rectangle for index.
    /**
     * @param[in] index  index
     * @return  rect
     */
    virtual QRect calcRect( const QModelIndex& index ) const;

    /// Paint section.
    /**
     * @param[in,out] painter  painter
     * @param[in] rect  rect to paint
     * @param[in] logicalIndex  logical index
     */
    virtual void paintSection( QPainter *painter, const QRect& rect, int logicalIndex ) const override;

private:

    static constexpr int DEFAULT_WIDTH = 50;
    static constexpr int DEFAULT_HEIGHT = 20;

    using model_type = GridTableHeaderModel;

    QVector<int> sectionSize_;

    /// Retrieve span for horizontal orientation.
    QModelIndex spanIndexHorizontal( const QModelIndex& index, const QMap<int, int>& viewportPos ) const;

    /// Retrieve span for vertical orientation.
    QModelIndex spanIndexVertical( const QModelIndex& index, const QMap<int, int>& viewportPos ) const;

    // not implemented
    GridTableHeaderView( const _Myt& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // GRIDTABLEHEADERVIEW_H
