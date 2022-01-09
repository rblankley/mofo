/**
 * @file fundamentalsviewerwidget.h
 * Widget for viewing fundamentals information.
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

#ifndef FUNDAMENTALSVIEWERWIDGET_H
#define FUNDAMENTALSVIEWERWIDGET_H

#include <QWidget>

class FundamentalsTableModel;

class QLabel;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Widget for viewing fundamentals information.
class FundamentalsViewerWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString symbol READ symbol )

    using _Myt = FundamentalsViewerWidget;
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
    FundamentalsViewerWidget( const QString& symbol, double price, QWidget *parent = nullptr );

    /// Destructor.
    virtual ~FundamentalsViewerWidget();

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

private:

    using model_type = FundamentalsTableModel;

    model_type *model_;

    QString symbol_;
    double price_;

    // ---- //

    QLabel *avgVolumeLabel_;
    QLabel *avgVolume_;

    QLabel *yearRangeLabel_;
    QLabel *yearRange_;

    QLabel *percentBelowHighLabel_;
    QLabel *percentBelowHigh_;

    QLabel *divLabel_;
    QLabel *div_;

    QLabel *divDateLabel_;
    QLabel *divDate_;

    QLabel *divPayDateLabel_;
    QLabel *divPayDate_;

    QLabel *betaLabel_;
    QLabel *beta_;

    QLabel *shortIntLabel_;
    QLabel *shortInt_;

    // share values
    QLabel *epsLabel_;
    QLabel *eps_;

    QLabel *dpsLabel_;
    QLabel *dps_;

    QLabel *bpsLabel_;
    QLabel *bps_;

    QLabel *cfpsLabel_;
    QLabel *cfps_;

    QLabel *fcfpsLabel_;
    QLabel *fcfps_;

    QLabel *spsLabel_;
    QLabel *sps_;

    // profitability
    QLabel *roeLabel_;
    QLabel *roe_;

    QLabel *roaLabel_;
    QLabel *roa_;

    QLabel *grossProfitMarginLabel_;
    QLabel *grossProfitMargin_;

    QLabel *operProfitMarginLabel_;
    QLabel *operProfitMargin_;

    QLabel *taxRateLabel_;
    QLabel *taxRate_;

    QLabel *intRateLabel_;
    QLabel *intRate_;

    QLabel *netProfitMarginLabel_;
    QLabel *netProfitMargin_;

    // activity ratios
    QLabel *totalAssetTurnoverLabel_;
    QLabel *totalAssetTurnover_;

    QLabel *inventoryTurnoverLabel_;
    QLabel *inventoryTurnover_;

    // financial ratios
    QLabel *ltDebtToCapitalLabel_;
    QLabel *ltDebtToCapital_;

    QLabel *financialLeverageLabel_;
    QLabel *financialLeverage_;

    QLabel *fixedChargeCoverageRatioLabel_;
    QLabel *fixedChargeCoverageRatio_;

    QLabel *divPayoutRatioLabel_;
    QLabel *divPayoutRatio_;

    QLabel *quickRatioLabel_;
    QLabel *quickRatio_;

    QLabel *currentRatioLabel_;
    QLabel *currentRatio_;

    // valuation
    QLabel *peRatioLabel_;
    QLabel *peRatio_;

    QLabel *pcfRatioLabel_;
    QLabel *pcfRatio_;

    QLabel *pbRatioLabel_;
    QLabel *pbRatio_;

    QLabel *marketCapRatioLabel_;
    QLabel *marketCapRatio_;

    QLabel *divYieldLabel_;
    QLabel *divYield_;

    QLabel *divPayoutPerShareLabel_;
    QLabel *divPayoutPerShare_;

    QLabel *sharesOutstandingLabel_;
    QLabel *sharesOutstanding_;

    QLabel *marketCapLabel_;
    QLabel *marketCap_;

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    /// Set label text.
    static void setLabelText( QLabel *l, const QString text, const QString emptyText = QString() );

    // not implemented
    FundamentalsViewerWidget( const _Myt& other ) = delete;

    // not implemented
    FundamentalsViewerWidget( const _Myt&& other ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt& rhs ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt&& rhs ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // FUNDAMENTALSVIEWERWIDGET_H
