/**
 * @file configdialog.h
 * Dialog for modifying configuration values.
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

#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QJsonObject>
#include <QString>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QToolButton;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Dialog for modifying configuration values.
class ConfigurationDialog : public QDialog
{
    Q_OBJECT

    using _Myt = ConfigurationDialog;
    using _Mybase = QDialog;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in,out] parent  parent widget
     * @param[in] f  window flags
     */
    ConfigurationDialog( QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    // ========================================================================
    // Properties
    // ========================================================================

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

private slots:

    /// Slot for button clicked.
    void onButtonClicked();

private:

    QJsonObject configs_;

    // ---- //

    QLabel *equityRefreshRateLabel_;
    QLineEdit *equityRefreshRate_;

    QLabel *equityTradeCostLabel_;
    QLineEdit *equityTradeCost_;

    QLabel *equityTradeCostNonExchangeLabel_;
    QLineEdit *equityTradeCostNonExchange_;

    QLabel *equityWatchListsLabel_;
    QLineEdit *equityWatchLists_;
    QToolButton *equityWatchListsDialog_;

    QLabel *historyLabel_;
    QLineEdit *history_;

    QLabel *marketTypesLabel_;
    QLineEdit *marketTypes_;

    QLabel *numDaysLabel_;
    QLineEdit *numDays_;

    QLabel *numTradingDaysLabel_;
    QLineEdit *numTradingDays_;

    QLabel *paletteLabel_;
    QComboBox *palette_;

    QLabel *paletteHighlightLabel_;
    QLineEdit *paletteHighlight_;
    QToolButton *paletteHighlightDialog_;

    QLabel *optionChainRefreshRateLabel_;
    QLineEdit *optionChainRefreshRate_;

    QLabel *optionChainExpiryEndDateLabel_;
    QLineEdit *optionChainExpiryEndDate_;

    QLabel *optionChainWatchListsLabel_;
    QLineEdit *optionChainWatchLists_;
    QToolButton *optionChainWatchListsDialog_;

    QLabel *optionTradeCostLabel_;
    QLineEdit *optionTradeCost_;

    QLabel *optionCalcMethodLabel_;
    QComboBox *optionCalcMethod_;

    QLabel *optionAnalysisFilterLabel_;
    QComboBox *optionAnalysisFilter_;
    QToolButton *optionAnalysisFilterDialog_;

    QPushButton *okay_;
    QPushButton *cancel_;

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    /// Save to database.
    void saveForm();

    /// Check configuration value changed.
    void checkConfigChanged( const QString& config, const QString& value );

    // not implemented
    ConfigurationDialog( const _Myt& ) = delete;

    // not implemented
    ConfigurationDialog( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // CONFIGDIALOG_H
