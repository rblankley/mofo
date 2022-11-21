/**
 * @file riskfreeinterestratesdialog.h
 * Dialog for showing interest rates over time.
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

#ifndef RISKFREEINTERESTRATESDIALOG_H
#define RISKFREEINTERESTRATESDIALOG_H

#include <QDialog>

class RiskFreeInterestRatesWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Dialog for showing interest rates over time.
class RiskFreeInterestRatesDialog : public QDialog
{
    Q_OBJECT

    using _Myt = RiskFreeInterestRatesDialog;
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
    RiskFreeInterestRatesDialog( QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    /// Destructor.
    virtual ~RiskFreeInterestRatesDialog();

    // ========================================================================
    // Properties
    // ========================================================================

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

private:

    static const QString STATE_GROUP_NAME;

    RiskFreeInterestRatesWidget *rates_;

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    /// Save dialog state.
    void saveState( QDialog *w ) const;

    /// Restore dialog state.
    void restoreState( QDialog *w ) const;

    // not implemented
    RiskFreeInterestRatesDialog( const _Myt& ) = delete;

    // not implemented
    RiskFreeInterestRatesDialog( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // RISKFREEINTERESTRATESDIALOG_H
