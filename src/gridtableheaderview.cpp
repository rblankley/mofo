/**
 * @file gridtableheaderview.cpp
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

#include "gridtableheaderview.h"
#include "gridtableheadermodel.h"

#include <QMouseEvent>
#include <QPainter>

///////////////////////////////////////////////////////////////////////////////////////////////////
GridTableHeaderView::GridTableHeaderView( Qt::Orientation orientation, int rows, int columns, QWidget *parent ) :
    _Mybase( orientation, parent )
{
    QSize baseSectionSize;

    if ( Qt::Horizontal == orientation )
    {
        baseSectionSize.setWidth( defaultSectionSize() );
        baseSectionSize.setHeight( DEFAULT_HEIGHT );
    }
    else if ( Qt::Vertical == orientation )
    {
        baseSectionSize.setWidth( DEFAULT_WIDTH );
        baseSectionSize.setHeight( defaultSectionSize() );
    }

    model_type *model( new model_type( rows, columns, this ) );

    for ( int row( rows ); row--; )
        for ( int column( columns ); column--; )
            model->setData( model->index( row, column ), baseSectionSize, Qt::SizeHintRole );

    setModel( model );

    connect( this, &QHeaderView::sectionResized, this, &_Myt::onSectionResized );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
GridTableHeaderView::~GridTableHeaderView()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GridTableHeaderView::setCellLabel( int row, int column, const QString& label )
{
    const QModelIndex idx( model()->index( row, column ) );
    model()->setData( idx, label, Qt::DisplayRole );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GridTableHeaderView::setCellBackgroundColor( int row, int column, const QColor& color )
{
    const QModelIndex idx( model()->index( row, column ) );
    model()->setData( idx, color, Qt::BackgroundRole );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GridTableHeaderView::setCellForegroundColor(int row, int column, const QColor& color )
{
    const QModelIndex idx( model()->index( row, column ) );
    model()->setData( idx, color, Qt::ForegroundRole );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QModelIndex GridTableHeaderView::indexAt( const QPoint& pos ) const
{
    const model_type *tblModel( qobject_cast<model_type*>( model() ) );

    const int rows( tblModel->rowCount() );
    const int cols( tblModel->columnCount() );

    const int logicalIdx( logicalIndexAt( pos ) );

    int delta = 0;

    if ( Qt::Horizontal == orientation() )
    {
        for ( int row( 0 ); row < rows; ++row )
        {
            const QModelIndex cellIndex( tblModel->index( row, logicalIdx ) );

            delta += cellIndex.data( Qt::SizeHintRole ).toSize().height();

            if ( pos.y() <= delta )
                return cellIndex;
        }
    }
    else if ( Qt::Vertical == orientation() )
    {
        for ( int col( 0 ); col < cols; ++col )
        {
            const QModelIndex cellIndex( tblModel->index( logicalIdx, col ) );

            delta += cellIndex.data( Qt::SizeHintRole ).toSize().width();

            if ( pos.x() <= delta )
                return cellIndex;
        }
    }

    return QModelIndex();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSize GridTableHeaderView::sizeHint() const
{
    const model_type *tblModel( qobject_cast<model_type*>( model() ) );

    int columns( tblModel->columnCount() );
    int rows( tblModel->rowCount() );

    int width( 0 );
    int height( 0 );

    while ( columns-- )
    {
        const QModelIndex cellIndex( tblModel->index( 0, columns ) );
        width += cellIndex.data( Qt::SizeHintRole ).toSize().width();
    }

    while ( rows-- )
    {
        const QModelIndex cellIndex( tblModel->index( rows, 0 ) );
        height += cellIndex.data( Qt::SizeHintRole ).toSize().height();
    }

    return QSize( width, height );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GridTableHeaderView::paintSection( QPainter *painter, const QRect& rect, int logicalIndex ) const
{
    const model_type *tblModel( qobject_cast<const model_type*>( model() ) );

    const int level( (Qt::Horizontal == orientation()) ? tblModel->rowCount() : tblModel->columnCount() );

    // paint each level
    for ( int i( 0 ); i < level; ++i )
    {
        QModelIndex cellIndex( (Qt::Horizontal == orientation()) ? tblModel->index( i, logicalIndex ) : tblModel->index( logicalIndex, i ) );

        QRect sectionRect( rect );

        // set position of the cell
        if ( Qt::Horizontal == orientation() )
            sectionRect.setTop( rowSpanSize( logicalIndex, 0, i ) ); // distance from 0 to i-1 rows
        else
            sectionRect.setLeft( columnSpanSize( logicalIndex, 0, i ) );

        sectionRect.setSize( cellIndex.data( Qt::SizeHintRole ).toSize() );

        // check up span column or row
        const QModelIndex colSpanIdx( columnSpanIndex( cellIndex ) );
        const QModelIndex rowSpanIdx( rowSpanIndex( cellIndex ) );

        if ( colSpanIdx.isValid() )
        {
            const int colSpanFrom( colSpanIdx.column() );
            const int colSpanCnt( colSpanIdx.data( model_type::ColumnSpanRole ).toInt() );
            const int colSpanTo( colSpanFrom + colSpanCnt - 1 );
            const int colSpan( columnSpanSize( cellIndex.row(), colSpanFrom, colSpanCnt ) );

            if ( Qt::Horizontal == orientation() )
                sectionRect.setLeft( sectionViewportPosition( colSpanFrom ) );
            else
            {
                sectionRect.setLeft( columnSpanSize( logicalIndex, 0, colSpanFrom ) );
                i = colSpanTo;
            }

            sectionRect.setWidth( colSpan );

            // check up  if the column span index has row span
            const QVariant subRowSpanData( colSpanIdx.data( model_type::RowSpanRole ) );

            if ( subRowSpanData.isValid() )
            {
                const int subRowSpanFrom( colSpanIdx.row() );
                const int subRowSpanCnt( subRowSpanData.toInt() );
                const int subRowSpanTo( subRowSpanFrom + subRowSpanCnt - 1 );
                const int subRowSpan( rowSpanSize( colSpanFrom, subRowSpanFrom, subRowSpanCnt ) );

                if ( Qt::Vertical == orientation() )
                    sectionRect.setTop( sectionViewportPosition( subRowSpanFrom ) );
                else
                {
                    sectionRect.setTop( rowSpanSize( colSpanFrom, 0, subRowSpanFrom ) );
                    i = subRowSpanTo;
                }

                sectionRect.setHeight( subRowSpan );
            }

            cellIndex = colSpanIdx;
        }

        if ( rowSpanIdx.isValid() )
        {
            const int rowSpanFrom( rowSpanIdx.row() );
            const int rowSpanCnt( rowSpanIdx.data( model_type::RowSpanRole ).toInt() );
            const int rowSpanTo( rowSpanFrom + rowSpanCnt - 1 );
            const int rowSpan( rowSpanSize( cellIndex.column(), rowSpanFrom, rowSpanCnt ) );

            if ( Qt::Vertical == orientation() )
                sectionRect.setTop( sectionViewportPosition( rowSpanFrom ) );
            else
            {
                sectionRect.setTop( rowSpanSize( logicalIndex, 0, rowSpanFrom ) );
                i = rowSpanTo;
            }

            sectionRect.setHeight( rowSpan );

            // check up if the row span index has column span
            const QVariant subColSpanData( rowSpanIdx.data( model_type::ColumnSpanRole ) );

            if ( subColSpanData.isValid() )
            {
                const int subColSpanFrom( rowSpanIdx.column() );
                const int subColSpanCnt( subColSpanData.toInt() );
                const int subColSpanTo( subColSpanFrom + subColSpanCnt - 1 );
                const int subColSpan( columnSpanSize( rowSpanFrom, subColSpanFrom, subColSpanCnt ) );

                if ( Qt::Horizontal == orientation() )
                    sectionRect.setLeft( sectionViewportPosition( subColSpanFrom ) );
                else
                {
                    sectionRect.setLeft( columnSpanSize( rowSpanFrom, 0, subColSpanFrom ) );
                    i = subColSpanTo;
                }

                sectionRect.setWidth( subColSpan );
            }

            cellIndex = rowSpanIdx;
        }

        // draw section with style
        QStyleOptionHeader opt;
        initStyleOption( &opt );
        opt.textAlignment = Qt::AlignCenter;
        opt.iconAlignment = Qt::AlignVCenter;
        opt.section = logicalIndex;
        opt.text = cellIndex.data( Qt::DisplayRole ).toString();
        opt.rect = sectionRect;

        const QVariant bg( cellIndex.data( Qt::BackgroundRole ) );
        const QVariant fg( cellIndex.data( Qt::ForegroundRole ) );

        if ( bg.canConvert<QBrush>() )
        {
            opt.palette.setBrush( QPalette::Button, bg.value<QBrush>() );
            opt.palette.setBrush( QPalette::Window, bg.value<QBrush>() );
        }

        if ( fg.canConvert<QBrush>() )
        {
            opt.palette.setBrush( QPalette::ButtonText, fg.value<QBrush>() );
        }

        painter->save();
        style()->drawControl( QStyle::CE_Header, &opt, painter, this );
        painter->restore();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSize GridTableHeaderView::sectionSizeFromContents( int logicalIndex ) const
{
    const model_type *tblModel( qobject_cast<const model_type*>( model() ) );

    const int level( (Qt::Horizontal == orientation()) ? tblModel->rowCount() : tblModel->columnCount() );

    QSize size( QHeaderView::sectionSizeFromContents( logicalIndex ) );

    for ( int i( 0 ); i < level; ++i )
    {
        const QModelIndex cellIndex( (Qt::Horizontal == orientation()) ? tblModel->index( i, logicalIndex ) : tblModel->index( logicalIndex, i ) );

        const QModelIndex colSpanIdx( columnSpanIndex( cellIndex ) );
        const QModelIndex rowSpanIdx( rowSpanIndex( cellIndex ) );

        size = cellIndex.data( Qt::SizeHintRole ).toSize();

        if (colSpanIdx.isValid())
        {
            const int colSpanFrom( colSpanIdx.column() );
            const int colSpanCnt( colSpanIdx.data( model_type::ColumnSpanRole ).toInt() );
            const int colSpanTo( colSpanFrom + colSpanCnt - 1 );

            size.setWidth( columnSpanSize( colSpanIdx.row(), colSpanFrom, colSpanCnt ) );

            if ( Qt::Vertical == orientation() )
                i = colSpanTo;
        }

        if ( rowSpanIdx.isValid() )
        {
            const int rowSpanFrom( rowSpanIdx.row() );
            const int rowSpanCnt( rowSpanIdx.data( model_type::RowSpanRole ).toInt() );
            const int rowSpanTo( rowSpanFrom + rowSpanCnt - 1 );

            size.setHeight( rowSpanSize( rowSpanIdx.column(), rowSpanFrom, rowSpanCnt ) );

            if ( Qt::Horizontal == orientation() )
                i = rowSpanTo;
        }
    }

    return size;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GridTableHeaderView::setRowHeight( int row, int height )
{
    model_type *m( qobject_cast<model_type*>( model() ) );

    int col( m->columnCount() );

    while ( col-- )
    {
        const QModelIndex idx( m->index( row, col ) );

        QSize sz( idx.data( Qt::SizeHintRole ).toSize() );
        sz.setHeight( height );

        m->setData( idx, sz, Qt::SizeHintRole );
    }

    if ( Qt::Vertical == orientation() )
        resizeSection( row, height );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GridTableHeaderView::setColumnWidth( int col, int width )
{
    model_type *m( qobject_cast<model_type*>( model() ) );

    int row( m->rowCount() );

    while ( row-- )
    {
        const QModelIndex idx( m->index( row, col ) );

        QSize sz( idx.data( Qt::SizeHintRole ).toSize() );
        sz.setWidth( width );

        m->setData( idx, sz, Qt::SizeHintRole );
    }

    if ( Qt::Horizontal == orientation() )
        resizeSection( col, width );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GridTableHeaderView::setSpan( int row, int column, int rowSpanCount, int columnSpanCount )
{
    model_type *m( qobject_cast<model_type*>( model() ) );

    const QModelIndex idx( m->index( row, column ) );

    if ( 0 < rowSpanCount )
        m->setData( idx, rowSpanCount, model_type::RowSpanRole );

    if ( 0 < columnSpanCount )
        m->setData( idx, columnSpanCount, model_type::ColumnSpanRole );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GridTableHeaderView::setSpan( int row, int column )
{
    if ( Qt::Horizontal == orientation() )
        setSpan( row, column, model()->rowCount(), 1 );
    else
        setSpan( row, column, 1, model()->columnCount() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QModelIndex GridTableHeaderView::columnSpanIndex( const QModelIndex& index ) const
{
    const model_type *tblModel( qobject_cast<const model_type *>( model() ) );

    const int curRow( index.row() );
    const int curCol( index.column() );

    int i( curCol );

    while ( 0 <= i )
    {
        const QModelIndex spanIndex( tblModel->index( curRow, i ) );
        const QVariant span( spanIndex.data( model_type::ColumnSpanRole ) );

        if (( span.isValid() ) && ( curCol <= (spanIndex.column() + span.toInt() - 1) ))
            return spanIndex;

        --i;
    }

    return QModelIndex();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QModelIndex GridTableHeaderView::rowSpanIndex( const QModelIndex& index ) const
{
    const model_type *tblModel( qobject_cast<const model_type*>( model() ) );

    const int curRow( index.row() );
    const int curCol( index.column() );

    int i( curRow );

    while ( 0 <= i )
    {
        const QModelIndex spanIndex( tblModel->index( i, curCol ) );
        const QVariant span( spanIndex.data( model_type::RowSpanRole ) );

        if (( span.isValid() ) && ( curRow <= (spanIndex.row() + span.toInt() - 1) ))
            return spanIndex;

        --i;
    }

    return QModelIndex();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int GridTableHeaderView::columnSpanSize( int row, int from, int spanCount ) const
{
    const model_type *tblModel( qobject_cast<const model_type*>( model() ) );

    int span = 0;

    for ( int i( from ); i < (from + spanCount); ++i )
    {
        const QSize cellSize( tblModel->index( row, i ).data( Qt::SizeHintRole ).toSize() );

        span += cellSize.width();
    }

    return span;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int GridTableHeaderView::rowSpanSize( int column, int from, int spanCount ) const
{
    const model_type *tblModel( qobject_cast<const model_type*>( model() ) );

    int span = 0;

    for ( int i( from ); i < (from + spanCount); ++i )
    {
        const QSize cellSize( tblModel->index( i, column ).data( Qt::SizeHintRole ).toSize() );

        span += cellSize.height();
    }

    return span;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int GridTableHeaderView::calcSectionRange( QModelIndex& index, int *beginSection, int *endSection ) const
{
    const QModelIndex colSpanIdx( columnSpanIndex( index ) );
    const QModelIndex rowSpanIdx( rowSpanIndex( index ) );

    if ( colSpanIdx.isValid() )
    {
        const int colSpanFrom( colSpanIdx.column() );
        const int colSpanCnt( colSpanIdx.data( model_type::ColumnSpanRole ).toInt() );
        const int colSpanTo( colSpanFrom + colSpanCnt - 1 );

        if ( Qt::Horizontal  == orientation() )
        {
            index = colSpanIdx;
            *beginSection = colSpanFrom;
            *endSection = colSpanTo;

            return colSpanCnt;
        }

        const QVariant subRowSpanData( colSpanIdx.data( model_type::RowSpanRole ) );

        if ( subRowSpanData.isValid() )
        {
            const int subRowSpanFrom( colSpanIdx.row() );
            const int subRowSpanCnt( subRowSpanData.toInt() );
            const int subRowSpanTo( subRowSpanFrom + subRowSpanCnt - 1 );

            index = colSpanIdx;
            *beginSection = subRowSpanFrom;
            *endSection = subRowSpanTo;

            return subRowSpanCnt;
        }
    }

    if ( rowSpanIdx.isValid() )
    {
        const int rowSpanFrom( rowSpanIdx.row() );
        const int rowSpanCnt( rowSpanIdx.data( model_type::RowSpanRole ).toInt() );
        const int rowSpanTo( rowSpanFrom + rowSpanCnt - 1 );

        if ( Qt::Vertical == orientation() )
        {
            index = rowSpanIdx;
            *beginSection = rowSpanFrom;
            *endSection = rowSpanTo;

            return rowSpanCnt;
        }

        const QVariant subColSpanData( rowSpanIdx.data( model_type::ColumnSpanRole ) );

        if ( subColSpanData.isValid() )
        {
            const int subColSpanFrom( rowSpanIdx.column() );
            const int subColSpanCnt( subColSpanData.toInt() );
            const int subColSpanTo( subColSpanFrom + subColSpanCnt - 1 );

            index = rowSpanIdx;
            *beginSection = subColSpanFrom;
            *endSection = subColSpanTo;

            return subColSpanCnt;
        }
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GridTableHeaderView::mouseReleaseEvent( QMouseEvent *event )
{
    _Mybase::mouseReleaseEvent( event );

    // ---- //

    QModelIndex index( indexAt( event->pos() ) );

    if ( index.isValid() )
    {
        // determine what columns/rows were pressed
        int beginSection( (Qt::Horizontal == orientation()) ? index.column() : index.row() );
        int endSection( beginSection );

        calcSectionRange( index, &beginSection, &endSection );

        emit sectionPressed( event->pos(), event->button(), beginSection, endSection );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GridTableHeaderView::onSectionResized( int logicalIndex, int oldSize, int newSize )
{
    Q_UNUSED( oldSize )

    model_type *tblModel( qobject_cast<model_type*>( model() ) );

    const int pos( sectionViewportPosition( logicalIndex ) );

    const int level( (Qt::Horizontal == orientation()) ? tblModel->rowCount() : tblModel->columnCount() );
    const int xx( (Qt::Horizontal == orientation()) ? pos : 0 );
    const int yy( (Qt::Horizontal == orientation()) ? 0 : pos );

    QRect sectionRect( xx, yy, 0, 0 );

    for ( int i( 0 ); i < level; ++i )
    {
        const QModelIndex cellIndex( (Qt::Horizontal == orientation()) ? tblModel->index( i, logicalIndex ) : tblModel->index( logicalIndex, i ) );

        QSize cellSize( cellIndex.data( Qt::SizeHintRole ).toSize() );

        // set position of cell
        if ( Qt::Horizontal == orientation() )
        {
            sectionRect.setTop( rowSpanSize( logicalIndex, 0, i ) );
            cellSize.setWidth( newSize );
        }
        else
        {
            sectionRect.setLeft( columnSpanSize( logicalIndex, 0, i ) );
            cellSize.setHeight( newSize );
        }

        tblModel->setData( cellIndex, cellSize, Qt::SizeHintRole );

        const QModelIndex colSpanIdx( columnSpanIndex( cellIndex ) );
        const QModelIndex rowSpanIdx( rowSpanIndex( cellIndex ) );

        if ( colSpanIdx.isValid() )
        {
            const int colSpanFrom( colSpanIdx.column() );

            if ( Qt::Horizontal == orientation() )
                sectionRect.setLeft( sectionViewportPosition( colSpanFrom ) );
            else
                sectionRect.setLeft( columnSpanSize( logicalIndex, 0, colSpanFrom ) );

        }

        if ( rowSpanIdx.isValid() )
        {
            const int rowSpanFrom( rowSpanIdx.row() );

            if ( Qt::Vertical == orientation() )
                sectionRect.setTop( sectionViewportPosition( rowSpanFrom ) );
            else
                sectionRect.setTop( rowSpanSize( logicalIndex, 0, rowSpanFrom ) );
        }

        QRect rToUpdate( sectionRect );
        rToUpdate.setWidth( viewport()->width() - sectionRect.left() );
        rToUpdate.setHeight( viewport()->height() - sectionRect.top() );

        viewport()->update( rToUpdate.normalized() );
    }
}
