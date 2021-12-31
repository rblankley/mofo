/**
 * @file symboldetailsdialog.h
 * Dialog for showing symbol details.
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

#ifndef SYMBOLDETAILSDIALOG_H
#define SYMBOLDETAILSDIALOG_H

#include <QDialog>

class SymbolPriceHistoryWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Dialog for showing symbol details.
class SymbolDetailsDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY( QString symbol READ symbol )

    using _Myt = SymbolDetailsDialog;
    using _Mybase = QDialog;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in,out] symbol  symbol
     * @param[in,out] parent  parent widget
     * @param[in] f  window flags
     */
    SymbolDetailsDialog( const QString symbol, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve size hint.
    /**
     * @return  size hint
     */
    virtual QSize sizeHint() const;

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

private:

    QString symbol_;

    SymbolPriceHistoryWidget *priceHistory_;

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    // not implemented
    SymbolDetailsDialog( const _Myt& ) = delete;

    // not implemented
    SymbolDetailsDialog( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // SYMBOLDETAILSDIALOG_H
