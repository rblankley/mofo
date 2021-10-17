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
    virtual QSize sizeHint() const;

    // ========================================================================
    // Methods
    // ========================================================================

    /// Translate strings.
    virtual void translate();

protected:

    QLabel *minInvestAmountLabel_;
    QDoubleSpinBox *minInvestAmount_;

    QLabel *maxInvestAmountLabel_;
    QDoubleSpinBox *maxInvestAmount_;

    QLabel *maxLossAmountLabel_;
    QDoubleSpinBox *maxLossAmount_;

    QLabel *minReturnOnInvestmentLabel_;
    QDoubleSpinBox *minReturnOnInvestment_;

    QLabel *maxReturnOnInvestmentLabel_;
    QDoubleSpinBox *maxReturnOnInvestment_;

    QLabel *minReturnOnInvestmentTimeLabel_;
    QDoubleSpinBox *minReturnOnInvestmentTime_;

    QLabel *maxReturnOnInvestmentTimeLabel_;
    QDoubleSpinBox *maxReturnOnInvestmentTime_;

    QLabel *maxSpreadPercentLabel_;
    QDoubleSpinBox *maxSpreadPercent_;

    QLabel *minVolatilityLabel_;
    QDoubleSpinBox *minVolatility_;

    QLabel *maxVolatilityLabel_;
    QDoubleSpinBox *maxVolatility_;

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

    QGroupBox *volatility_;
    QCheckBox *histLessThanImpl_;
    QCheckBox *histGreaterThanImpl_;

    QLabel *verticalDepthLabel_;
    QSpinBox *verticalDepth_;

    QPushButton *okay_;
    QPushButton *cancel_;

private slots:

    /// Slot for button clicked.
    void onButtonClicked();

private:

    QString name_;

    OptionProfitCalculatorFilter f_;

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
