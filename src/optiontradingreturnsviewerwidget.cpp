/**
 * @file optiontradingreturnsviewerwidget.cpp
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

#include "common.h"
#include "optiontradingreturnsviewerwidget.h"

#include "db/optiontradingitemmodel.h"

#include <cmath>

#include <QFormLayout>
#include <QLabel>
#include <QLocale>
#include <QSpacerItem>
#include <QVBoxLayout>

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionTradingReturnsViewerWidget::OptionTradingReturnsViewerWidget( int index, model_type *model, QWidget *parent ) :
    _Mybase( parent ),
    model_( model ),
    index_( index )
{
    // grab model data
    underlying_ = modelData( model_type::UNDERLYING ).toString();
    underlyingPrice_ = modelData( model_type::UNDERLYING_PRICE ).toDouble();

    strat_ = modelData( model_type::STRATEGY ).toInt();

    // single
    shortStrikePrice_ = modelData( model_type::STRIKE_PRICE ).toDouble();
    longStrikePrice_ = 0.0;

    // spread
    if (( model_type::VERT_BULL_PUT == strat_ ) || ( model_type::VERT_BEAR_CALL == strat_ ))
    {
        // format is "short/long"
        const QStringList strikes( modelData( model_type::STRIKE_PRICE ).toString().split( "/" ) );

        if ( 2 == strikes.size() )
        {
            shortStrikePrice_ = strikes[0].toDouble();
            longStrikePrice_ = strikes[1].toDouble();
        }
    }

    isCall_ = modelData( model_type::TYPE ).toString().contains( "CALL", Qt::CaseInsensitive );

    // init
    initialize();
    createLayout();
    translate();

    // refresh data
    refreshData();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionTradingReturnsViewerWidget::~OptionTradingReturnsViewerWidget()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingReturnsViewerWidget::translate()
{
    bidPriceLabel_->setText( tr( "Bid Price" ) );
    askPriceLabel_->setText( tr( "Ask Price" ) );
    markPriceLabel_->setText( tr( "Mark Price" ) );
    daysToExpiryLabel_->setText( tr( "Days to Expiration" ) );

    investPriceLabel_->setText( tr( "Market Price" ) );
    theoPriceLabel_->setText( tr( "Theoretical Price" ) );
    implVolLabel_->setText( tr( "Implied Volatility" ) );
    histVolLabel_->setText( tr( "Historical Volatility" ) );

    divAmountLabel_->setText( tr( "Dividend Amount" ) );
    riskFreeRateLabel_->setText( tr( "Risk Free Interest Rate" ) );

    //
    // estimated returns
    //

    if (( model_type::VERT_BULL_PUT == strat_ ) || ( model_type::VERT_BEAR_CALL == strat_ ) ||
        (( model_type::SINGLE == strat_ ) && ( isCall_ )))
    {
        costOfEntryLabel_->setText( tr( "Entry Credit" ) );
    }
    else
    {
        costOfEntryLabel_->setText( tr( "Cost of Entry" ) );
    }

    maxRiskLabel_->setText( tr( "Maximum Risk" ) );
    maxReturnLabel_->setText( tr( "Maximum Return" ) );

    maxReturnOnRiskLabel_->setText( tr( "Max Return on Risk" ) );
    maxReturnOnInvestLabel_->setText( tr( "Max Return on Investment" ) );
    expectedValueLabel_->setText( tr( "Expected Value" ) );

    breakevenLabel_->setText( tr( "Breakeven at Expiration" ) );
    probProfitLabel_->setText( tr( "Probability of Profit" ) );

    //
    // greeks
    //

    deltaLabel_->setText( tr( "Delta" ) );
    gammaLabel_->setText( tr( "Gamma" ) );
    thetaLabel_->setText( tr( "Theta" ) );
    vegaLabel_->setText( tr( "Vega" ) );
    rhoLabel_->setText( tr( "Rho" ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingReturnsViewerWidget::refreshData()
{
    if ( !model_->rowCount() )
        return;

    const QLocale l( QLocale::system() );

    // ---- //

    setLabelText( bidPrice_,
        l.toString( modelData( model_type::BID_PRICE ).toDouble(), 'f', 2 ),
        "-" );

    setLabelText( askPrice_,
        l.toString( modelData( model_type::ASK_PRICE ).toDouble(), 'f', 2 ),
        "-" );

    setLabelText( markPrice_,
        l.toString( modelData( model_type::MARK ).toDouble(), 'f', 3 ),
        "-" );

    setLabelText( daysToExpiry_,
        l.toString( modelData( model_type::DAYS_TO_EXPIRY ).toInt() ),
        "-" );

    setLabelText( investPrice_,
        l.toString( modelData( model_type::INVESTMENT_OPTION_PRICE ).toDouble(), 'f', 2 ),
        "-" );

    setLabelText( theoPrice_,
        l.toString( modelData( model_type::CALC_THEO_OPTION_VALUE ).toDouble(), 'f', 2 ),
        "-" );

    setLabelText( implVol_,
        l.toString( modelData( model_type::CALC_THEO_VOLATILITY ).toDouble(), 'f', 4 ),
        "-" );

    // red - volatility < historic
    // green - volatility >= historic
    setLabelColor( implVol_, (modelData( model_type::HIST_VOLATILITY ).toDouble() <= modelData( model_type::CALC_THEO_VOLATILITY ).toDouble()) ? Qt::darkGreen : Qt::red );

    setLabelText( histVol_,
        l.toString( modelData( model_type::HIST_VOLATILITY ).toDouble(), 'f', 4 ),
        "-" );

    setLabelText( divAmount_,
        QString( "%0 (%1%)" )
            .arg( l.toString( modelData( model_type::DIV_AMOUNT ).toDouble(), 'f', 2 ) )
            .arg( l.toString( modelData( model_type::DIV_YIELD ).toDouble(), 'f', 2 ) ),
        "0.00 (0.00%)" );

    setLabelText( riskFreeRate_,
        QString( "%0%" )
            .arg( l.toString( modelData( model_type::RISK_FREE_INTEREST_RATE ).toDouble(), 'f', 2 ) ),
        "0.00%" );

    //
    // estimated returns
    //

    if (( model_type::VERT_BULL_PUT == strat_ ) || ( model_type::VERT_BEAR_CALL == strat_ ) ||
        (( model_type::SINGLE == strat_ ) && ( isCall_ )))
    {
        setLabelText( costOfEntry_,
            l.toString( modelData( model_type::PREMIUM_AMOUNT ).toDouble(), 'f', 2 ),
            "-" );

        setLabelColor( costOfEntry_, (0.0 < modelData( model_type::PREMIUM_AMOUNT ).toDouble()) ? Qt::darkGreen : Qt::red );
    }
    else
    {
        setLabelText( costOfEntry_,
            l.toString( modelData( model_type::INVESTMENT_AMOUNT ).toDouble(), 'f', 2 ),
            "-" );
    }

    setLabelText( maxRisk_,
        QString( "%0 (at %1 %2)" )
            .arg( l.toString( modelData( model_type::MAX_LOSS ).toDouble(), 'f', 2 ) )
            .arg( underlying_ )
            .arg( l.toString( longStrikePrice_, 'f', 2 ) ),
        "-" );

    setLabelText( maxReturn_,
        QString( "%0 (at %1 %2)" )
            .arg( l.toString( modelData( model_type::MAX_GAIN ).toDouble(), 'f', 2 ) )
            .arg( underlying_ )
            .arg( l.toString( shortStrikePrice_, 'f', 2 ) ),
        "-" );

    setLabelColor( maxReturn_, (0.0 < modelData( model_type::MAX_GAIN ).toDouble()) ? Qt::darkGreen : Qt::red );

    setLabelText( maxReturnOnRisk_,
        QString( "%0 (%1% /wk, %2% /yr)" )
            .arg( l.toString( modelData( model_type::ROR ).toDouble(), 'f', 2 ) )
            .arg( l.toString( modelData( model_type::ROR_WEEK ).toDouble(), 'f', 3 ) )
            .arg( l.toString( modelData( model_type::ROR_YEAR ).toDouble(), 'f', 3 ) ),
        "-" );

    setLabelColor( maxReturnOnRisk_, (0.0 < modelData( model_type::ROR ).toDouble()) ? Qt::darkGreen : Qt::red );

    setLabelText( maxReturnOnInvest_,
        QString( "%0 (%1% /wk, %2% /yr)" )
            .arg( l.toString( modelData( model_type::ROI ).toDouble(), 'f', 2 ) )
            .arg( l.toString( modelData( model_type::ROI_WEEK ).toDouble(), 'f', 3 ) )
            .arg( l.toString( modelData( model_type::ROI_YEAR ).toDouble(), 'f', 3 ) ),
        "-" );

    // red - lose money
    // orange - make less money than risk free investment (i.e. government bond)
    // green - make more money than risk free investment
    if ( modelData( model_type::ROI ).toDouble() < 0.0 )
        setLabelColor( maxReturnOnInvest_, Qt::red );
    else if ( modelData( model_type::ROI_YEAR ).toDouble() <= modelData( model_type::RISK_FREE_INTEREST_RATE ).toDouble() )
        setLabelColor( maxReturnOnInvest_, QColor( 255, 165, 0 ) ); // orange
    else
        setLabelColor( maxReturnOnInvest_, Qt::darkGreen );

    setLabelText( expectedValue_,
        l.toString( modelData( model_type::EXPECTED_VALUE ).toDouble(), 'f', 2 ),
        "-" );

    setLabelColor( expectedValue_, (0.0 < modelData( model_type::EXPECTED_VALUE ).toDouble()) ? Qt::darkGreen : Qt::red );

    setLabelText( breakeven_,
        l.toString( modelData( model_type::BREAK_EVEN_PRICE ).toDouble(), 'f', 2 ),
        "-" );

    setLabelText( probProfit_,
        QString( "%0%" )
            .arg( l.toString( modelData( model_type::PROBABILITY_PROFIT ).toDouble(), 'f', 2 ) ),
        "-" );

    //
    // greeks
    //

    setLabelText( delta_,
        l.toString( modelData( model_type::CALC_DELTA ).toDouble(), 'f', 4 ),
        "-" );

    setLabelText( gamma_,
        l.toString( modelData( model_type::CALC_GAMMA ).toDouble(), 'f', 4 ),
        "-" );

    setLabelText( theta_,
        l.toString( modelData( model_type::CALC_THETA ).toDouble(), 'f', 4 ),
        "-" );

    setLabelText( vega_,
        l.toString( modelData( model_type::CALC_VEGA ).toDouble(), 'f', 4 ),
        "-" );

    setLabelText( rho_,
        l.toString( modelData( model_type::CALC_RHO ).toDouble(), 'f', 4 ),
        "-" );

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingReturnsViewerWidget::initialize()
{
    bidPriceLabel_ = new QLabel( this );
    bidPrice_ = new QLabel( this );

    askPriceLabel_ = new QLabel( this );
    askPrice_ = new QLabel( this );

    markPriceLabel_ = new QLabel( this );
    markPrice_ = new QLabel( this );

    daysToExpiryLabel_ = new QLabel( this );
    daysToExpiry_ = new QLabel( this );

    investPriceLabel_ = new QLabel( this );
    investPrice_ = new QLabel( this );

    theoPriceLabel_ = new QLabel( this );
    theoPrice_ = new QLabel( this );

    implVolLabel_ = new QLabel( this );
    implVol_ = new QLabel( this );

    histVolLabel_ = new QLabel( this );
    histVol_ = new QLabel( this );

    divAmountLabel_ = new QLabel( this );
    divAmount_ = new QLabel( this );

    riskFreeRateLabel_ = new QLabel( this );
    riskFreeRate_ = new QLabel( this );

    // estimated returns
    costOfEntryLabel_ = new QLabel( this );
    costOfEntry_ = new QLabel( this );

    maxRiskLabel_ = new QLabel( this );
    maxRisk_ = new QLabel( this );

    maxReturnLabel_ = new QLabel( this );
    maxReturn_ = new QLabel( this );

    maxReturnOnRiskLabel_ = new QLabel( this );
    maxReturnOnRisk_ = new QLabel( this );

    maxReturnOnInvestLabel_ = new QLabel( this );
    maxReturnOnInvest_ = new QLabel( this );

    expectedValueLabel_ = new QLabel( this );
    expectedValue_ = new QLabel( this );

    breakevenLabel_ = new QLabel( this );
    breakeven_ = new QLabel( this );

    probProfitLabel_ = new QLabel( this );
    probProfit_ = new QLabel( this );

    // greeks
    deltaLabel_ = new QLabel( this );
    delta_ = new QLabel( this );

    gammaLabel_ = new QLabel( this );
    gamma_ = new QLabel( this );

    thetaLabel_ = new QLabel( this );
    theta_ = new QLabel( this );

    vegaLabel_ = new QLabel( this );
    vega_ = new QLabel( this );

    rhoLabel_ = new QLabel( this );
    rho_ = new QLabel( this );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingReturnsViewerWidget::createLayout()
{
    QFormLayout *fields( new QFormLayout() );
    fields->setContentsMargins( QMargins() );

    fields->addRow( bidPriceLabel_, bidPrice_ );
    fields->addRow( askPriceLabel_, askPrice_ );
    fields->addRow( markPriceLabel_, markPrice_ );
    fields->addRow( daysToExpiryLabel_, daysToExpiry_ );

    fields->addItem( new QSpacerItem( 16, 16 ) );
    fields->addRow( investPriceLabel_, investPrice_ );
    fields->addRow( theoPriceLabel_, theoPrice_ );
    fields->addRow( implVolLabel_, implVol_ );
    fields->addRow( histVolLabel_, histVol_ );

    fields->addItem( new QSpacerItem( 16, 16 ) );
    fields->addRow( divAmountLabel_, divAmount_ );
    fields->addRow( riskFreeRateLabel_, riskFreeRate_ );

    // estimated returns
    fields->addItem( new QSpacerItem( 16, 16 ) );
    fields->addRow( costOfEntryLabel_, costOfEntry_ );
    fields->addRow( maxRiskLabel_, maxRisk_ );
    fields->addRow( maxReturnLabel_, maxReturn_ );

    fields->addItem( new QSpacerItem( 16, 16 ) );
    fields->addRow( maxReturnOnRiskLabel_, maxReturnOnRisk_ );
    fields->addRow( maxReturnOnInvestLabel_, maxReturnOnInvest_ );
    fields->addRow( expectedValueLabel_, expectedValue_ );

    fields->addItem( new QSpacerItem( 16, 16 ) );
    fields->addRow( breakevenLabel_, breakeven_ );
    fields->addRow( probProfitLabel_, probProfit_ );

    //
    // greeks
    //
    fields->addItem( new QSpacerItem( 16, 16 ) );
    fields->addRow( deltaLabel_, delta_ );
    fields->addRow( gammaLabel_, gamma_ );
    fields->addRow( thetaLabel_, theta_ );
    fields->addRow( vegaLabel_, vega_ );
    fields->addRow( rhoLabel_, rho_ );

    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->addLayout( fields );
    form->addStretch();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant OptionTradingReturnsViewerWidget::modelData( int col ) const
{
    return model_->data( index_, col, Qt::UserRole );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingReturnsViewerWidget::setLabelColor( QLabel *l, const QColor& c )
{
    QPalette p( l->palette() );
    p.setColor( l->foregroundRole(), c );

    l->setPalette( p );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingReturnsViewerWidget::setLabelText( QLabel *l, const QString text, const QString emptyText )
{
    const bool isEmpty( text == emptyText );

    if (( isEmpty ) || ( "inf" == text ) || ( "nan" == text ))
        l->setText( "-" );
    else
        l->setText( text );

    l->setAlignment( Qt::AlignCenter );
}
