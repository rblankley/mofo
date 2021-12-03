/**
 * @file filtereditordialog.h
 * Dialog for editing/modifying a filter.
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

#ifndef FILTEREDITORDIALOG_H
#define FILTEREDITORDIALOG_H

#include "optionprofitcalcfilter.h"

#include <QDialog>

class QCheckBox;
class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QSpinBox;
class QTabWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Dialog for editing/modifying a filter.
class FilterEditorDialog : public QDialog
{
    Q_OBJECT

    using _Myt = FilterEditorDialog;
    using _Mybase = QDialog;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] name  filter name
     * @param[in] value  filter
     * @param[in,out] parent  parent widget
     * @param[in] f  window flags
     */
    FilterEditorDialog( const QString& name, const QByteArray& value, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve filter value.
    /**
     * @return  filter
     */
    virtual QByteArray filterValue() const;

    /// Retrieve size hint.
    /**
     * @return  size hint
     */
    virtual QSize sizeHint() const override;

    // ========================================================================
    // Methods
    // ========================================================================

    /// Translate strings.
    virtual void translate();

protected:

    // ========================================================================
    // Events
    // ========================================================================

    /// Resize event.
    /**
     * @param[in,out] event  event
     */
    virtual void resizeEvent( QResizeEvent *event ) override;

protected:

    QLabel *minColumnLabel_;
    QLabel *maxColumnLabel_;

    QLabel *underlyingPriceLabel_;
    QDoubleSpinBox *minUnderlyingPrice_;
    QDoubleSpinBox *maxUnderlyingPrice_;

    QLabel *verticalDepthLabel_;
    QSpinBox *verticalDepth_;

    QTabWidget *tabs_;

    // ---- //

    QWidget *tab0_;

    QLabel *minColumnLabel0_;
    QLabel *maxColumnLabel0_;

    QLabel *investAmountLabel_;
    QDoubleSpinBox *minInvestAmount_;
    QDoubleSpinBox *maxInvestAmount_;

    QLabel *lossAmountLabel_;
    QDoubleSpinBox *maxLossAmount_;

    QLabel *gainAmountLabel_;
    QDoubleSpinBox *minGainAmount_;

    QLabel *bidSizeLabel_;
    QSpinBox *minBidSize_;

    QLabel *askSizeLabel_;
    QSpinBox *minAskSize_;

    QLabel *spreadPercentLabel_;
    QDoubleSpinBox *maxSpreadPercent_;

    QLabel *daysToExpiryLabel_;
    QSpinBox *minDaysToExpiry_;
    QSpinBox *maxDaysToExpiry_;

    QLabel *implVolatilityLabel_;
    QDoubleSpinBox *minImplVolatility_;
    QDoubleSpinBox *maxImplVolatility_;

    QLabel *divAmountLabel_;
    QDoubleSpinBox *minDivAmount_;
    QDoubleSpinBox *maxDivAmount_;

    QLabel *divYieldLabel_;
    QDoubleSpinBox *minDivYield_;
    QDoubleSpinBox *maxDivYield_;

    // ---- //

    QWidget *tab1_;

    QLabel *minColumnLabel1_;
    QLabel *maxColumnLabel1_;

    QLabel *probITMLabel_;
    QDoubleSpinBox *minProbITM_;
    QDoubleSpinBox *maxProbITM_;

    QLabel *probOTMLabel_;
    QDoubleSpinBox *minProbOTM_;
    QDoubleSpinBox *maxProbOTM_;

    QLabel *probProfitLabel_;
    QDoubleSpinBox *minProbProfit_;
    QDoubleSpinBox *maxProbProfit_;

    QLabel *returnOnRiskLabel_;
    QDoubleSpinBox *minReturnOnRisk_;
    QDoubleSpinBox *maxReturnOnRisk_;

    QLabel *returnOnRiskTimeLabel_;
    QDoubleSpinBox *minReturnOnRiskTime_;
    QDoubleSpinBox *maxReturnOnRiskTime_;

    QLabel *returnOnInvestmentLabel_;
    QDoubleSpinBox *minReturnOnInvestment_;
    QDoubleSpinBox *maxReturnOnInvestment_;

    QLabel *returnOnInvestmentTimeLabel_;
    QDoubleSpinBox *minReturnOnInvestmentTime_;
    QDoubleSpinBox *maxReturnOnInvestmentTime_;

    QLabel *expectedValueLabel_;
    QDoubleSpinBox *minExpectedValue_;
    QDoubleSpinBox *maxExpectedValue_;

    QLabel *expectedValueReturnOnInvestmentLabel_;
    QDoubleSpinBox *minExpectedValueReturnOnInvestment_;
    QDoubleSpinBox *maxExpectedValueReturnOnInvestment_;

    QLabel *expectedValueReturnOnInvestmentTimeLabel_;
    QDoubleSpinBox *minExpectedValueReturnOnInvestmentTime_;
    QDoubleSpinBox *maxExpectedValueReturnOnInvestmentTime_;

    // ---- //

    QGroupBox *optionTypes_;
    QCheckBox *itmCalls_;
    QCheckBox *otmCalls_;
    QCheckBox *itmPuts_;
    QCheckBox *otmPuts_;

    QGroupBox *optionTradingStrats_;
    QCheckBox *single_;
    QCheckBox *vertical_;
    QCheckBox *calendar_;
    QCheckBox *strangle_;
    QCheckBox *straddle_;
    QCheckBox *butterfly_;
    QCheckBox *condor_;
    QCheckBox *diagonal_;
    QCheckBox *collar_;

    QGroupBox *pricing_;
    QCheckBox *theoPriceLessThanMarket_;
    QCheckBox *theoPriceGreaterThanMarket_;

    QGroupBox *volatility_;
    QCheckBox *histLessThanImpl_;
    QCheckBox *histGreaterThanImpl_;

    QPushButton *okay_;
    QPushButton *cancel_;

private slots:

    /// Slot for button clicked.
    void onButtonClicked();

    /// Slot for double spin box value changed.
    void onDoubleSpinBoxValueChanged( double value );

private:

    QString name_;

    OptionProfitCalculatorFilter f_;

    bool sized_;

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    // not implemented
    FilterEditorDialog( const _Myt& ) = delete;

    // not implemented
    FilterEditorDialog( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // FILTEREDITORDIALOG_H
