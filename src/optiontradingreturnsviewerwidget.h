/**
 * @file optiontradingreturnsviewerwidget.h
 * Widget for viewing option trade estimated returns information.
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

#ifndef OPTIONTRADINGRETURNSVIEWERWIDGET_H
#define OPTIONTRADINGRETURNSVIEWERWIDGET_H

#include <QWidget>

class OptionTradingItemModel;

class QLabel;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Widget for viewing option trade estimated returns information.
class OptionTradingReturnsViewerWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString underlying READ underlying )

    using _Myt = OptionTradingReturnsViewerWidget;
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
    OptionTradingReturnsViewerWidget( int index, model_type *model, QWidget *parent = nullptr );

    /// Destructor.
    virtual ~OptionTradingReturnsViewerWidget();

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

    /// Refresh underlying data.
    virtual void refreshData();

private:

    model_type *model_;
    int index_;

    QString underlying_;
    double underlyingPrice_;

    int strat_;

    double longStrikePrice_;
    double shortStrikePrice_;

    bool isCall_;

    // ---- //

    QLabel *bidPriceLabel_;
    QLabel *bidPrice_;

    QLabel *askPriceLabel_;
    QLabel *askPrice_;

    QLabel *markPriceLabel_;
    QLabel *markPrice_;

    QLabel *daysToExpiryLabel_;
    QLabel *daysToExpiry_;

    QLabel *investPriceLabel_;
    QLabel *investPrice_;

    QLabel *theoPriceLabel_;
    QLabel *theoPrice_;

    QLabel *implVolLabel_;
    QLabel *implVol_;

    QLabel *histVolLabel_;
    QLabel *histVol_;

    QLabel *divAmountLabel_;
    QLabel *divAmount_;

    QLabel *riskFreeRateLabel_;
    QLabel *riskFreeRate_;

    // estimated returns
    QLabel *costOfEntryLabel_;
    QLabel *costOfEntry_;

    QLabel *maxRiskLabel_;
    QLabel *maxRisk_;

    QLabel *maxReturnLabel_;
    QLabel *maxReturn_;

    QLabel *maxReturnOnRiskLabel_;
    QLabel *maxReturnOnRisk_;

    QLabel *maxReturnOnInvestLabel_;
    QLabel *maxReturnOnInvest_;

    QLabel *expectedValueLabel_;
    QLabel *expectedValue_;

    QLabel *breakevenLabel_;
    QLabel *breakeven_;

    QLabel *probProfitLabel_;
    QLabel *probProfit_;

    // greeks
    QLabel *deltaLabel_;
    QLabel *delta_;

    QLabel *gammaLabel_;
    QLabel *gamma_;

    QLabel *thetaLabel_;
    QLabel *theta_;

    QLabel *vegaLabel_;
    QLabel *vega_;

    QLabel *rhoLabel_;
    QLabel *rho_;

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    /// Retrieve model data.
    QVariant modelData( int col ) const;

    /// Set label color.
    static void setLabelColor( QLabel *l, const QColor& c );

    /// Set label text.
    static void setLabelText( QLabel *l, const QString text, const QString emptyText = QString() );

    // not implemented
    OptionTradingReturnsViewerWidget( const _Myt& other ) = delete;

    // not implemented
    OptionTradingReturnsViewerWidget( const _Myt&& other ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt& rhs ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt&& rhs ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // OPTIONTRADINGRETURNSVIEWERWIDGET_H
