/**
 * @file symbolimplvolwidget.h
 * Implied Volatility Skew (Graph) for a symbol.
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

#ifndef SYMBOLIMPLVOLWIDGET_H
#define SYMBOLIMPLVOLWIDGET_H

#include <QDate>
#include <QDateTime>
#include <QList>
#include <QPixmap>
#include <QWidget>

#include "db/optiondata.h"

class QComboBox;
class QStandardItem;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Implied Volatility Skew (Graph) for a symbol.
class SymbolImpliedVolatilityWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString symbol READ symbol )

    using _Myt = SymbolImpliedVolatilityWidget;
    using _Mybase = QWidget;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] symbol  symbol
     * @param[in] price  market price per share
     * @param[in] parent  parent object
     */
    SymbolImpliedVolatilityWidget( const QString& symbol, double price, QWidget *parent = nullptr );

    /// Destructor.
    virtual ~SymbolImpliedVolatilityWidget();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve symbol.
    /**
     * @return  symbol
     */
    virtual QString symbol() const {return symbol_;}

    // ========================================================================
    // Methods
    // ========================================================================

    /// Translate strings.
    virtual void translate();

public slots:

    /// Refresh graph data.
    virtual void refreshData();

protected:

    // ========================================================================
    // Events
    // ========================================================================

    /// Paint event.
    /**
     * @param[in,out] e  event
     */
    virtual void paintEvent( QPaintEvent *e ) override;

    /// Resize event.
    /**
     * @param[in,out] e  event
     */
    virtual void resizeEvent( QResizeEvent *e ) override;

private slots:

    /// Slot for standard item changed.
    void onItemChanged( QStandardItem *item );

private:

    static constexpr int SPACING = 6;

    QString symbol_;
    double price_;

    QDateTime stamp_;
    QList<QDate> expiryDates_;

    QMap<QDate, OptionChainCurves> curves_;

    QPixmap graph_;

    QComboBox *dates_;

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    /// Calculate min/max values from list data.
    bool calcMinMaxValues( const QMap<double, double>& values, double& kmin, double& kmax, double& vmin, double& vmax ) const;

    /// Calculate interval values.
    void calcIntervalValues( double min, double max, double h, double ints, double& interval, int& numDecimals ) const;

    /// Draw graph.
    void drawGraph();

    /// Scale value.
    static int scaled( double p, double min, double max, int height );

    /// Retrieve date color.
    static QColor dateColor( const QString& desc );

    // not implemented
    SymbolImpliedVolatilityWidget( const _Myt& other ) = delete;

    // not implemented
    SymbolImpliedVolatilityWidget( const _Myt&& other ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt& rhs ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt&& rhs ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // SYMBOLIMPLVOLWIDGET_H
