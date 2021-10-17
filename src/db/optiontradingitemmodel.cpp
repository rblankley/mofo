/**
 * @file optiontradingitemmodel.cpp
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

#include "optiontradingitemmodel.h"

#include <cmath>

#include <QDate>
#include <QLocale>
#include <QPalette>

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionTradingItemModel::OptionTradingItemModel( QObject *parent ) :
    _Mybase( 0, _NUM_COLUMNS, parent )
{
    // generate column is currency
    columnIsCurrency_[BID_PRICE] = true;
    columnIsCurrency_[ASK_PRICE] = true;
    columnIsCurrency_[LAST_PRICE] = true;

    columnIsCurrency_[BREAK_EVEN_PRICE] = true;
    columnIsCurrency_[INTRINSIC_VALUE] = true;

    columnIsCurrency_[OPEN_PRICE] = true;
    columnIsCurrency_[HIGH_PRICE] = true;
    columnIsCurrency_[LOW_PRICE] = true;
    columnIsCurrency_[CLOSE_PRICE] = true;

    columnIsCurrency_[CHANGE] = true;

    columnIsCurrency_[MARK] = true;
    columnIsCurrency_[MARK_CHANGE] = true;

    columnIsCurrency_[TIME_VALUE] = true;
    columnIsCurrency_[THEO_OPTION_VALUE] = true;

    columnIsCurrency_[CALC_THEO_OPTION_VALUE] = true;

    columnIsCurrency_[BID_ASK_SPREAD] = true;

    columnIsCurrency_[INVESTMENT_OPTION_PRICE] = true;
    columnIsCurrency_[INVESTMENT_OPTION_PRICE_VS_THEO] = true;

    columnIsCurrency_[INVESTMENT_VALUE] = true;
    columnIsCurrency_[MAX_GAIN] = true;
    columnIsCurrency_[MAX_LOSS] = true;

    columnIsCurrency_[EXPECTED_VALUE] = true;

    columnIsCurrency_[STRIKE_PRICE] = true;

    // color of money!!!!
    inTheMoneyColor_ = Qt::green;
    inTheMoneyColor_.setAlpha( 32 );

    mixedMoneyColor_ = Qt::yellow;
    mixedMoneyColor_.setAlpha( 32 );

    const QPalette p;

    textColor_ = p.color( QPalette::Active, QPalette::Text );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionTradingItemModel::~OptionTradingItemModel()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant OptionTradingItemModel::data( const QModelIndex& index, int role ) const
{
    QMutexLocker guard( &m_ );

    if ( Qt::DisplayRole == role )
    {
        if ( STRATEGY == index.column() )
            return strategyText( (Strategy) _Mybase::data( index, role ).toInt() );
    }
    else if ( Qt::TextAlignmentRole == role )
    {
        switch ( index.column() )
        {
        case UNDERLYING:
        case TYPE:
        case STRATEGY:
        case STRATEGY_DESC:
        case SYMBOL:
        case EXPIRY_DATE:
            return QVariant( Qt::AlignLeft | Qt::AlignVCenter );
        default:
            break;
        }

        return QVariant( Qt::AlignRight | Qt::AlignVCenter );
    }
    else if ( Qt::BackgroundRole == role )
    {
        if ( _Mybase::data( index.row(), IS_IN_THE_MONEY ).toBool() )
        {
            if ( _Mybase::data( index.row(), IS_OUT_OF_THE_MONEY ).toBool() )
                return QVariant( mixedMoneyColor_ );

            return QVariant( inTheMoneyColor_ );
        }
    }
    else if ( Qt::ForegroundRole == role )
    {
        switch ( index.column() )
        {
        case CALC_THEO_OPTION_VALUE:
            return calcErrorColor( index.row(), THEO_OPTION_VALUE, CALC_THEO_OPTION_VALUE, textColor_ );

        case CALC_THEO_VOLATILITY:
            return calcErrorColor( index.row(), VOLATILITY, CALC_THEO_VOLATILITY, textColor_ );

        case CALC_DELTA:
            return calcErrorColor( index.row(), DELTA, CALC_DELTA, textColor_ );

        case CALC_GAMMA:
            return calcErrorColor( index.row(), GAMMA, CALC_GAMMA, textColor_ );

        case CALC_THETA:
            return calcErrorColor( index.row(), THETA, CALC_THETA, textColor_ );

        case CALC_VEGA:
            return calcErrorColor( index.row(), VEGA, CALC_VEGA, textColor_ );

        case CALC_RHO:
            return calcErrorColor( index.row(), RHO, CALC_RHO, textColor_ );

        case INVESTMENT_OPTION_PRICE:
            if ( 0.0 < _Mybase::data( index.row(), INVESTMENT_OPTION_PRICE_VS_THEO ).toDouble() )
                return QVariant( QColor( Qt::green ) );
            else if ( _Mybase::data( index.row(), INVESTMENT_OPTION_PRICE_VS_THEO ).toDouble() < 0.0 )
                return QVariant( QColor( Qt::red ) );

            break;

        case INVESTMENT_OPTION_PRICE_VS_THEO:
        case ROI:
        case ROI_TIME:
        case EXPECTED_VALUE:
        case EXPECTED_VALUE_ROI:
        case EXPECTED_VALUE_ROI_TIME:
            if ( 0.0 < _Mybase::data( index.row(), index.column() ).toDouble() )
                return QVariant( QColor( Qt::green ) );
            else if ( _Mybase::data( index.row(), index.column() ).toDouble() < 0.0 )
                return QVariant( QColor( Qt::red ) );

            break;

        default:
            break;
        }

        return QVariant( textColor_ );
    }

    return _Mybase::data( index, role );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Qt::ItemFlags OptionTradingItemModel::flags( const QModelIndex& index ) const
{
    QMutexLocker guard( &m_ );

    // disable item
    Qt::ItemFlags f( _Mybase::flags( index ) );
    f &= ~Qt::ItemIsEnabled;

    return f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingItemModel::addRow( const ColumnValueMap& values )
{
/*
    const double roiTime( values[EXPECTED_ROI_TIME].toDouble() );
*/
    QMutexLocker guard( &m_ );

    // find insertion row
    int row( 0 );
