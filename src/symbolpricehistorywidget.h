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
class QStandardItem;

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

    /// Slot for standard item changed.
    void onItemChanged( QStandardItem *item );

    /// Slot for value changed.
    void onValueChanged( int value );

private:

    static const QString STATE_GROUP_NAME;
    static const QString STATE_NAME;

    static constexpr int MIN_CANDLE_WIDTH = 5;
    static constexpr int SPACING = 6;

    bool init_;

    QString symbol_;
    QList<CandleData> candles_;

    QList<MovingAverages> ma_;

    QList<HistoricalVolatilities> hv_;
    QList<MovingAveragesConvergenceDivergence> macd_;
    QList<RelativeStrengthIndexes> rsi_;

    QPixmap graph_;
    QPixmap margin_;

    QComboBox *period_;

    QComboBox *freqMin_;
    QComboBox *freqDayWeek_;
    QComboBox *freqDayWeekMonth_;

    QComboBox *overlays_;
    QComboBox *lowers_;

    QScrollBar *scroll_;

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

    /// Check if we have historical volatilities.
    bool haveHistoricalVolatilities();

    /// Check if we have moving averages.
    bool haveMovingAverages();

    /// Check if we have moving averages convergence/divergence (MACD)
    bool haveMovingAveragesConvergenceDivergence( bool emaOnly = false );

    /// Check if we have realtive strength indexes.
    bool haveRelativeStrengthIndexes();

    /// Calculate min/max values from list data.
    template <class T>
    void calcMinMaxValues( const QList<T>& values, double& min, double& max, unsigned long long& vmax ) const;

    /// Calculate interval values.
    void calcIntervalValues( double min, double max, double h, double div, double& interval, int& numDecimals ) const;

    /// Draw graph.
    void drawGraph();

    /// Scale value.
    static int scaled( double p, double min, double max, int height );

    /// Translate overlays.
    static void translateOverlays( QComboBox *w );

    /// Translate lowers.
    static void translateLowers( QComboBox *w );

    /// Retrieve overlay color.
    static QColor overlayColor( const QString& desc );

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
