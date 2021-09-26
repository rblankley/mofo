/**
 * @file binomialcalc.cpp
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

#include "common.h"
#include "binomialcalc.h"

#include "db/appdb.h"
#include "db/optionchaintablemodel.h"

#include "util/abstractoptionpricing.h"

#include <QObject>

///////////////////////////////////////////////////////////////////////////////////////////////////
ExpectedValueCalculator::ExpectedValueCalculator( double underlying, const table_model_type *chains, item_model_type *results ) :
    _Mybase( underlying, chains, results )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ExpectedValueCalculator::~ExpectedValueCalculator()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ExpectedValueCalculator::analyze( item_model_type::Strategy strat )
{
    // generate greeks
    if (( greeksCall_.isEmpty() ) && ( greeksPut_.isEmpty() ))
        valid_ &= generateGreeks();

    // generate probability curve
    if ( probCurve_.isEmpty() )
        valid_ &= generateProbCurve();

    if ( !valid_ )
        return;

    // ---- //

    const bool calls( ONLY_CALLS & optionTypes_ );
    const bool puts( ONLY_PUTS & optionTypes_ );

    switch ( strat )
    {
    // single
    case item_model_type::SINGLE:
        analyzeSingle( calls, puts );
        break;

    // verticals
    case item_model_type::VERT_BEAR_CALL:
        if ( calls )
            analyzeVertBearCalls();
        break;
    case item_model_type::VERT_BULL_PUT:
        if ( puts )
            analyzeVertBullPuts();
        break;

    default:
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ExpectedValueCalculator::analyzeSingle( bool call, bool put ) const
{
    // analyze!
    if ( call )
    {
        for ( int row( chains_->rowCount() ); row--; )
            analyzeSingleCall( row );
    }

    if ( put )
    {
        for ( int row( chains_->rowCount() ); row--; )
            analyzeSinglePut( row );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ExpectedValueCalculator::analyzeVertBearCalls() const
{
    // TODO
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ExpectedValueCalculator::analyzeVertBullPuts() const
{
    // analyze!
    for ( int row( 1 ); row < chains_->rowCount(); ++row )
        analyzeVertBullPut( row );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool ExpectedValueCalculator::generateGreeks()
{
    if ( !valid_ )
        return false;

    // calculate greeks
    // iterate over all options
    for ( int row( 0 ); row < chains_->rowCount(); ++row )
    {
        const double strike( chains_->tableData( row, table_model_type::STRIKE_PRICE ).toDouble() );

        if (( !generateGreeks( row, strike, true ) ) ||     // calls
            ( !generateGreeks( row, strike, false ) ))      // puts
        {
            LOG_WARN << "error generating greeks!";
            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool ExpectedValueCalculator::generateProbCurve()
{
    if ( !valid_ )
        return false;

    // separate strikes into ascending and descending lists
    QList<double> asc;
    QList<double> desc;

    for ( int row( 0 ); row < chains_->rowCount(); ++row )
    {
        const double strike( chains_->tableData( row, table_model_type::STRIKE_PRICE ).toDouble() );

        asc.append( strike );
        desc.prepend( strike );

        // populate curve with data
        if (( !generateProbCurve( strike, true ) ) ||           // call
            ( !generateProbCurve( strike, false ) ))            // put
        {
            LOG_WARN << "error generating probablity curve data!";
            return false;
        }
    }

    // adjust curve
    if (( !calcProbCurve( probCurveCall_, asc, true ) ) ||      // calls
        ( !calcProbCurve( probCurvePut_, desc, false ) ))       // puts
    {
        LOG_WARN << "error calculating probablity curve!";
        return false;
    }

    // merge call and put curve implied volatility
    foreach ( const double& strike, asc )
    {
        ProbCurve call( probCurveCall_[strike] );
        ProbCurve put( probCurvePut_[strike] );

        LOG_DEBUG << "PROB " << strike <<
            " CALL " << call.min << " " << call.max << " " << call.minvi <<  " " << call.maxvi <<
            " PUT " << put.min << " " << put.max << " " << put.minvi <<  " " << put.maxvi;

        const double minvi( qMax( call.minvi, put.minvi ) );
        const double maxvi( qMin( call.maxvi, put.maxvi ) );

        if ( maxvi < minvi )
        {
            LOG_WARN << "non-overlapping prob curve " << strike;

            if ( call.maxvi < put.minvi )
            {
                call.vi = call.maxvi;
                put.vi = put.minvi;
            }
            else
            {
                call.vi = call.minvi;
                put.vi = put.maxvi;
            }
        }
        else
        {
            const double vi( minvi + ((maxvi - minvi) / 2.0) );

            call.vi = put.vi = vi;
        }

        probCurveCall_[strike] = call;
        probCurvePut_[strike] = put;
    }

    // calculate prices
    if (( !calcProbCurvePrices( probCurveCall_, asc, true ) ) ||      // calls
        ( !calcProbCurvePrices( probCurvePut_, desc, false ) ))       // puts
    {
        LOG_WARN << "error calculating probablity curve prices!";
        return false;
    }

    // generate probabilities
    foreach ( const double& strike, asc )
    {
        ProbCurve call( probCurveCall_[strike] );
        ProbCurve put( probCurvePut_[strike] );

        LOG_DEBUG << "PROB ITM " << strike << " " << call.vi << " " << call.delta << " " << put.vi << " " << put.delta;

        double calldelta( call.delta );
        double putdelta( 1.0 + put.delta );

        if ( calldelta < 0.0 )
            calldelta = 0.0;
        else if ( 1.0 < calldelta )
            calldelta = 1.0;

        if ( putdelta < 0.0 )
            putdelta = 0.0;
        else if ( 1.0 < putdelta )
            putdelta = 1.0;

        probCurve_[strike] = (calldelta + putdelta) / 2.0;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ExpectedValueCalculator::analyzeSingleCall( int row ) const
{
    const double strike( chains_->tableData( row, table_model_type::STRIKE_PRICE ).toDouble() );

    const QString optionType( QObject::tr( "Call" ) );

    const QString description( QString( "%1 $%2 %3" )
        .arg( chains_->expirationDate().toString( "MMM dd ''yy" ) )
        .arg( strike )
        .arg( optionType ) );

    item_model_type::ColumnValueMap result;

    result[item_model_type::STRATEGY] = item_model_type::SINGLE;
    result[item_model_type::STRATEGY_DESC] = description;

    // Option Chain Information
    populateResultModel( row, strike, true, result );

    // Calculated Fields
    if ( greeksCall_.contains( strike ) )
        populateResultModelGreeks( greeksCall_[strike], result );

    if ( probCurve_.contains( strike ) )
    {
        result[item_model_type::PROBABILITY_ITM] = round4( 100.0 * probCurve_[strike] );
        result[item_model_type::PROBABILITY_OTM] = round4( 100.0 * (1.0 - probCurve_[strike]) );
    }

    // TODO

    results_->addRow( result );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ExpectedValueCalculator::analyzeSinglePut( int row ) const
{
    const double strike( chains_->tableData( row, table_model_type::STRIKE_PRICE ).toDouble() );

    const QString optionType( QObject::tr( "Put" ) );

    const QString description( QString( "%1 $%2 %3" )
        .arg( chains_->expirationDate().toString( "MMM dd ''yy" ) )
        .arg( strike )
        .arg( optionType ) );

    item_model_type::ColumnValueMap result;

    result[item_model_type::STRATEGY] = item_model_type::SINGLE;
    result[item_model_type::STRATEGY_DESC] = description;

    // Option Chain Information
    populateResultModel( row, strike, false, result );

    // Calculated Fields
    if ( greeksPut_.contains( strike ) )
        populateResultModelGreeks( greeksPut_[strike], result );

    if ( probCurve_.contains( strike ) )
    {
        result[item_model_type::PROBABILITY_ITM] = round4( 100.0 * (1.0 - probCurve_[strike]) );
        result[item_model_type::PROBABILITY_OTM] = round4( 100.0 * probCurve_[strike] );
    }

    // TODO

    results_->addRow( result );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ExpectedValueCalculator::analyzeVertBullPut( int row ) const
{
    // TODO
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool ExpectedValueCalculator::generateGreeks( int row, double strike, bool isCall )
{
    static const double SECONDS_PER_DAY = 86400;

    const QDateTime quoteTime( chains_->tableData( row, isCall ? table_model_type::CALL_QUOTE_TIME : table_model_type::PUT_QUOTE_TIME ).toDateTime() );
    const QDateTime expiryDate( chains_->tableData( row, isCall ? table_model_type::CALL_EXPIRY_DATE : table_model_type::PUT_EXPIRY_DATE ).toDateTime() );

    bool okay( false );

    if (( quoteTime.isValid() ) && ( expiryDate.isValid() ) && ( quoteTime < expiryDate ))
    {
        const double bid( chains_->tableData( row, isCall ? table_model_type::CALL_BID_PRICE : table_model_type::PUT_BID_PRICE ).toDouble() );
        const double ask( chains_->tableData( row, isCall ? table_model_type::CALL_ASK_PRICE : table_model_type::PUT_ASK_PRICE ).toDouble() );
        const double mark( chains_->tableData( row, isCall ? table_model_type::CALL_MARK : table_model_type::PUT_MARK ).toDouble() );

        const OptionType type( isCall ? OptionType::Call : OptionType::Put );

        Greeks result = {0};
        result.spread = ask - bid;
        result.spreadPercent = result.spread / bid;

        result.bid = bid;
        result.ask = ask;

        // calc expiry time in years
        result.timeToExpiry = quoteTime.secsTo( expiryDate );
        result.timeToExpiry /= SECONDS_PER_DAY;
        result.timeToExpiry /= AppDatabase::instance()->numDays();

        // get risk free interest rate
        result.riskFreeRate = AppDatabase::instance()->riskFreeRate( result.timeToExpiry );

        if ( result.riskFreeRate <= 0.0 )
            LOG_WARN << "risk free rate is zero";

        // generate VI for bid, ask, mark prices
        AbstractOptionPricing *o( createPricingMethod( underlying_, result.riskFreeRate, result.riskFreeRate, 0.0, result.timeToExpiry ) );

        result.bidvi = calcImplVol( o, type, strike, result.bid );
        result.askvi = calcImplVol( o, type, strike, result.ask );
        result.markvi = calcImplVol( o, type, strike, mark );

        // determine theo option value and VI
        double theoOptionValue( chains_->tableData( row, isCall ? table_model_type::CALL_THEO_OPTION_VALUE : table_model_type::PUT_THEO_OPTION_VALUE ).toDouble() );

        // use mark if they giving us bad value
        if ( theoOptionValue < 0.0 )
        {
            LOG_WARN << "using mark for theo option value";
            theoOptionValue = mark;
        }

        // calc!
        okay = calcGreeks( o, theoOptionValue, strike, isCall, result );

        if ( okay )
        {
            if ( isCall )
                greeksCall_[strike] = result;
            else
                greeksPut_[strike] = result;
        }

        delete o;
    }

    return okay;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool ExpectedValueCalculator::calcProbCurve( OptionProbCurve& curve, const QList<double>& direction, bool isCall ) const
{
    const OptionType type( isCall ? OptionType::Call : OptionType::Put );

    bool init( false );

    ProbCurve prev;

    // check prices are decreasing!
    foreach ( const double& strike, direction )
    {
        ProbCurve c( curve[strike] );

        if ( !init )
            init = true;
        else if (( prev.min < c.min ) || ( prev.max < c.max ))
        {
            const Greeks g( isCall ? greeksCall_[strike] : greeksPut_[strike] );

            AbstractOptionPricing *o( createPricingMethod( underlying_, g.riskFreeRate, g.riskFreeRate, 0.0, g.timeToExpiry ) );

            bool okay;

            if ( prev.min < c.min )
            {
                c.min = prev.min;
                c.minvi = calcImplVol( o, type, strike, c.min, &okay );

                if ( !okay )
                    return false;
            }

            if ( prev.max < c.max )
            {
                c.max = prev.max;
                c.maxvi = calcImplVol( o, type, strike, c.max, &okay );

                if ( !okay )
                    return false;
            }

            delete o;

            curve[strike] = c;
        }

        prev = c;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool ExpectedValueCalculator::calcProbCurvePrices( OptionProbCurve& curve, const QList<double>& direction, bool isCall )
{
    const OptionType type( isCall ? OptionType::Call : OptionType::Put );

    bool init( false );

    ProbCurve prev;

    // check prices are decreasing!
    foreach ( const double& strike, direction )
    {
        ProbCurve c( curve[strike] );

        Greeks g( isCall ? greeksCall_[strike] : greeksPut_[strike] );

        AbstractOptionPricing *o( createPricingMethod( underlying_, g.riskFreeRate, g.riskFreeRate, c.vi, g.timeToExpiry ) );

        c.price = round2( o->optionPrice( type, strike ) );

        if ( !init )
            init = true;
        else if ( prev.price < c.price )
            c.price = prev.price;

        if ( !calcGreeks( o, c.price, strike, isCall, g ) )
        {
            LOG_WARN << "failed to calc greeks for curve " << strike;
            return false;
        }

        c.delta = g.delta;

        if ( isCall )
        {
            probCurveCall_[strike] = c;
            greeksCall_[strike] = g;
        }
        else
        {
            probCurvePut_[strike] = c;
            greeksPut_[strike] = g;
        }

        delete o;

        prev = c;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool ExpectedValueCalculator::generateProbCurve( double strike, bool isCall )
{
    const OptionType type( isCall ? OptionType::Call : OptionType::Put );

    const Greeks g( isCall ? greeksCall_[strike] : greeksPut_[strike] );

    ProbCurve c = {0};
    c.min = g.bid;
    c.minvi = g.bidvi;
    c.max = g.ask;
    c.maxvi = g.askvi;

    // invalid max vi
    if ( 0.0 == c.maxvi )
        return false;

    // check for invalid min vi
    if ( c.minvi <= 0.0 )
    {
        AbstractOptionPricing *o( createPricingMethod( underlying_, g.riskFreeRate, g.riskFreeRate, 0.0, g.timeToExpiry ) );

        do
        {
            c.min += 0.01;
            c.minvi = calcImplVol( o, type, strike, c.min );

        } while ( c.minvi <= 0.0 );

        delete o;
    }

    if (( c.min < 0.0 ) || ( c.max < 0.0 ))
        return false;
    else if (( 0.0 < c.max ) && ( c.max <= c.min ))
        return false;

    if ( isCall )
        probCurveCall_[strike] = c;
    else
        probCurvePut_[strike] = c;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool ExpectedValueCalculator::calcGreeks( AbstractOptionPricing *o, double theoOptionValue, double strike, bool isCall, Greeks& result ) const
{
    const OptionType type( isCall ? OptionType::Call : OptionType::Put );

    bool okay;

    // calc vi
    result.vi = calcImplVol( o, type, strike, theoOptionValue, &okay );

    if (( okay ) && ( 0.0 < result.vi ))
    {
        // calc price
        result.price = o->optionPrice( type, strike );

        if ( 0.0 <= result.price )
        {
            // calc partials
            o->partials( type, strike, result.delta, result.gamma, result.theta, result.vega, result.rho );
            result.theta /= AppDatabase::instance()->numDays();
            result.vega /= 100.0;
            result.rho /= 100.0;

            return true;
        }
    }
    else
    {
        // failure
        static const QString failure(
            "failed to calc vi\n"
            "    underlying:     %1\n"
            "    risk free rate: %2\n"
            "    time to expiry: %3\n"
            "    type:           %4\n"
            "    strike:         %5\n"
            "    option price:   %6" );

        LOG_WARN << qPrintable( failure
            .arg( underlying_ )
            .arg( result.riskFreeRate )
            .arg( result.timeToExpiry )
            .arg( (OptionType::Call == type) ? "CALL" : "PUT" )
            .arg( strike )
            .arg( theoOptionValue ) );
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ExpectedValueCalculator::populateResultModelGreeks( const Greeks& g, item_model_type::ColumnValueMap &result )
{
    result[item_model_type::BID_ASK_SPREAD] = round2( g.spread );
    result[item_model_type::BID_ASK_SPREAD_PERCENT] = round4( 100.0 * g.spreadPercent);

    result[item_model_type::TIME_TO_EXPIRY] = round4( g.timeToExpiry );
    result[item_model_type::RISK_FREE_INTEREST_RATE] = round4( 100.0 * g.riskFreeRate );

    result[item_model_type::CALC_BID_PRICE_VI] = round4( 100.0 * g.bidvi );
    result[item_model_type::CALC_ASK_PRICE_VI] = round4( 100.0 * g.askvi );
    result[item_model_type::CALC_MARK_VI] = round4( 100.0 * g.markvi );

    result[item_model_type::CALC_THEO_OPTION_VALUE] = round2( g.price );
    result[item_model_type::CALC_THEO_VOLATILITY] = round4( 100.0 * g.vi );
    result[item_model_type::CALC_DELTA] = round4( g.delta );
    result[item_model_type::CALC_GAMMA] = round4( g.gamma );
    result[item_model_type::CALC_THETA] = round4( g.theta );
    result[item_model_type::CALC_VEGA] = round4( g.vega );
    result[item_model_type::CALC_RHO] = round4( g.rho );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double ExpectedValueCalculator::round2( double value )
{
    static const double ROUNDING = 100.0;
    return std::round( value * ROUNDING ) / ROUNDING;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double ExpectedValueCalculator::round4( double value )
{
    static const double ROUNDING = 10000.0;
    return std::round( value * ROUNDING ) / ROUNDING;
}

