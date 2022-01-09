/**
 * @file fundamentalstablemodel.h
 * Table model for symbol fundamentals.
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

#ifndef FUNDAMENTALSTABLEMODEL_H
#define FUNDAMENTALSTABLEMODEL_H

#include "sqltablemodel.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Table model for for symbol fundamentals.
class FundamentalsTableModel : public SqlTableModel
{
    Q_OBJECT

    using _Myt = FundamentalsTableModel;
    using _Mybase = SqlTableModel;

public:

    /// Column index values.
    enum ColumnIndex
    {
        STAMP,
        SYMBOL,
        HIGH52,
        LOW52,
        DIV_AMOUNT,
        DIV_YIELD,
        DIV_DATE,
        DIV_FREQUENCY,
        PE_RATIO,
        PEG_RATIO,
        PB_RATIO,
        PR_RATIO,
        PCF_RATIO,
        GROSS_MARGIN_TTM,
        GROSS_MARGIN_MRQ,
        NET_PROFIT_MARGIN_TTM,
        NET_PROFIT_MARGIN_MRQ,
        OPERATING_MARGIN_TTM,
        OPERATING_MARGIN_MRQ,
        RETURN_ON_EQUITY,
        RETURN_ON_ASSETS,
        RETURN_ON_INVESTMENT,
        QUICK_RATIO,
        CURRENT_RATIO,
        INTEREST_COVERAGE,
        TOTAL_DEBT_TO_CAPITAL,
        LT_DEBT_TO_EQUITY,
        TOTAL_DEBT_TO_EQUITY,
        EPS_TTM,
        EPS_CHANGE_PERCENT_TTM,
        EPS_CHANGE_YEAR,
        EPS_CHANGE,
        REV_CHANGE_YEAR,
        REV_CHANGE_TTM,
        REV_CHANGE_IN,
        SHARES_OUTSTANDING,
        MARKET_CAP_FLOAT,
        MARKET_CAP,
        BOOK_VALUE_PER_SHARE,
        SHORT_INT_TO_FLOAT,
        SHORT_INT_DAY_TO_COVER,
        DIV_GROWTH_RATE_3YEAR,
        DIV_PAY_AMOUNT,
        DIV_PAY_DATE,
        BETA,
        VOL_1DAY_AVG,
        VOL_10DAY_AVG,
        VOL_3MONTH_AVG,

        _NUM_COLUMNS,
    };

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] symbol  symbol
     * @param[in] stamp  date time
     * @param[in] parent  parent object
     */
    FundamentalsTableModel( const QString& symbol, const QDateTime& stamp = QDateTime(), QObject *parent = nullptr );

    /// Destructor.
    virtual ~FundamentalsTableModel();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve symbol.
    /**
     * @return  symbol
     */
    virtual QString symbol() const {return symbol_;}

    /// Retrieve table data.
    /**
     * @param[in] col  column
     * @param[in] role  role
     * @return  data
     */
    virtual QVariant tableData( ColumnIndex col, int role = Qt::DisplayRole ) const {return _Mybase::data0( col, role );}

protected:

    QString symbol_;                                ///< Underlying symbol.

private:

    // not implemented
    FundamentalsTableModel( const _Myt& ) = delete;

    // not implemented
    FundamentalsTableModel( const _Myt&& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // FUNDAMENTALSTABLEMODEL_H
