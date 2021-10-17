/**
 * @file optionprofitcalcfilter.cpp
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

#include "optionprofitcalcfilter.h"

#include <common.h>

#include <QJsonDocument>
#include <QJsonObject>

static const QString JSON_MIN_INVEST_AMOUNT( "minInvestAmount" );
static const QString JSON_MAX_INVEST_AMOUNT( "maxInvestAmount" );

static const QString JSON_MAX_LOSS_AMOUNT( "maxLossAmount" );

static const QString JSON_MIN_ROI( "minReturnOnInvestment" );
static const QString JSON_MAX_ROI( "maxReturnOnInvestment" );

static const QString JSON_MIN_ROI_TIME( "minReturnOnInvestmentTime" );
static const QString JSON_MAX_ROI_TIME( "maxReturnOnInvestmentTime" );

static const QString JSON_MAX_SPREAD_PERCENT( "maxSpreadPercent" );

static const QString JSON_MIN_VI( "minVolatility" );
static const QString JSON_MAX_VI( "maxVolatility" );

static const QString JSON_OPTION_TYPES( "optionTypes" );
static const QString JSON_OPTION_TRADING_STRATS( "optionTradingStrats" );

static const QString JSON_VOLATILITY( "volatility" );

static const QString JSON_VERT_DEPTH( "vertDepth" );

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionProfitCalculatorFilter::OptionProfitCalculatorFilter() :
    minInvestAmount_( 0.0 ),
    maxInvestAmount_( 0.0 ),
    maxLossAmount_( 0.0 ),
    minReturnOnInvestment_( 0.0 ),
    maxReturnOnInvestment_( 0.0 ),
    minReturnOnInvestmentTime_( 0.0 ),
    maxReturnOnInvestmentTime_( 0.0 ),
    maxSpreadPercent_( 0.0 ),
    minVolatility_( 0.0 ),
    maxVolatility_( 0.0 ),
    optionTypes_( ALL_OPTION_TYPES ),
    optionTradingStrats_( ALL_STRATEGIES ),
    volatility_( ALL_VOLATILITY ),
    vertDepth_( DEFAULT_VERT_DEPTH )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionProfitCalculatorFilter::OptionProfitCalculatorFilter( const QByteArray& state ) :
    OptionProfitCalculatorFilter()
{
    restoreState( state );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionProfitCalculatorFilter::~OptionProfitCalculatorFilter()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QByteArray OptionProfitCalculatorFilter::saveState() const
{
    QJsonObject obj;
    obj[JSON_MIN_INVEST_AMOUNT] = minInvestAmount_;
    obj[JSON_MAX_INVEST_AMOUNT] = maxInvestAmount_;
    obj[JSON_MAX_LOSS_AMOUNT] = maxLossAmount_;
    obj[JSON_MIN_ROI] = minReturnOnInvestment_;
    obj[JSON_MAX_ROI] = maxReturnOnInvestment_;
    obj[JSON_MIN_ROI_TIME] = minReturnOnInvestmentTime_;
    obj[JSON_MAX_ROI_TIME] = maxReturnOnInvestmentTime_;
    obj[JSON_MAX_SPREAD_PERCENT] = maxSpreadPercent_;
    obj[JSON_MIN_VI] = minVolatility_;
    obj[JSON_MAX_VI] = maxVolatility_;
    obj[JSON_OPTION_TYPES] = optionTypes_;
    obj[JSON_OPTION_TRADING_STRATS] = optionTradingStrats_;
    obj[JSON_VOLATILITY] = volatility_;
    obj[JSON_VERT_DEPTH] = vertDepth_;

    const QJsonDocument doc( obj );

    return doc.toJson( QJsonDocument::Compact );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionProfitCalculatorFilter::restoreState( const QByteArray& state )
{
    QJsonParseError e;

    const QJsonDocument doc( QJsonDocument::fromJson( state, &e ) );

    // check for error parsing state
    if ( QJsonParseError::NoError != e.error )
    {
        LOG_WARN << "error parsing state " << e.error << " " << qPrintable( e.errorString() ) << " offset " << e.offset;
        return;
    }
    else if ( !doc.isObject() )
    {
        LOG_WARN << "document not an object";
        return;
    }

    // restore member variables
    const QJsonObject obj( doc.object() );

    if ( obj.contains( JSON_MIN_INVEST_AMOUNT ) )
        minInvestAmount_ = obj[JSON_MIN_INVEST_AMOUNT].toDouble();
    if ( obj.contains( JSON_MAX_INVEST_AMOUNT ) )
        maxInvestAmount_ = obj[JSON_MAX_INVEST_AMOUNT].toDouble();

    if ( obj.contains( JSON_MAX_LOSS_AMOUNT ) )
        maxLossAmount_ = obj[JSON_MAX_LOSS_AMOUNT].toDouble();

    if ( obj.contains( JSON_MIN_ROI ) )
        minReturnOnInvestment_ = obj[JSON_MIN_ROI].toDouble();
    if ( obj.contains( JSON_MAX_ROI ) )
        maxReturnOnInvestment_ = obj[JSON_MAX_ROI].toDouble();

    if ( obj.contains( JSON_MIN_ROI_TIME ) )
        minReturnOnInvestmentTime_ = obj[JSON_MIN_ROI_TIME].toDouble();
    if ( obj.contains( JSON_MAX_ROI_TIME ) )
        maxReturnOnInvestmentTime_ = obj[JSON_MAX_ROI_TIME].toDouble();

    if ( obj.contains( JSON_MAX_SPREAD_PERCENT ) )
        maxSpreadPercent_ = obj[JSON_MAX_SPREAD_PERCENT].toDouble();

    if ( obj.contains( JSON_MIN_VI ) )
        minVolatility_ = obj[JSON_MIN_VI].toDouble();
    if ( obj.contains( JSON_MAX_VI ) )
        maxVolatility_ = obj[JSON_MAX_VI].toDouble();

    if ( obj.contains( JSON_OPTION_TYPES ) )
        optionTypes_ = (OptionTypeFilter) obj[JSON_OPTION_TYPES].toInt();
    if ( obj.contains( JSON_OPTION_TRADING_STRATS ) )
        optionTradingStrats_ = (OptionTradingStrategyFilter) obj[JSON_OPTION_TRADING_STRATS].toInt();

    if ( obj.contains( JSON_VOLATILITY ) )
        volatility_ = (VolatilityFilter) obj[JSON_VOLATILITY].toInt();

    if ( obj.contains( JSON_VERT_DEPTH ) )
        vertDepth_ = obj[JSON_VERT_DEPTH].toInt();
}
