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

    columnIsText_[BID_ASK_SIZE] = true;

    columnIsText_[QUOTE_TIME] = true;
    columnIsText_[TRADE_TIME] = true;

    columnIsText_[EXCHANGE_NAME] = true;

    columnIsText_[EXPIRY_DATE] = true;
    columnIsText_[EXPIRY_TYPE] = true;

    columnIsText_[LAST_TRADING_DAY] = true;

    columnIsText_[SETTLEMENT_TYPE] = true;
    columnIsText_[DELIVERABLE_NOTE] = true;

    // number of decimal places
    numDecimalPlaces_[UNDERLYING_PRICE] = 2;

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
    numDecimalPlaces_[PERCENT_CHANGE] = 2;

    numDecimalPlaces_[MARK] = 2;
    numDecimalPlaces_[MARK_CHANGE] = 2;
    numDecimalPlaces_[MARK_PERCENT_CHANGE] = 2;

    numDecimalPlaces_[VOLATILITY] = 4;
    numDecimalPlaces_[DELTA] = 4;
    numDecimalPlaces_[GAMMA] = 4;
    numDecimalPlaces_[THETA] = 4;
    numDecimalPlaces_[VEGA] = 4;
    numDecimalPlaces_[RHO] = 4;

    numDecimalPlaces_[TIME_VALUE] = 2;
    numDecimalPlaces_[THEO_OPTION_VALUE] = 2;
    numDecimalPlaces_[THEO_VOLATILITY] = 4;

    numDecimalPlaces_[STRIKE_PRICE] = 2;

    numDecimalPlaces_[HIST_VOLATILITY] = 4;
    numDecimalPlaces_[TIME_TO_EXPIRY] = 4;
    numDecimalPlaces_[RISK_FREE_INTEREST_RATE] = 4;

    numDecimalPlaces_[DIV_AMOUNT] = 2;
    numDecimalPlaces_[DIV_YIELD] = 2;

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
    numDecimalPlaces_[BID_ASK_SPREAD_PERCENT] = 2;

    numDecimalPlaces_[PROBABILITY_ITM] = 2;
    numDecimalPlaces_[PROBABILITY_OTM] = 2;
    numDecimalPlaces_[PROBABILITY_PROFIT] = 2;

    numDecimalPlaces_[INVESTMENT_OPTION_PRICE] = 2;
    numDecimalPlaces_[INVESTMENT_OPTION_PRICE_VS_THEO] = 2;

    numDecimalPlaces_[INVESTMENT_AMOUNT] = 2;
    numDecimalPlaces_[PREMIUM_AMOUNT] = 2;
    numDecimalPlaces_[MAX_GAIN] = 2;
    numDecimalPlaces_[MAX_LOSS] = 2;

    numDecimalPlaces_[ROR] = 3;
    numDecimalPlaces_[ROR_TIME] = 3;

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
QString OptionTradingItemModel::columnDescription( int col ) const
{
    switch ( (ColumnIndex) col )
    {
    case STAMP:
        return tr( "Stamp" );
    case UNDERLYING:
        return tr( "Underlying Symbol" );
    case UNDERLYING_PRICE:
        return tr( "Underlying Price" );
    case TYPE:
        return tr( "Option Type" );

    case STRATEGY:
        return tr( "Trading Strategy" );
    case STRATEGY_DESC:
        return tr( "Trading Strategy Description" );

    case SYMBOL:
        return tr( "Symbol" );
    case DESC:
        return tr( "Description" );
    case BID_ASK_SIZE:
        return tr( "Bid/Ask Size" );
    case BID_PRICE:
        return tr( "Bid Price" );
    case BID_SIZE:
        return tr( "Bid Size" );
    case ASK_PRICE:
        return tr( "Ask Price" );
    case ASK_SIZE:
        return tr( "Ask Size" );
    case LAST_PRICE:
        return tr( "Last Price" );
    case LAST_SIZE:
        return tr( "Last Size" );
    case BREAK_EVEN_PRICE:
        return tr( "Break Even Price" );
    case INTRINSIC_VALUE:
        return tr( "Intrinsic Value" );
    case OPEN_PRICE:
        return tr( "Open Price" );
    case HIGH_PRICE:
        return tr( "High Price" );
    case LOW_PRICE:
        return tr( "Low Price" );
    case CLOSE_PRICE:
        return tr( "Close Price" );
    case CHANGE:
        return tr( "Change" );
    case PERCENT_CHANGE:
        return tr( "Percent Change" );
    case TOTAL_VOLUME:
        return tr( "Volume" );
    case QUOTE_TIME:
        return tr( "Quote Time" );
    case TRADE_TIME:
        return tr( "Trade Time" );
    case MARK:
        return tr( "Mark" );
    case MARK_CHANGE:
        return tr( "Mark Change" );
    case MARK_PERCENT_CHANGE:
        return tr( "Mark Percent Change" );
    case EXCHANGE_NAME:
        return tr( "Exchange" );
    case VOLATILITY:
        return tr( "Volatility" );
    case DELTA:
        return tr( "Delta" );
    case GAMMA:
        return tr( "Gamma" );
    case THETA:
        return tr( "Theta" );
    case VEGA:
        return tr( "Vega" );
    case RHO:
        return tr( "Rho" );
    case TIME_VALUE:
        return tr( "Time Value" );
    case OPEN_INTEREST:
        return tr( "Open Interest" );
    case IS_IN_THE_MONEY:
        return tr( "In The Money" );
    case IS_OUT_OF_THE_MONEY:
        return tr( "Out of The Money" );
    case THEO_OPTION_VALUE:
        return tr( "Theoretical Option Value" );
    case THEO_VOLATILITY:
        return tr( "Theoretical Volatility" );
    case IS_MINI:
        return tr( "Is Mini" );
    case IS_NON_STANDARD:
        return tr( "Is Non-Standard" );
    case IS_INDEX:
        return tr( "Is Index" );
    case IS_WEEKLY:
        return tr( "Is Weekly" );
    case IS_QUARTERLY:
        return tr( "Is Quarterly" );
    case EXPIRY_DATE:
        return tr( "Expiration Date" );
    case EXPIRY_TYPE:
        return tr( "Expiration Type" );
    case DAYS_TO_EXPIRY:
        return tr( "Days to Expiration" );
    case LAST_TRADING_DAY:
        return tr( "Last Trading Day" );
    case MULTIPLIER:
        return tr( "Multiplier" );
    case SETTLEMENT_TYPE:
        return tr( "Settlement Type" );
    case DELIVERABLE_NOTE:
        return tr( "Deliverable Note" );
    case STRIKE_PRICE:
        return tr( "Strike Price" );

    case HIST_VOLATILITY:
        return tr( "Historical Volatility" );

    case TIME_TO_EXPIRY:
        return tr( "Time to Expiration (Years)" );
    case RISK_FREE_INTEREST_RATE:
        return tr( "Risk Free Interest Rate Percent" );

    case DIV_AMOUNT:
        return tr( "Dividend Amount - Expected" );
    case DIV_YIELD:
        return tr( "Dividend Yield - Expected" );

    case CALC_BID_PRICE_VI:
        return tr( "Bid Price Volatility - Calculated" );
    case CALC_ASK_PRICE_VI:
        return tr( "Ask Price Volatility - Calculated" );
    case CALC_MARK_VI:
        return tr( "Mark Volatility - Calculated" );

    case CALC_THEO_OPTION_VALUE:
        return tr( "Theoretical Option Value - Calculated" );
    case CALC_THEO_VOLATILITY:
        return tr( "Theoretical Volatility - Calculated" );
    case CALC_DELTA:
        return tr( "Delta - Calculated" );
    case CALC_GAMMA:
        return tr( "Gamma - Calculated" );
    case CALC_THETA:
        return tr( "Theta - Calculated" );
    case CALC_VEGA:
        return tr( "Vega - Calculated" );
    case CALC_RHO:
        return tr( "Rho - Calculated" );

    case BID_ASK_SPREAD:
        return tr( "Bid/Ask Spread Amount" );
    case BID_ASK_SPREAD_PERCENT:
        return tr( "Bid/Ask Spread Percent" );

    case PROBABILITY_ITM:
        return tr( "Probability In The Money" );
    case PROBABILITY_OTM:
        return tr( "Probability Out of The Money" );
    case PROBABILITY_PROFIT:
        return tr( "Probability of Profit" );

    case INVESTMENT_OPTION_PRICE:
        return tr( "Market Option Value" );
    case INVESTMENT_OPTION_PRICE_VS_THEO:
        return tr( "Market Option Value versus Theoretical Option Value" );

    case INVESTMENT_AMOUNT:
        return tr( "Investment Amount" );
    case PREMIUM_AMOUNT:
        return tr( "Premium Amount" );
    case MAX_GAIN:
        return tr( "Maximum Gain" );
    case MAX_LOSS:
        return tr( "Maximum Loss" );

    case ROR:
        return tr( "Return on Risk" );
    case ROR_TIME:
        return tr( "Return on Risk / Time" );

    case ROI:
        return tr( "Return on Investment" );
    case ROI_TIME:
        return tr( "Return on Investment / Time" );

    case EXPECTED_VALUE:
        return tr( "Expected Value" );
    case EXPECTED_VALUE_ROI:
        return tr( "Expected Value Return on Investment" );
    case EXPECTED_VALUE_ROI_TIME:
        return tr( "Expected Value Return on Investment / Time" );
    default:
        break;
    }

    return QString();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Qt::ItemFlags OptionTradingItemModel::flags( const QModelIndex& index ) const
{
    // disable item
    Qt::ItemFlags f( _Mybase::flags( index ) );
    f &= ~Qt::ItemIsEnabled;

    return f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingItemModel::addRow( const ColumnValueMap& values )
{
    item_type *items( allocRowItems() );

    const bool freeMoney( values[INVESTMENT_AMOUNT].toDouble() < 0.0 );

    // populate
    for ( ColumnValueMap::const_iterator i( values.constBegin() ); i != values.constEnd(); ++i )
    {
        item_type *item( &items[i.key()] );

        // ------------
        // display role
        // ------------

        QString text;

        if ( STRATEGY == i.key() )
            text = strategyText( (Strategy) i->toInt() );

        // no bid/ask size
        else if ((( BID_PRICE == i.key() ) && ( 0 == values[BID_SIZE].toInt() )) ||
                 (( ASK_PRICE == i.key() ) && ( 0 == values[ASK_SIZE].toInt() )))
            ; // empty string

        // invalid calculated volatility
        else if ((( CALC_BID_PRICE_VI == i.key() ) || ( CALC_ASK_PRICE_VI == i.key() ) || ( CALC_MARK_VI == i.key() ) ||
                  ( CALC_THEO_VOLATILITY == i.key() )) &&
                 ( i.value().toDouble() <= 0.0 ))
            ; // empty string

        else
            text = formatValue( i.value(), numDecimalPlaces_[i.key()] );

        item->setData( text, Qt::DisplayRole );

        // -------------------
        // text alignment role
        // -------------------

        Qt::Alignment align;

        if ( columnIsText_[i.key()] )
            align = Qt::AlignLeft | Qt::AlignVCenter;
        else
            align = Qt::AlignRight | Qt::AlignVCenter;

        item->setData( QVariant( align ), Qt::TextAlignmentRole );

        // ---------------
        // background role
        // ---------------

        if ( values[IS_IN_THE_MONEY].toBool() )
        {
            if ( values[IS_OUT_OF_THE_MONEY].toBool() )
                item->setData( QVariant( mixedMoneyColor_ ), Qt::BackgroundRole );
            else
                item->setData( QVariant( inTheMoneyColor_ ), Qt::BackgroundRole );
        }

        // ---------------
        // foreground role
        // ---------------

        double v;

        item->setData( textColor_, Qt::ForegroundRole );

        switch ( i.key() )
        {
        case CALC_THEO_OPTION_VALUE:
            item->setData( calcErrorColor( values[THEO_OPTION_VALUE], values[CALC_THEO_OPTION_VALUE], textColor_ ), Qt::ForegroundRole );
            break;
        case CALC_THEO_VOLATILITY:
            item->setData( calcErrorColor( values[VOLATILITY], values[CALC_THEO_VOLATILITY], textColor_ ), Qt::ForegroundRole );
            break;
        case CALC_DELTA:
            item->setData( calcErrorColor( values[DELTA], values[CALC_DELTA], textColor_ ), Qt::ForegroundRole );
            break;
        case CALC_GAMMA:
            item->setData( calcErrorColor( values[GAMMA], values[CALC_GAMMA], textColor_ ), Qt::ForegroundRole );
            break;
        case CALC_THETA:
            item->setData( calcErrorColor( values[THETA], values[CALC_THETA], textColor_ ), Qt::ForegroundRole );
            break;
        case CALC_VEGA:
            item->setData( calcErrorColor( values[VEGA], values[CALC_VEGA], textColor_ ), Qt::ForegroundRole );
            break;
        case CALC_RHO:
            item->setData( calcErrorColor( values[RHO], values[CALC_RHO], textColor_ ), Qt::ForegroundRole );
            break;

        case INVESTMENT_OPTION_PRICE:
        case INVESTMENT_OPTION_PRICE_VS_THEO:
            v = values[INVESTMENT_OPTION_PRICE_VS_THEO].toDouble();
            if ( 0.005 <= v )
                item->setData( QColor( Qt::darkGreen ), Qt::ForegroundRole );
            else if ( v < -0.005 )
                item->setData( QColor( Qt::red ), Qt::ForegroundRole );
            break;

        case INVESTMENT_AMOUNT:
        case MAX_LOSS:
            if ( values[i.key()].toDouble() < 0.0 )
                item->setData( QColor( Qt::darkGreen ), Qt::ForegroundRole );
            break;
        case PREMIUM_AMOUNT:
        case MAX_GAIN:
            if ( values[i.key()].toDouble() < 0.0 )
                item->setData( QColor( Qt::red ), Qt::ForegroundRole );
            break;

        case ROR:
        case ROR_TIME:
            if ( values[i.key()].toDouble() < 0.0 )
            {
                if ( freeMoney )
                    item->setData( QColor( Qt::darkGreen ), Qt::ForegroundRole );
                else
                    item->setData( QColor( Qt::red ), Qt::ForegroundRole );
            }
            break;

        case ROI:
        case ROI_TIME:
            if ( values[i.key()].toDouble() < 0.0 )
            {
                if ( freeMoney )
                    item->setData( QColor( Qt::darkGreen ), Qt::ForegroundRole );
                else
                    item->setData( QColor( Qt::red ), Qt::ForegroundRole );
            }
            break;

        case EXPECTED_VALUE:
            v = values[i.key()].toDouble();
            if ( 0.0 < v )
                item->setData( QColor( Qt::darkGreen ), Qt::ForegroundRole );
            else if ( v < 0.0 )
                item->setData( QColor( Qt::red ), Qt::ForegroundRole );
            break;

        case EXPECTED_VALUE_ROI:
        case EXPECTED_VALUE_ROI_TIME:
            v = values[i.key()].toDouble();
            if ( 0.0 < v )
                item->setData( QColor( Qt::darkGreen ), Qt::ForegroundRole );
            else if ( v < 0.0 )
            {
                if ( freeMoney )
                    item->setData( QColor( Qt::darkGreen ), Qt::ForegroundRole );
                else
                    item->setData( QColor( Qt::red ), Qt::ForegroundRole );
            }
            break;

        default:
            break;
        }

        // ---------
        // user role
        // ---------

        item->setData( i.value(), Qt::UserRole );
    }

    // append row!
    appendRow( items );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double OptionTradingItemModel::calcError( const QVariant& col0, const QVariant& col1, bool &valid )
{
#if QT_VERSION_CHECK( 6, 0, 0 ) <= QT_VERSION
    valid = (( QMetaType::Double == col0.typeId() ) && ( QMetaType::Double == col1.typeId() ));
#else
    valid = (( QVariant::Double == col0.type() ) && ( QVariant::Double == col1.type() ));
#endif

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
