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
    // when sorting, use user role data (raw data)
    setSortRole( Qt::UserRole );

    // text columns
    columnIsText_[STAMP] = true;
    columnIsText_[UNDERLYING] = true;
    columnIsText_[TYPE] = true;

    columnIsText_[STRATEGY] = true;
    columnIsText_[STRATEGY_DESC] = true;

    columnIsText_[SYMBOL] = true;
    columnIsText_[DESC] = true;

    columnIsText_[EXCHANGE_NAME] = true;

    columnIsText_[SETTLEMENT_TYPE] = true;
    columnIsText_[DELIVERABLE_NOTE] = true;

    // number of decimal places
    numDecimalPlaces_[BID_PRICE] = 2;
    numDecimalPlaces_[ASK_PRICE] = 2;
    numDecimalPlaces_[LAST_PRICE] = 2;

    numDecimalPlaces_[BREAK_EVEN_PRICE] = 2;
    numDecimalPlaces_[INTRINSIC_VALUE] = 2;

    numDecimalPlaces_[OPEN_PRICE] = 2;
    numDecimalPlaces_[HIGH_PRICE] = 2;
    numDecimalPlaces_[LOW_PRICE] = 2;
    numDecimalPlaces_[CLOSE_PRICE] = 2;

    numDecimalPlaces_[CHANGE] = 2;
    numDecimalPlaces_[PERCENT_CHANGE] = 1;

    numDecimalPlaces_[MARK] = 2;
    numDecimalPlaces_[MARK_CHANGE] = 2;
    numDecimalPlaces_[MARK_PERCENT_CHANGE] = 1;

    numDecimalPlaces_[VOLATILITY] = 4;
    numDecimalPlaces_[DELTA] = 4;
    numDecimalPlaces_[GAMMA] = 4;
    numDecimalPlaces_[THETA] = 4;
    numDecimalPlaces_[VEGA] = 4;
    numDecimalPlaces_[RHO] = 4;

    numDecimalPlaces_[TIME_VALUE] = 2;
    numDecimalPlaces_[THEO_OPTION_VALUE] = 2;
    numDecimalPlaces_[THEO_VOLATILITY] = 4;

    numDecimalPlaces_[HIST_VOLATILITY] = 4;
    numDecimalPlaces_[TIME_TO_EXPIRY] = 4;
    numDecimalPlaces_[RISK_FREE_INTEREST_RATE] = 4;

    numDecimalPlaces_[CALC_BID_PRICE_VI] = 4;
    numDecimalPlaces_[CALC_ASK_PRICE_VI] = 4;
    numDecimalPlaces_[CALC_MARK_VI] = 4;

    numDecimalPlaces_[CALC_THEO_OPTION_VALUE] = 2;
    numDecimalPlaces_[CALC_THEO_VOLATILITY] = 4;
    numDecimalPlaces_[CALC_DELTA] = 4;
    numDecimalPlaces_[CALC_GAMMA] = 4;
    numDecimalPlaces_[CALC_THETA] = 4;
    numDecimalPlaces_[CALC_VEGA] = 4;
    numDecimalPlaces_[CALC_RHO] = 4;

    numDecimalPlaces_[BID_ASK_SPREAD] = 2;
    numDecimalPlaces_[BID_ASK_SPREAD_PERCENT] = 1;

    numDecimalPlaces_[PROBABILITY_ITM] = 1;
    numDecimalPlaces_[PROBABILITY_OTM] = 1;
    numDecimalPlaces_[PROBABILITY_PROFIT] = 1;

    numDecimalPlaces_[INVESTMENT_OPTION_PRICE] = 2;
    numDecimalPlaces_[INVESTMENT_OPTION_PRICE_VS_THEO] = 2;

    numDecimalPlaces_[INVESTMENT_AMOUNT] = 2;
    numDecimalPlaces_[PREMIUM_AMOUNT] = 2;
    numDecimalPlaces_[MAX_GAIN] = 2;
    numDecimalPlaces_[MAX_LOSS] = 2;

    numDecimalPlaces_[ROI] = 3;
    numDecimalPlaces_[ROI_TIME] = 3;

    numDecimalPlaces_[EXPECTED_VALUE] = 2;
    numDecimalPlaces_[EXPECTED_VALUE_ROI] = 3;
    numDecimalPlaces_[EXPECTED_VALUE_ROI_TIME] = 3;

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
    QMutexLocker guard( &m_ );

    // insert!
    insertRow( 0 );

    // populate
    for ( ColumnValueMap::const_iterator i( values.constBegin() ); i != values.constEnd(); ++i )
    {
        const QModelIndex idx( index( 0, i.key() ) );

        // ------------
        // display role
        // ------------

        QString text;

        if ( STRATEGY == idx.column() )
            text = strategyText( (Strategy) i->toInt() );

        // no bid/ask size
        else if ((( BID_PRICE == idx.column() ) && ( 0 == values[BID_SIZE].toInt() )) ||
                 (( ASK_PRICE == idx.column() ) && ( 0 == values[ASK_SIZE].toInt() )))
            ; // empty string

        // invalid calculated volatility
        else if ((( CALC_BID_PRICE_VI == idx.column() ) || ( CALC_ASK_PRICE_VI == idx.column() ) || ( CALC_MARK_VI == idx.column() ) ||
                  ( CALC_THEO_VOLATILITY  == idx.column() )) &&
                 ( i.value().toDouble() <= 0.0 ))
            ; // empty string

        else
            text = formatValue( i.value(), numDecimalPlaces_[i.key()] );

        setData( idx, text, Qt::DisplayRole );

        // -------------------
        // text alignment role
        // -------------------

        Qt::Alignment align;

        if ( columnIsText_[idx.column()] )
            align = Qt::AlignLeft | Qt::AlignVCenter;
        else
            align = Qt::AlignRight | Qt::AlignVCenter;

        setData( idx, QVariant( align ), Qt::TextAlignmentRole );

        // ---------------
        // background role
        // ---------------

        if ( values[IS_IN_THE_MONEY].toBool() )
        {
            if ( values[IS_OUT_OF_THE_MONEY].toBool() )
                setData( idx, QVariant( mixedMoneyColor_ ), Qt::BackgroundRole );
            else
                setData( idx, QVariant( inTheMoneyColor_ ), Qt::BackgroundRole );
        }

        // ---------------
        // foreground role
        // ---------------

        double v;

        setData( idx, textColor_, Qt::ForegroundRole );

        switch ( idx.column() )
        {
        case CALC_THEO_OPTION_VALUE:
            setData( idx, calcErrorColor( values[THEO_OPTION_VALUE], values[CALC_THEO_OPTION_VALUE], textColor_ ), Qt::ForegroundRole );
            break;
        case CALC_THEO_VOLATILITY:
            setData( idx, calcErrorColor( values[VOLATILITY], values[CALC_THEO_VOLATILITY], textColor_ ), Qt::ForegroundRole );
            break;
        case CALC_DELTA:
            setData( idx, calcErrorColor( values[DELTA], values[CALC_DELTA], textColor_ ), Qt::ForegroundRole );
            break;
        case CALC_GAMMA:
            setData( idx, calcErrorColor( values[GAMMA], values[CALC_GAMMA], textColor_ ), Qt::ForegroundRole );
            break;
        case CALC_THETA:
            setData( idx, calcErrorColor( values[THETA], values[CALC_THETA], textColor_ ), Qt::ForegroundRole );
            break;
        case CALC_VEGA:
            setData( idx, calcErrorColor( values[VEGA], values[CALC_VEGA], textColor_ ), Qt::ForegroundRole );
            break;
        case CALC_RHO:
            setData( idx, calcErrorColor( values[RHO], values[CALC_RHO], textColor_ ), Qt::ForegroundRole );
            break;

        case INVESTMENT_OPTION_PRICE:
        case INVESTMENT_OPTION_PRICE_VS_THEO:
            v = values[INVESTMENT_OPTION_PRICE_VS_THEO].toDouble();
            if ( 0.0 < v )
                setData( idx, QColor( Qt::darkGreen ), Qt::ForegroundRole );
            else if ( v < 0.0 )
                setData( idx, QColor( Qt::red ), Qt::ForegroundRole );
            break;

        case INVESTMENT_AMOUNT:
        case MAX_LOSS:
            if ( values[idx.column()].toDouble() < 0.0 )
                setData( idx, QColor( Qt::darkGreen ), Qt::ForegroundRole );
            break;
        case PREMIUM_AMOUNT:
        case MAX_GAIN:
            if ( values[idx.column()].toDouble() < 0.0 )
                setData( idx, QColor( Qt::red ), Qt::ForegroundRole );
            break;

        case ROI:
        case ROI_TIME:
            if ( values[idx.column()].toDouble() < 0.0 )
            {
                if ( values[INVESTMENT_AMOUNT].toDouble() < 0.0 )
                    setData( idx, QColor( Qt::darkGreen ), Qt::ForegroundRole );
                else
                    setData( idx, QColor( Qt::red ), Qt::ForegroundRole );
            }
            break;

        case EXPECTED_VALUE:
            v = values[idx.column()].toDouble();
            if ( 0.0 < v )
                setData( idx, QColor( Qt::darkGreen ), Qt::ForegroundRole );
            else if ( v < 0.0 )
                setData( idx, QColor( Qt::red ), Qt::ForegroundRole );
            break;

        case EXPECTED_VALUE_ROI:
        case EXPECTED_VALUE_ROI_TIME:
            v = values[idx.column()].toDouble();
            if ( 0.0 < v )
                setData( idx, QColor( Qt::darkGreen ), Qt::ForegroundRole );
            else if ( v < 0.0 )
            {
                if ( values[INVESTMENT_AMOUNT].toDouble() < 0.0 )
                    setData( idx, QColor( Qt::darkGreen ), Qt::ForegroundRole );
                else
                    setData( idx, QColor( Qt::red ), Qt::ForegroundRole );
            }
            break;

        default:
            break;
        }

        // ---------
        // user role
        // ---------

        setData( idx, i.value(), Qt::UserRole );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double OptionTradingItemModel::calcError( const QVariant& col0, const QVariant& col1, bool &valid )
{
    valid = (( QVariant::Double == col0.type() ) && ( QVariant::Double == col1.type() ));

    if ( !valid )
        return 0.0;

    return std::fabs( (col1.toDouble() - col0.toDouble()) / col0.toDouble() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QColor OptionTradingItemModel::calcErrorColor( const QVariant& col0, const QVariant& col1, const QColor& orig )
{
    bool valid;
    const double e( calcError( col0, col1, valid ) );

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