/*
    while ( row < rowCount() )
    {
        const QModelIndex idx( index( row, EXPECTED_ROI_TIME ) );

        if ( data( idx ).toDouble() <= roiTime )
            break;

        ++row;
    }
*/
    // insert!
    insertRow( row );

    // populate
    for ( ColumnValueMap::const_iterator i( values.constBegin() ); i != values.constEnd(); ++i )
    {
        const QModelIndex idx( index( row, i.key() ) );
        setData( idx, formatValue( i.value(), columnIsCurrency_[i.key()] ), Qt::DisplayRole );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString OptionTradingItemModel::formatValue( const QVariant& v, bool isCurrency )
{
    static const QString invalidNumber( "NaN" );

    if ( QVariant::String == v.type() )
    {
        const QString result( v.toString() );

        if ( invalidNumber == result )
            return QString();

        return result;
    }
    else if ( QVariant::Date == v.type() )
    {
        const QDate result( v.toDate() );

        return result.toString();
    }

    const double doubleValue( v.toDouble() );

    if ( isCurrency )
        return QString::number( doubleValue, 'f', 2 );

    // check for integer
    const qlonglong intValue( v.toLongLong() );

    if ( doubleValue == (double) intValue )
    {
        const QLocale l( QLocale::system() );
        return l.toString( intValue );
    }

    return QString::number( doubleValue );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double OptionTradingItemModel::calcError( int row, ColumnIndex col0, ColumnIndex col1, bool &valid ) const
{
    const QVariant v0( data( row, col0 ) );
    const QVariant v1( data( row, col1 ) );

    if ( !(valid = (( v0.isValid() ) && ( v1.isValid() ))) )
        return 0.0;

    return std::fabs( (v1.toDouble() - v0.toDouble()) / v0.toDouble() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QColor OptionTradingItemModel::calcErrorColor( int row, ColumnIndex col0, ColumnIndex col1, const QColor& orig ) const
{
    bool valid;
    const double e( calcError( row, col0, col1, valid ) );

    if ( 0.50 < e )
        return Qt::darkRed;
    else if ( 0.20 < e )
        return Qt::red;
    else if ( 0.10 < e )
        return QColor( 255, 165, 0 ); // orange

    return orig;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString OptionTradingItemModel::strategyText( Strategy strat )
{
    switch ( strat )
    {
    case SINGLE:
        return tr( "SINGLE" );
    case VERT_BULL_PUT:
        return tr( "VERTICAL BULL PUT" );
    case VERT_BEAR_CALL:
        return tr( "VERTICAL BEAR CALL" );
    default:
        break;
    }

    return QString();
}
