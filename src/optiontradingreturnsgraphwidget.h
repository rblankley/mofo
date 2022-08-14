/**
 * @file optiontradingreturnsgraphwidget.h
 * Widget for viewing option trade estimated returns information (graph).
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

#ifndef OPTIONTRADINGRETURNSGRAPHWIDGET_H
#define OPTIONTRADINGRETURNSGRAPHWIDGET_H

#include <QDate>
#include <QDateTime>
#include <QMap>
#include <QPixmap>
#include <QWidget>

class OptionTradingItemModel;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Widget for viewing option trade estimated returns information (graph).
class OptionTradingReturnsGraphWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString underlying READ underlying )

    using _Myt = OptionTradingReturnsGraphWidget;
    using _Mybase = QWidget;

public:

    /// Model type.
    using model_type = OptionTradingItemModel;

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] index  trading model index
     * @param[in] model  trading model
     * @param[in,out] parent  parent object
     */
    OptionTradingReturnsGraphWidget( int index, model_type *model, QWidget *parent = nullptr );

    /// Destructor.
    virtual ~OptionTradingReturnsGraphWidget();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve underlying.
    /**
     * @return  underlying
     */
    virtual QString underlying() const {return underlying_;}

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

private:

    static constexpr int SPACING = 6;

    using LinearEquation = QPair<double, double>;
    using LinearEquationMap = QMap<double, LinearEquation>;

    using ValuesMap = QMap<double, double>;

    model_type *model_;
    int index_;

    QString underlying_;
    double underlyingPrice_;

    int strat_;

    double longStrikePrice_;
    double shortStrikePrice_;
    double breakEvenPrice_;

    QDateTime stamp_;
    QDate expiryDate_;

    ValuesMap returns_;

    QPixmap graph_;

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    /// Retrieve model data.
    QVariant modelData( int col ) const;

    /// Calculate min/max values from list data.
    bool calcMinMaxValues( const ValuesMap& values, double& kmin, double& kmax, double& vmin, double& vmax ) const;

    /// Calculate interval values.
    void calcIntervalValues( double min, double max, double h, double ints, double& interval, int& numDecimals ) const;

    /// Draw graph.
    void drawGraph();

    /// Scale value.
    static int scaled( double p, double min, double max, int height );

    // not implemented
    OptionTradingReturnsGraphWidget( const _Myt& other ) = delete;

    // not implemented
    OptionTradingReturnsGraphWidget( const _Myt&& other ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt& rhs ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt&& rhs ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // OPTIONTRADINGRETURNSGRAPHWIDGET_H
