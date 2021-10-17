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
    // separate strikes into ascending and descending lists
    for ( int row( 0 ); row < chains_->rowCount(); ++row )
    {
        // ignore non-standard options
        if ( isNonStandard( row ) )
            continue;

        const double strike( chains_->tableData( row, table_model_type::STRIKE_PRICE ).toDouble() );

        asc_.append( strike );
        desc_.prepend( strike );
    }

    // check we have data
    if (( !asc_.size() ) || ( !desc_.size() ) || ( asc_.size() != desc_.size() ))
        valid_ = false;
    else
    {
        // compute min/max of underlying movement
        // i.e. how far down or up the equity can go
        const double range( qMax( underlying_ - asc_.first(), desc_.first() - underlying_ ) );

        assert( 0.0 < range );

        underlyingMin_ = qMax( 0.0, underlying_ - range );
        underlyingMax_ = underlying_ + range;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ExpectedValueCalculator::~ExpectedValueCalculator()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ExpectedValueCalculator::analyze( item_model_type::Strategy strat )
{
    if ( !valid_ )
        return;

    // generate greeks
    if (( greeksCall_.isEmpty() ) && ( greeksPut_.isEmpty() ))
        valid_ &= generateGreeks();

    // generate probability curve
    if ( probCurve_.isEmpty() )
        valid_ &= generateProbCurve();

    if ( !valid_ )
        return;

    // ---- //

    const bool calls( filter_type::ONLY_CALLS & f_.optionTypeFilter() );
    const bool puts( filter_type::ONLY_PUTS & f_.optionTypeFilter() );

    switch ( strat )
    {
    // single
    case item_model_type::SINGLE:
        if ( filter_type::SINGLE & f_.optionTradingStrategyFilter() )
            analyzeSingle( calls, puts );
        break;

    // verticals
    case item_model_type::VERT_BEAR_CALL:
        if (( calls ) && ( filter_type::VERTICAL & f_.optionTradingStrategyFilter() ))
            analyzeVertBearCalls();
        break;
    case item_model_type::VERT_BULL_PUT:
        if (( puts ) && ( filter_type::VERTICAL & f_.optionTradingStrategyFilter() ))
            analyzeVertBullPuts();
        break;

    default:
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ExpectedValueCalculator::analyzeSingle( bool call, bool put ) const
{
    if ( call )
    {
        // analyze!
        for ( int row( chains_->rowCount() ); row--; )
        {
            if ( isFilteredOut( row, true ) )
                continue;

            analyzeSingleCall( row );
        }
    }

    if ( put )
    {
        // analyze!
        for ( int row( chains_->rowCount() ); row--; )
        {
            if ( isFilteredOut( row, false ) )
                continue;

            analyzeSinglePut( row );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ExpectedValueCalculator::analyzeVertBearCalls() const
{
    // analyze!
    for ( int rowLong( chains_->rowCount() ); rowLong--; )
        for ( int rowShort( qMax( 0, rowLong - f_.verticalDepth() ) ); rowShort < rowLong; ++rowShort )
        {
            if (( isFilteredOut( rowLong, true ) ) || ( isFilteredOut( rowShort, true )))
                continue;

            analyzeVertBearCall( rowLong, rowShort );
        }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ExpectedValueCalculator::analyzeVertBullPuts() const
{
    // analyze!
    for ( int rowShort( chains_->rowCount() ); rowShort--; )
        for ( int rowLong( qMax( 0, rowShort - f_.verticalDepth() ) ); rowLong < rowShort; ++rowLong )
        {
            if (( isFilteredOut( rowLong, false ) ) || ( isFilteredOut( rowShort, false )))
                continue;

            analyzeVertBullPut( rowLong, rowShort );
        }
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
        // ignore non-standard options
        if ( isNonStandard( row ) )
            continue;

        const double strike( chains_->tableData( row, table_model_type::STRIKE_PRICE ).toDouble() );

        if (( !generateGreeks( row, strike, true ) ) ||     // calls
            ( !generateGreeks( row, strike, false ) ))      // puts
        {
            LOG_WARN << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << " error generating greeks!";
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

    bool parity( false );

    // populate curve with data
    foreach ( const double& strike, asc_ )
    {
        // call
        if ( !generateProbCurve( strike, true ) )
        {
            LOG_WARN << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << " CALL error generating probablity curve data!";
            parity = true;
        }

        // put
        if ( !generateProbCurve( strike, false ) )
        {
            LOG_WARN << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << " PUT error generating probablity curve data!";
            parity = true;
        }
    }

    // errors generating probability, attempt to use put/call parity
    if ( parity )
    {
        foreach ( const double& strike, asc_ )
        {
            if (( probCurveCall_.contains( strike ) ) && ( probCurvePut_.contains( strike )))
                continue;
            else if (( !probCurveCall_.contains( strike ) ) && ( !probCurvePut_.contains( strike )))
            {
                LOG_WARN << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << " error generating probability from put/call parity";
                return false;
            }

            // put
            if ( probCurveCall_.contains( strike ) )
                generateProbCurveParity( strike, false );

            // call
            else if ( probCurvePut_.contains( strike ) )
                generateProbCurveParity( strike, true );

            LOG_INFO << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << " generating probability using put/call parity";
        }
    }

    // adjust curve
    if (( !calcProbCurve( probCurveCall_, asc_, true ) ) ||      // calls
        ( !calcProbCurve( probCurvePut_, desc_, false ) ))       // puts
    {
        LOG_WARN << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " error calculating probablity curve!";
        return false;
    }

    // merge call and put curve implied volatility
    foreach ( const double& strike, asc_ )
    {
        ProbCurve call( probCurveCall_[strike] );
        ProbCurve put( probCurvePut_[strike] );

        LOG_TRACE << "PROB " << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << "\n" <<
            "    CALL " << call.min << " " << call.max << " " << call.minvi <<  " " << call.maxvi << "\n" <<
            "    PUT " << put.min << " " << put.max << " " << put.minvi <<  " " << put.maxvi;

        const double minvi( qMax( call.minvi, put.minvi ) );
        const double maxvi( qMin( call.maxvi, put.maxvi ) );

        if ( maxvi < minvi )
        {
            LOG_WARN << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << " non-overlapping prob curve";

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
    if (( !calcProbCurvePrices( probCurveCall_, asc_, true ) ) ||      // calls
        ( !calcProbCurvePrices( probCurvePut_, desc_, false ) ))       // puts
    {
        LOG_WARN << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " error calculating probablity curve prices!";
        return false;
    }

    // generate probabilities
    foreach ( const double& strike, asc_ )
    {
        ProbCurve call( probCurveCall_[strike] );
        ProbCurve put( probCurvePut_[strike] );

        LOG_DEBUG << "PROB ITM " << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << " " << call.vi << " " << call.delta << " " << put.vi << " " << put.delta;

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
    populateResultModelSingle( row, true, result );

    // Calculated Fields
    if ( greeksCall_.contains( strike ) )
        populateResultModelGreeks( greeksCall_[strike], result );

    // ---- //

    // check for active trading
    if (( !result[item_model_type::BID_SIZE].toInt() ) || ( !result[item_model_type::ASK_SIZE].toInt() ))
        return;

    // ensure we have probabilities
    if ( !probCurve_.contains( strike ) )
        return;

    const double itmProb( calcProbInTheMoney( strike, true ) );
    const double otmProb( 1.0 - itmProb );

    result[item_model_type::PROBABILITY_ITM] = round4( 100.0 * itmProb );
    result[item_model_type::PROBABILITY_OTM] = round4( 100.0 * otmProb );

    double timeToExpiryWeeks( result[item_model_type::TIME_TO_EXPIRY].toDouble() );
    timeToExpiryWeeks *= AppDatabase::instance()->numDays();
    timeToExpiryWeeks /= 7.0;

    // ---- //

    const double theoOptionValue( result[item_model_type::CALC_THEO_OPTION_VALUE].toDouble() );

    // return on investment
    const double multiplier( result[item_model_type::MULTIPLIER].toDouble() );
    const double mark( result[item_model_type::INVESTMENT_OPTION_PRICE].toDouble() );

    // when cost basis provided, use that for calculations
    // otherwise we need to purchase the shares at market price
    const double costBasis( (0.0 < costBasis_) ? costBasis_ : (underlying_ + (equityTradeCost_ / multiplier)) );

    const double maxGain( (multiplier * mark) - optionTradeCost_ );
    const double maxLoss( (multiplier * costBasis) - maxGain );
    const double investmentValue( maxLoss );

    // nothing to gain
    if ( maxGain <= 0.0 )
        return;

    const double roi( maxGain / investmentValue );

    result[item_model_type::INVESTMENT_OPTION_PRICE_VS_THEO] = mark - theoOptionValue;          // when selling, favorable when mark price above theo price

    result[item_model_type::INVESTMENT_VALUE] = investmentValue;
    result[item_model_type::MAX_GAIN] = maxGain;
    result[item_model_type::MAX_LOSS] = maxLoss;

    result[item_model_type::ROI] = round2( 100.0 * roi );
    result[item_model_type::ROI_TIME] = round2( 100.0 * (roi / timeToExpiryWeeks) );

    // ---- //

    // expected value
    double ev( otmProb * maxGain );
    ev -= calcExpectedLossCall( multiplier, strike, 999999.0, costBasis, otmProb );

    const double ev_roi( ev / investmentValue );

    result[item_model_type::EXPECTED_VALUE] = round2( ev );
    result[item_model_type::EXPECTED_VALUE_ROI] = round2( 100.0 * ev_roi );
    result[item_model_type::EXPECTED_VALUE_ROI_TIME] = round2( 100.0 * (ev_roi / timeToExpiryWeeks) );

    // add row
    addRowToItemModel( result );
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
    populateResultModelSingle( row, false, result );

    // Calculated Fields
    if ( greeksPut_.contains( strike ) )
        populateResultModelGreeks( greeksPut_[strike], result );

    // ---- //

    // check for active trading
    if (( !result[item_model_type::BID_SIZE].toInt() ) || ( !result[item_model_type::ASK_SIZE].toInt() ))
        return;

    // ensure we have probabilities
    if ( !probCurve_.contains( strike ) )
        return;

    const double itmProb( calcProbInTheMoney( strike, false ) );
    const double otmProb( 1.0 - itmProb );

    result[item_model_type::PROBABILITY_ITM] = round4( 100.0 * itmProb );
    result[item_model_type::PROBABILITY_OTM] = round4( 100.0 * otmProb );

    double timeToExpiryWeeks( result[item_model_type::TIME_TO_EXPIRY].toDouble() );
    timeToExpiryWeeks *= AppDatabase::instance()->numDays();
    timeToExpiryWeeks /= 7.0;

    // ---- //

    const double theoOptionValue( result[item_model_type::CALC_THEO_OPTION_VALUE].toDouble() );

    // return on investment
    const double multiplier( result[item_model_type::MULTIPLIER].toDouble() );
    const double mark( result[item_model_type::INVESTMENT_OPTION_PRICE].toDouble() );

    const double maxGain( (multiplier * mark) - optionTradeCost_ );
    const double maxLoss( (multiplier * strike) - maxGain );
    const double investmentValue( maxLoss );

    const double costBasis( investmentValue / multiplier );

    // nothing to gain
    if ( maxGain <= 0.0 )
        return;

    const double roi( maxGain / investmentValue );

    result[item_model_type::INVESTMENT_OPTION_PRICE_VS_THEO] = mark - theoOptionValue;          // when selling, favorable when mark price above theo price

    result[item_model_type::INVESTMENT_VALUE] = investmentValue;
    result[item_model_type::MAX_GAIN] = maxGain;
    result[item_model_type::MAX_LOSS] = maxLoss;

    result[item_model_type::ROI] = round2( 100.0 * roi );
    result[item_model_type::ROI_TIME] = round2( 100.0 * (roi / timeToExpiryWeeks) );

    // ---- //

    // expected value
    double ev( otmProb * maxGain );
    ev -= calcExpectedLossPut( multiplier, 0.0, strike, costBasis, otmProb );

    const double ev_roi( ev / investmentValue );

    result[item_model_type::EXPECTED_VALUE] = round2( ev );
    result[item_model_type::EXPECTED_VALUE_ROI] = round2( 100.0 * ev_roi );
    result[item_model_type::EXPECTED_VALUE_ROI_TIME] = round2( 100.0 * (ev_roi / timeToExpiryWeeks) );

    // add row
    addRowToItemModel( result );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ExpectedValueCalculator::analyzeVertBearCall( int rowLong, int rowShort ) const
{
    const double strikeLong( chains_->tableData( rowLong, table_model_type::STRIKE_PRICE ).toDouble() );
    const double strikeShort( chains_->tableData( rowShort, table_model_type::STRIKE_PRICE ).toDouble() );

    const QString optionType( QObject::tr( "Vertical Bear Call" ) );

    const QString description( QString( "%1 $%2/$%3 %4" )
        .arg( chains_->expirationDate().toString( "MMM dd ''yy" ) )
        .arg( strikeShort )
        .arg( strikeLong )
        .arg( optionType ) );

    item_model_type::ColumnValueMap result;

    result[item_model_type::STRATEGY] = item_model_type::VERT_BEAR_CALL;
    result[item_model_type::STRATEGY_DESC] = description;

    // Option Chain Information
    populateResultModelVertical( rowLong, rowShort, true, result );

    // Calculated Fields
    if (( greeksCall_.contains( strikeLong ) ) && ( greeksCall_.contains( strikeShort ) ))
        populateResultModelGreeksSpread( greeksCall_[strikeLong], greeksCall_[strikeShort], result );

    // ---- //

    // check for active trading
    if (( !result[item_model_type::BID_SIZE].toInt() ) || ( !result[item_model_type::ASK_SIZE].toInt() ))
        return;

    // ensure we have probabilities
    if (( !probCurve_.contains( strikeLong ) ) || ( !probCurve_.contains( strikeShort )))
        return;

    const double breakEvenPrice( result[item_model_type::BREAK_EVEN_PRICE].toDouble() );

    const double itmProb( calcProbInTheMoney( breakEvenPrice, true ) );
    const double otmProb( 1.0 - itmProb );

    result[item_model_type::PROBABILITY_ITM] = round4( 100.0 * itmProb );
    result[item_model_type::PROBABILITY_OTM] = round4( 100.0 * otmProb );

    double timeToExpiryWeeks( result[item_model_type::TIME_TO_EXPIRY].toDouble() );
    timeToExpiryWeeks *= AppDatabase::instance()->numDays();
    timeToExpiryWeeks /= 7.0;

    // ---- //

    const double theoOptionValue( result[item_model_type::CALC_THEO_OPTION_VALUE].toDouble() );

    // return on investment
    const double multiplier( result[item_model_type::MULTIPLIER].toDouble() );
    const double mark( result[item_model_type::INVESTMENT_OPTION_PRICE].toDouble() );

    // when cost basis provided, use that for calculations
    // otherwise we need to purchase the shares at market price
    const double costBasis( (0.0 < costBasis_) ? costBasis_ : (underlying_ + (equityTradeCost_ / multiplier)) );

    const double maxGain( (multiplier * mark) - (2.0 * optionTradeCost_) );
    const double maxLoss( (multiplier * (strikeLong - strikeShort)) + (2.0 * optionTradeCost_) );
    const double investmentValue( (multiplier * costBasis) - maxGain );

    // nothing to gain
    if ( maxGain <= 0.0 )
        return;

    const double roi( maxGain / investmentValue );

    result[item_model_type::INVESTMENT_OPTION_PRICE_VS_THEO] = mark - theoOptionValue;          // when selling, favorable when mark price above theo price

    result[item_model_type::INVESTMENT_VALUE] = investmentValue;
    result[item_model_type::MAX_GAIN] = maxGain;
    result[item_model_type::MAX_LOSS] = maxLoss;

    result[item_model_type::ROI] = round2( 100.0 * roi );
    result[item_model_type::ROI_TIME] = round2( 100.0 * (roi / timeToExpiryWeeks) );

    // ---- //

    const double itmProbLong( calcProbInTheMoney( strikeLong, true ) );
    const double otmProbShort( 1.0 - calcProbInTheMoney( strikeShort, true ) );

    // expected value
    double ev( otmProbShort * maxGain );
    ev -= itmProbLong * maxLoss;
    ev -= calcExpectedLossCall( multiplier, strikeShort, strikeLong, costBasis, (itmProbLong + otmProbShort) );

    const double ev_roi( ev / investmentValue );

    result[item_model_type::EXPECTED_VALUE] = round2( ev );
    result[item_model_type::EXPECTED_VALUE_ROI] = round2( 100.0 * ev_roi );
    result[item_model_type::EXPECTED_VALUE_ROI_TIME] = round2( 100.0 * (ev_roi / timeToExpiryWeeks) );

    // add row
    addRowToItemModel( result );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ExpectedValueCalculator::analyzeVertBullPut( int rowLong, int rowShort ) const
{
    const double strikeLong( chains_->tableData( rowLong, table_model_type::STRIKE_PRICE ).toDouble() );
    const double strikeShort( chains_->tableData( rowShort, table_model_type::STRIKE_PRICE ).toDouble() );

    const QString optionType( QObject::tr( "Vertical Bull Put" ) );

    const QString description( QString( "%1 $%2/$%3 %4" )
        .arg( chains_->expirationDate().toString( "MMM dd ''yy" ) )
        .arg( strikeShort )
        .arg( strikeLong )
        .arg( optionType ) );

    item_model_type::ColumnValueMap result;

    result[item_model_type::STRATEGY] = item_model_type::VERT_BULL_PUT;
    result[item_model_type::STRATEGY_DESC] = description;

    // Option Chain Information
    populateResultModelVertical( rowLong, rowShort, false, result );

    // Calculated Fields
    if (( greeksPut_.contains( strikeLong ) ) && ( greeksPut_.contains( strikeShort ) ))
        populateResultModelGreeksSpread( greeksPut_[strikeLong], greeksPut_[strikeShort], result );

    // ---- //

    // check for active trading
    if (( !result[item_model_type::BID_SIZE].toInt() ) || ( !result[item_model_type::ASK_SIZE].toInt() ))
        return;

    // ensure we have probabilities
    if (( !probCurve_.contains( strikeLong ) ) || ( !probCurve_.contains( strikeShort )))
        return;

    const double breakEvenPrice( result[item_model_type::BREAK_EVEN_PRICE].toDouble() );

    const double itmProb( calcProbInTheMoney( breakEvenPrice, false ) );
    const double otmProb( 1.0 - itmProb );

    result[item_model_type::PROBABILITY_ITM] = round4( 100.0 * itmProb );
    result[item_model_type::PROBABILITY_OTM] = round4( 100.0 * otmProb );

    double timeToExpiryWeeks( result[item_model_type::TIME_TO_EXPIRY].toDouble() );
    timeToExpiryWeeks *= AppDatabase::instance()->numDays();
    timeToExpiryWeeks /= 7.0;

    // ---- //

    const double theoOptionValue( result[item_model_type::CALC_THEO_OPTION_VALUE].toDouble() );

    // return on investment
    const double multiplier( result[item_model_type::MULTIPLIER].toDouble() );
    const double mark( result[item_model_type::INVESTMENT_OPTION_PRICE].toDouble() );

    const double maxGain( (multiplier * mark) - (2.0 * optionTradeCost_) );
    const double maxLoss( (multiplier * (strikeShort - strikeLong)) + (2.0 * optionTradeCost_) );
    const double investmentValue( (multiplier * strikeShort) - maxGain );

    const double costBasis( investmentValue / multiplier );

    // nothing to gain
    if ( maxGain <= 0.0 )
        return;

    const double roi( maxGain / investmentValue );

    result[item_model_type::INVESTMENT_OPTION_PRICE_VS_THEO] = mark - theoOptionValue;          // when selling, favorable when mark price above theo price

    result[item_model_type::INVESTMENT_VALUE] = investmentValue;
    result[item_model_type::MAX_GAIN] = maxGain;
    result[item_model_type::MAX_LOSS] = maxLoss;

    result[item_model_type::ROI] = round2( 100.0 * roi );
    result[item_model_type::ROI_TIME] = round2( 100.0 * (roi / timeToExpiryWeeks) );

    // ---- //

    const double itmProbLong( calcProbInTheMoney( strikeLong, false ) );
    const double otmProbShort( 1.0 - calcProbInTheMoney( strikeShort, false ) );

    // expected value
    double ev( otmProbShort * maxGain );
    ev -= itmProbLong * maxLoss;
    ev -= calcExpectedLossPut( multiplier, strikeLong, strikeShort, costBasis, (itmProbLong + otmProbShort) );

    const double ev_roi( ev / investmentValue );

    result[item_model_type::EXPECTED_VALUE] = round2( ev );
    result[item_model_type::EXPECTED_VALUE_ROI] = round2( 100.0 * ev_roi );
    result[item_model_type::EXPECTED_VALUE_ROI_TIME] = round2( 100.0 * (ev_roi / timeToExpiryWeeks) );

    // add row
    addRowToItemModel( result );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool ExpectedValueCalculator::generateGreeks( int row, double strike, bool isCall )
{
    static const double SECONDS_PER_DAY = 86400;

    const QDateTime quoteTime( chains_->tableData( row, isCall ? table_model_type::CALL_QUOTE_TIME : table_model_type::PUT_QUOTE_TIME ).toDateTime() );
    const QDateTime expiryDate( chains_->tableData( row, isCall ? table_model_type::CALL_EXPIRY_DATE : table_model_type::PUT_EXPIRY_DATE ).toDateTime() );

    if (( quoteTime.isValid() ) && ( expiryDate.isValid() ) && ( quoteTime < expiryDate ))
    {
        const double bid( chains_->tableData( row, isCall ? table_model_type::CALL_BID_PRICE : table_model_type::PUT_BID_PRICE ).toDouble() );
        const double ask( chains_->tableData( row, isCall ? table_model_type::CALL_ASK_PRICE : table_model_type::PUT_ASK_PRICE ).toDouble() );
        const double mark( chains_->tableData( row, isCall ? table_model_type::CALL_MARK : table_model_type::PUT_MARK ).toDouble() );

        const OptionType type( isCall ? OptionType::Call : OptionType::Put );

        Greeks result = {};
        result.spread = ask - bid;
        result.spreadPercent = result.spread / ask;

        result.bid = bid;
        result.ask = ask;
        result.mark = mark;

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

        result.bidvi = calcImplVol( o, type, strike, bid );
        result.askvi = calcImplVol( o, type, strike, ask );
        result.markvi = calcImplVol( o, type, strike, mark );
/*
        // determine theo option value and VI
        double theoOptionValue( chains_->tableData( row, isCall ? table_model_type::CALL_THEO_OPTION_VALUE : table_model_type::PUT_THEO_OPTION_VALUE ).toDouble() );

        // use mark if they giving us bad value
        if ( theoOptionValue < 0.0 )
        {
            LOG_WARN << "using mark for theo option value";
            theoOptionValue = mark;
        }

        // calc!
        if ( !calcGreeks( o, theoOptionValue, strike, isCall, result ) )
            LOG_WARN << "invalid theo option value";
*/
        destroyPricingMethod( o );

        if ( isCall )
            greeksCall_[strike] = result;
        else
            greeksPut_[strike] = result;

        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool ExpectedValueCalculator::calcProbCurve( OptionProbCurve& curve, const QList<double>& direction, bool isCall ) const
{
    const OptionType type( isCall ? OptionType::Call : OptionType::Put );

    bool init( false );

    ProbCurve prev = {};

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

            destroyPricingMethod( o );

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

    ProbCurve prev = {};

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
            destroyPricingMethod( o );

            LOG_WARN << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << " " << (isCall ? "CALL" : "PUT") << " failed to calc greeks";
            return false;
        }

        c.delta = g.delta;

        g.marketPrice = c.min + ((c.max - c.min) / 2.0);

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

        destroyPricingMethod( o );

        prev = c;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool ExpectedValueCalculator::generateProbCurve( double strike, bool isCall )
{
    const OptionType type( isCall ? OptionType::Call : OptionType::Put );

    const Greeks g( isCall ? greeksCall_[strike] : greeksPut_[strike] );

    ProbCurve c = {};
    c.min = g.bid;
    c.minvi = g.bidvi;
    c.max = g.ask;
    c.maxvi = g.askvi;

    // invalid max vi
    if ( c.maxvi <= 0.0 )
    {
        AbstractOptionPricing *o( createPricingMethod( underlying_, g.riskFreeRate, g.riskFreeRate, 0.0, g.timeToExpiry ) );

        // reduce max until we get a valid maxvi
        do
        {
            c.max -= 0.01;

            if (( c.max <= 0.0 ) || ( c.max < c.min ))
                break;

            c.maxvi = calcImplVol( o, type, strike, c.max );

        } while ( c.maxvi <= 0.0 );

        destroyPricingMethod( o );

        // invalid max vi
        if ( c.maxvi <= 0.0 )
        {
            LOG_WARN << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << " " << (isCall ? "CALL" : "PUT") << " invalid max " << c.max << " " << c.maxvi;
            return false;
        }
    }

    // check for invalid min vi
    if ( c.minvi <= 0.0 )
    {
        AbstractOptionPricing *o( createPricingMethod( underlying_, g.riskFreeRate, g.riskFreeRate, 0.0, g.timeToExpiry ) );

        do
        {
            c.min += 0.01;

            if ( c.max < c.min )
                break;

            c.minvi = calcImplVol( o, type, strike, c.min );

        } while ( c.minvi <= 0.0 );

        destroyPricingMethod( o );

        // invalid min vi
        if ( c.minvi <= 0.0 )
        {
            LOG_WARN << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << " " << (isCall ? "CALL" : "PUT") << " invalid min " << c.min << " " << c.minvi;
            return false;
        }
    }

    if (( c.min < 0.0 ) || ( c.max < 0.0 ))
    {
        LOG_WARN << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << " " << (isCall ? "CALL" : "PUT") << " negative min/max";
        return false;
    }
    else if (( 0.0 < c.max ) && ( c.max <= c.min ))
    {
        LOG_WARN << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << " " << (isCall ? "CALL" : "PUT") << " inverted min/max";
        return false;
    }

    if ( isCall )
        probCurveCall_[strike] = c;
    else
        probCurvePut_[strike] = c;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool ExpectedValueCalculator::generateProbCurveParity( double strike, bool isCall )
{
    const OptionType type( isCall ? OptionType::Call : OptionType::Put );

    const Greeks g( isCall ? greeksCall_[strike] : greeksPut_[strike] );

    const ProbCurve other( isCall ? probCurvePut_[strike] : probCurveCall_[strike] );

    AbstractOptionPricing *o( createPricingMethod( underlying_, g.riskFreeRate, g.riskFreeRate, 0.0, g.timeToExpiry ) );

    // calculate using parity
    ProbCurve c = {};

    o->setSigma( c.minvi = other.minvi );
    c.min = o->optionPrice( type, strike );

    o->setSigma( c.maxvi = other.maxvi );
    c.max = o->optionPrice( type, strike );

    destroyPricingMethod( o );

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

        LOG_DEBUG << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << " " << (isCall ? "CALL" : "PUT") << " " <<
            qPrintable( failure
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
double ExpectedValueCalculator::calcProbInTheMoney( double price, bool isCall ) const
{
    // check if this price is in curve (i.e. strike price)
    if ( probCurve_.contains( price ) )
        return ( isCall ? probCurve_[price] : (1.0 - probCurve_[price]) );

    double prevStrike( 0.0 );

    // find strike above and below price
    foreach ( const double& s, asc_ )
    {
        if (( prevStrike < price ) && ( price <= s ))
        {
            const double itmProbMin( isCall ? probCurve_[prevStrike] : (1.0 - probCurve_[prevStrike]) );
            const double itmProbMax( isCall ? probCurve_[s] : (1.0 - probCurve_[s]) );

            // interpolate
            return ( itmProbMin + ((double)(price - prevStrike) / (double)(s - prevStrike)) * (itmProbMax - itmProbMin) );
        }

        prevStrike = s;
    }

    return 0.0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double ExpectedValueCalculator::calcExpectedLossCall( double multiplier, double priceMin, double priceMax, double costBasis, double totalProb ) const
{
    bool done( false );

    double prevStrike( 999999.0 );
    double prevProb( 0.0 );

    double loss( 0.0 );

    // iterate each strike price
    foreach ( const double& strike, desc_ )
    {
        const double prob( calcProbInTheMoney( strike, true ) );

        // check this strike within our min/max
        if ( strike < priceMax )
        {
            const double probDelta( prob - prevProb );

            const double ceil( qMin( underlyingMax_, prevStrike ) );
            const double price( ceil - ((ceil - strike) / 2.0) );

            // accumulate loss at this possibility
            loss += multiplier * probDelta * (price - costBasis);
            totalProb += probDelta;

            if ( (done = (strike <= priceMin)) )
                break;
        }

        prevStrike = strike;
        prevProb = prob;
    }

    // last strike was above min price... add in remaining loss at min price
    if ( !done )
    {
        const double probDelta( 1.0 - prevProb );

        const double ceil( qMin( underlyingMax_, prevStrike ) );
        const double price( ceil - ((ceil - priceMin) / 2.0) );

        // accumulate loss at this possibility
        loss += multiplier * probDelta * (price - costBasis);
        totalProb += probDelta;
    }

    assert(( 0.999 <= totalProb ) && ( totalProb <= 1.001 ));
    return loss;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double ExpectedValueCalculator::calcExpectedLossPut( double multiplier, double priceMin, double priceMax, double costBasis, double totalProb ) const
{
    bool done( false );

    double prevStrike( 0.0 );
    double prevProb( 0.0 );

    double loss( 0.0 );

    // iterate each strike price
    foreach ( const double& strike, asc_ )
    {
        const double prob( calcProbInTheMoney( strike, false ) );

        // check this strike within our min/max
        if ( priceMin < strike )
        {
            const double probDelta( prob - prevProb );

            const double floor( qMax( underlyingMin_, prevStrike ) );
            const double price( floor + ((strike - floor) / 2.0) );

            // accumulate loss at this possibility
            loss += multiplier * probDelta * (costBasis - price);
            totalProb += probDelta;

            if ( (done = (priceMax <= strike)) )
                break;
        }

        prevStrike = strike;
        prevProb = prob;
    }

    // last strike was below max price... add in remaining loss at max price
    if ( !done )
    {
        const double probDelta( 1.0 - prevProb );

        const double floor( qMax( underlyingMin_, prevStrike ) );
        const double price( floor + ((priceMax - floor) / 2.0) );

        // accumulate loss at this possibility
        loss += multiplier * probDelta * (costBasis - price);
        totalProb += probDelta;
    }

    assert(( 0.999 <= totalProb ) && ( totalProb <= 1.001 ));
    return loss;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ExpectedValueCalculator::populateResultModelGreeks( const Greeks& g, item_model_type::ColumnValueMap &result )
{
    result[item_model_type::BID_ASK_SPREAD] = round2( g.spread );
    result[item_model_type::BID_ASK_SPREAD_PERCENT] = round4( 100.0 * g.spreadPercent );

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

    result[item_model_type::INVESTMENT_OPTION_PRICE] = round2( g.marketPrice );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ExpectedValueCalculator::populateResultModelGreeksSpread( const Greeks& glong, const Greeks& gshort, item_model_type::ColumnValueMap &result )
{
    const double bid( gshort.bid - glong.ask );
    const double ask( gshort.ask - glong.bid );

    result[item_model_type::BID_ASK_SPREAD] = round2( ask - bid );
    result[item_model_type::BID_ASK_SPREAD_PERCENT] = round4( 100.0 * (( ask - bid ) / ask) );

    result[item_model_type::TIME_TO_EXPIRY] = round4( gshort.timeToExpiry );
    result[item_model_type::RISK_FREE_INTEREST_RATE] = round4( 100.0 * gshort.riskFreeRate );

    // net volatility
    // https://en.wikipedia.org/wiki/Net_volatility#:~:text=Net%20volatility%20refers%20to%20the,implied%20volatility%20of%20the%20spread.
    double bidviNet = ((glong.vega * glong.bidvi) - (gshort.vega * gshort.bidvi)) / (glong.vega - gshort.vega);
    double askviNet = ((glong.vega * glong.askvi) - (gshort.vega * gshort.askvi)) / (glong.vega - gshort.vega);
    double markviNet = ((glong.vega * glong.markvi) - (gshort.vega * gshort.markvi)) / (glong.vega - gshort.vega);

    result[item_model_type::CALC_BID_PRICE_VI] = round4( 100.0 * bidviNet );
    result[item_model_type::CALC_ASK_PRICE_VI] = round4( 100.0 * askviNet );
    result[item_model_type::CALC_MARK_VI] = round4( 100.0 * markviNet );

    // net volatility
    // https://en.wikipedia.org/wiki/Net_volatility#:~:text=Net%20volatility%20refers%20to%20the,implied%20volatility%20of%20the%20spread.
    double viNet = ((glong.vega * glong.vi) - (gshort.vega * gshort.vi)) / (glong.vega - gshort.vega);

    result[item_model_type::CALC_THEO_OPTION_VALUE] = round2( gshort.price - glong.price );
    result[item_model_type::CALC_THEO_VOLATILITY] = round4( 100.0 * viNet );
    result[item_model_type::CALC_DELTA] = round4( glong.delta - gshort.delta );
    result[item_model_type::CALC_GAMMA] = round4( glong.gamma - gshort.gamma );
    result[item_model_type::CALC_THETA] = round4( glong.theta - gshort.theta );
    result[item_model_type::CALC_VEGA] = round4( glong.vega - gshort.vega );
    result[item_model_type::CALC_RHO] = round4( glong.rho - gshort.rho );

    result[item_model_type::INVESTMENT_OPTION_PRICE] = round2( gshort.marketPrice - glong.marketPrice );
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

