/**
 * @file riskfreeinterestrateswidget.h
 * Interest Rate History (Graph).
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

#ifndef RISKFREEINTERESTRATESWIDGET_H
#define RISKFREEINTERESTRATESWIDGET_H

#include <QMap>
#include <QPixmap>
#include <QWidget>

class QComboBox;
class QScrollBar;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Interest Rate History (Graph).
class RiskFreeInterestRatesWidget : public QWidget
{
    Q_OBJECT

    using _Myt = RiskFreeInterestRatesWidget;
    using _Mybase = QWidget;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] parent  parent object
     */
    RiskFreeInterestRatesWidget( QWidget *parent = nullptr );

    /// Destructor.
    virtual ~RiskFreeInterestRatesWidget();

    // ========================================================================
    // Methods
    // ========================================================================

    /// Translate strings.
    virtual void translate();

public slots:

    /// Refresh underlying data.
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

    /// Slot for current index changed.
    void onCurrentIndexChanged( int index );

    /// Slot for value changed.
    void onValueChanged( int value );

private:

    static const QString STATE_GROUP_NAME;
    static const QString STATE_NAME;

    static constexpr int MIN_RATE_WIDTH = 5;
    static constexpr int SPACING = 6;

    static constexpr int MIN_RATE_WIDTH_DAY = 3;

    using ValuesMap = QMap<QDate, double>;
    using TermMap = QMap<QString, ValuesMap>;

    bool init_;
    bool scrollVisible_;

    QPixmap graph_;

    QComboBox *term_;
    QComboBox *period_;

    QScrollBar *scroll_;

    TermMap rates_;

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    /// Check is scroll bar is visible.
    bool scrollBarVisible() const {return scrollVisible_;}

    /// Calculate min/max values from list data.
    bool calcMinMaxValues( const ValuesMap& values, QDate& kmin, QDate& kmax, double& vmin, double& vmax ) const;

    /// Calculate interval values.
    void calcIntervalValues( double min, double max, double h, double ints, double& interval, int& numDecimals ) const;

    /// Draw graph.
    void drawGraph();

    /// Scale value.
    static int scaled( double p, double min, double max, int height );

    // not implemented
    RiskFreeInterestRatesWidget( const _Myt& other ) = delete;

    // not implemented
    RiskFreeInterestRatesWidget( const _Myt&& other ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt& rhs ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt&& rhs ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // RISKFREEINTERESTRATESWIDGET_H
