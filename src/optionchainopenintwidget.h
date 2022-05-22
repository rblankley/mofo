/**
 * @file optionchainopenintwidget.h
 * Open Interest (Graph) for an option chain.
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

#ifndef OPTIONCHAINOPENINTWIDGET_H
#define OPTIONCHAINOPENINTWIDGET_H

#include <QDate>
#include <QDateTime>
#include <QPixmap>
#include <QWidget>

#include "db/optiondata.h"

class QScrollBar;
class QToolButton;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Open Interest (Graph) for an option chain.
class OptionChainOpenInterestWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QDate expirationDate READ expirationDate )
    Q_PROPERTY( QString underlying READ underlying )
    Q_PROPERTY( double underlyingPrice READ underlyingPrice )

    using _Myt = OptionChainOpenInterestWidget;
    using _Mybase = QWidget;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] underlying  underlying symbol
     * @param[in] underlyingPrice  underlying market price per share
     * @param[in] expiryDate  option chain expiration date
     * @param[in] stamp  option chain stamp
     * @param[in] parent  parent object
     */
    OptionChainOpenInterestWidget( const QString& underlying, double underlyingPrice, const QDate& expiryDate, const QDateTime& stamp = QDateTime(), QWidget *parent = nullptr );

    /// Destructor.
    virtual ~OptionChainOpenInterestWidget();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve expiration date.
    /**
     * @return  expiration date
     */
    virtual QDate expirationDate() const {return expiryDate_;}

    /// Retrieve underlying symbol.
    /**
     * @return  underlying symbol
     */
    virtual QString underlying() const {return underlying_;}

    /// Retrieve underlying price.
    /**
     * @return  underlying price
     */
    virtual double underlyingPrice() const {return price_;}

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

    /// Slot for button clicked.
    void onButtonClicked();

    /// Slot for scroll bar value changed.
    void onValueChanged( int value );

private:

    static constexpr int SPACING = 6;

    static constexpr int BAR_SEPARATION = 3;
    static constexpr int BAR_WIDTH = 8;
    static constexpr int BAR_WIDTH_MIN = 2;
    static constexpr int BAR_WIDTH_MAX = 32;

    static constexpr int MIN_ZOOM = BAR_WIDTH - BAR_WIDTH_MAX;
    static constexpr int MAX_ZOOM = BAR_WIDTH - BAR_WIDTH_MIN;

    using ValuesMap = QMap<double, int>;

    QString underlying_;
    double price_;

    QDateTime end_;

    QDateTime stamp_;
    QDate expiryDate_;

    OptionChainOpenInterest openInt_;

    QPixmap graph_;

    double multiplier_;
    int step_;

    int zoom_;

    QToolButton *zout_;
    QToolButton *zin_;

    QScrollBar *scroll_;

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    /// Check for curve data.
    bool haveCurveData() const;

    /// Calculate min/max values from list data.
    bool calcMinMaxValues( const ValuesMap& values, double& kmin, double& kmax, int& vmin, int& vmax ) const;

    /// Calculate interval values.
    void calcIntervalValues( double min, double max, double h, double ints, double& interval, int& numDecimals ) const;

    /// Draw graph.
    void drawGraph();

    /// Scale value.
    static int scaled( double p, double min, double max, int height );

    // not implemented
    OptionChainOpenInterestWidget( const _Myt& other ) = delete;

    // not implemented
    OptionChainOpenInterestWidget( const _Myt&& other ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt& rhs ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt&& rhs ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // OPTIONCHAINOPENINTWIDGET_H
