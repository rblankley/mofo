/**
 * @file symbolpricehistorywidget.h
 * Price History (Graph) for a symbol.
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

#ifndef SYMBOLPRICEHISTORYWIDGET_H
#define SYMBOLPRICEHISTORYWIDGET_H

#include <QPixmap>
#include <QWidget>

#include "db/candledata.h"

class QComboBox;
class QScrollBar;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Price History (Graph) for a symbol.
class SymbolPriceHistoryWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString symbol READ symbol )

    using _Myt = SymbolPriceHistoryWidget;
    using _Mybase = QWidget;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] symbol  symbol
     * @param[in] parent  parent object
     */
    SymbolPriceHistoryWidget( const QString& symbol, QWidget *parent = nullptr );

    /// Destructor.
    virtual ~SymbolPriceHistoryWidget();

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
    void paintEvent( QPaintEvent *e ) override;

    /// Resize event.
    /**
     * @param[in,out] e  event
     */
    void resizeEvent( QResizeEvent *e ) override;

private slots:

    /// Slot for candle data changed.
    void onCandleDataChanged( const QString& symbol, const QDateTime& start, const QDateTime& stop, int period, const QString& periodType, int freq, const QString& freqType, const QList<CandleData>& candles );

    /// Slot for current index changed.
    void onCurrentIndexChanged( int index );

    /// Slot for value changed.
    void onValueChanged( int value );

private:

    static const QString STATE_GROUP_NAME;
    static const QString STATE_NAME;

    static constexpr int MIN_CANDLE_WIDTH = 5;
    static constexpr int SPACING = 6;

    QString symbol_;
    QList<CandleData> candles_;

    QComboBox *period_;

    QComboBox *freqMin_;
    QComboBox *freqDayWeek_;
    QComboBox *freqDayWeekMonth_;

    QScrollBar *scroll_;

    unsigned long long vmax_;

    double gmin_;
    double gmax_;
    double ginterval_;

    int numDecimalPlaces_;

    QPixmap graph_;
    QPixmap margin_;

    int candlesWidth_;
    int marginWidth_;
    int marginHeight_;

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    /// Retrieve current period.
    bool currentPeriod( int& period, QString& periodType ) const;

    /// Retrieve current frequency.
    bool currentFrequency( int& freq, QString& freqType ) const;

    /// Retrieve scroll bar maximum value.
    int scrollBarMaximum() const;

    /// Check is scroll bar is visible.
    bool scrollBarVisible() const {return (0 < scrollBarMaximum());}

    /// Draw graph.
    void drawGraph();

    /// Scale value.
    static int scaled( double p, double min, double max, int height );

    // not implemented
    SymbolPriceHistoryWidget( const _Myt& other ) = delete;

    // not implemented
    SymbolPriceHistoryWidget( const _Myt&& other ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt& rhs ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt&& rhs ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // SYMBOLPRICEHISTORYWIDGET_H
