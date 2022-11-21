/**
 * @file expectedvaluecalc.cpp
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
#include "db/optiondata.h"
#include "db/symboldbs.h"

#include "util/abstractoptionpricing.h"

#include <QObject>

// uncomment to debug calculations
//#define DEBUG_CALC

///////////////////////////////////////////////////////////////////////////////////////////////////
ExpectedValueCalculator::ExpectedValueCalculator( double underlying, const table_model_type *chains, item_model_type *results ) :
    _Mybase( underlying, chains, results )
{
    const QDateTime now( AppDatabase::instance()->currentDateTime() );

    // separate strikes into ascending and descending lists
    for ( int row( 0 ); row < chains_->rowCount(); ++row )
    {
        // ignore non-standard options
        if ( isNonStandard( row ) )
            continue;
        // ignore expired options
        else if ( chains->tableData( row, table_model_type::CALL_EXPIRY_DATE ).toDateTime() < now )
            continue;
        else if ( chains->tableData( row, table_model_type::PUT_EXPIRY_DATE ).toDateTime() < now )
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
            LOG_ERROR << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << " error generating greeks!";
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
                LOG_ERROR << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << " error generating probability from put/call parity";
                return false;
            }

            LOG_INFO << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << " generating probability using put/call parity";

            // use parity to generate other side of curve
            generateProbCurveParity( strike, probCurvePut_.contains( strike ) );
        }
    }

    // adjust curve
    if (( !calcProbCurve( probCurveCall_, asc_, true ) ) ||      // calls
        ( !calcProbCurve( probCurvePut_, desc_, false ) ))       // puts
    {
        LOG_ERROR << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " error calculating probablity curve!";
        return false;
    }

    // merge call and put curve implied volatility
    foreach ( const double& strike, asc_ )
    {
        ProbCurve call( probCurveCall_[strike] );
        ProbCurve put( probCurvePut_[strike] );

#ifdef DEBUG_CALC
        static const QString STRIKE_VI( "    %1 %2 %3 %4 %5" );

        LOG_TRACE << "PROB " << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << "\n" <<
            qPrintable( STRIKE_VI.arg( "CALL", 4 ).arg( call.min, 8, 'f', 3 ).arg( call.max, 8, 'f', 3 ).arg( call.minvi, 12, 'f', 6 ).arg( call.maxvi, 12, 'f', 6 ) ) << "\n" <<
            qPrintable( STRIKE_VI.arg( "PUT", 4 ).arg( put.min, 8, 'f', 3 ).arg( put.max, 8, 'f', 3 ).arg( put.minvi, 12, 'f', 6 ).arg( put.maxvi, 12, 'f', 6 ) );
#endif

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

    LOG_TRACE << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " calc prices...";

    // calculate prices
    if (( !calcProbCurvePrices( probCurveCall_, asc_, true ) ) ||      // calls
        ( !calcProbCurvePrices( probCurvePut_, desc_, false ) ))       // puts
    {
        LOG_ERROR << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " error calculating probablity curve prices!";
        return false;
    }

    // generate probabilities
    OptionChainCurves curves;

    double prevProb( 1.0 );

    foreach ( const double& strike, asc_ )
    {
        ProbCurve call( probCurveCall_[strike] );
        ProbCurve put( probCurvePut_[strike] );

#ifdef DEBUG_CALC
        static const QString PROB_ITM( "PROB ITM %1 %2 %3 %4 %5 %6 %7" );

        LOG_DEBUG << qPrintable( PROB_ITM
            .arg( chains_->symbol() )
            .arg( daysToExpiry_ )
            .arg( strike, 7, 'f', 1 )
            .arg( call.vi, 12, 'f', 6 )
            .arg( call.delta, 12, 'f', 6 )
            .arg( put.vi, 12, 'f', 6 )
            .arg( put.delta, 12, 'f', 6 ) );
#endif

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

        // ensure probability is always decreasing
        if ( probCurve_[strike] <= prevProb )
            prevProb = probCurve_[strike];
        else
        {
            LOG_WARN << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << " probability inversion";
            probCurve_[strike] = prevProb;
        }

        // track curve data
        if ( greeksCall_.contains( strike ) )
            curves.callVolatility[strike] = greeksCall_[strike].markvi;

        if ( greeksPut_.contains( strike ) )
            curves.putVolatility[strike] = greeksPut_[strike].markvi;

        curves.volatility[strike] = (call.vi + put.vi) / 2.0;

        curves.itmProbability[strike] = probCurve_[strike];
        curves.otmProbability[strike] = 1.0 - probCurve_[strike];
    }

    // save option curve info
    const QString stamp( chains_->data0( table_model_type::STAMP ).toString() );

    LOG_DEBUG << "set option chain curve " << qPrintable( chains_->symbol() ) << " " << qPrintable( chains_->expirationDate().toString() ) << " " << qPrintable( stamp );

    SymbolDatabases::instance()->setOptionChainCurves(
        chains_->symbol(),
        chains_->expirationDate(),
        QDateTime::fromString( stamp, Qt::ISODateWithMs ),
        curves );

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
AbstractOptionPricing *ExpectedValueCalculator::createPricingMethod( double S, double r, double b, double sigma, double T, const std::vector<double>& divTimes, const std::vector<double>& divYields, bool european ) const
{
    Q_UNUSED( divYields )
    Q_UNUSED( divTimes )

    // for this mode we should have no dividend already passed in
    assert( b == r );

    // support dividends
    return createPricingMethod( S, r, (r - dividendYield()), sigma, T, european );
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

    // ensure we have probabilities
    if ( !probCurve_.contains( strike ) )
        return;

    const double itmProb( calcProbInTheMoney( strike, true ) );
    const double otmProb( 1.0 - itmProb );

    result[item_model_type::PROBABILITY_ITM] = 100.0 * itmProb;
    result[item_model_type::PROBABILITY_OTM] = 100.0 * otmProb;

    double timeToExpiryWeeks( result[item_model_type::TIME_TO_EXPIRY].toDouble() );
    timeToExpiryWeeks *= AppDatabase::instance()->numDays();
    timeToExpiryWeeks /= 7.0;

    // ---- //

    const double numWeeksPerYear( AppDatabase::instance()->numDays() / 7.0 );

    const double multiplier( result[item_model_type::MULTIPLIER].toDouble() );

    const double theoOptionValue( result[item_model_type::CALC_THEO_OPTION_VALUE].toDouble() );
    const double mark( result[item_model_type::INVESTMENT_OPTION_PRICE].toDouble() );

    result[item_model_type::INVESTMENT_OPTION_PRICE_VS_THEO] = mark - theoOptionValue;                  // when selling, favorable when mark price above theo price

    // when cost basis provided, use that for calculations
    // otherwise we need to purchase the shares at market price
    const double equitySharePrice( (0.0 < costBasis_) ? costBasis_ : (underlying_ + (equityTradeCost_ / multiplier)) );

    // return on investment
    const double premium( (multiplier * mark) - optionTradeCost_ );                                     // how much you get for selling the option

    const double investmentValue( (multiplier * equitySharePrice) - premium );                          // how much money is locked up
    const double maxGain( premium + (multiplier * (strike - equitySharePrice)) - equityTradeCost_ );    // maximum amount of money you can possibly make
    const double maxLoss( investmentValue );                                                            // maximum amount of money you can possibly lose (i.e. underlying goes to 0.00)

    result[item_model_type::INVESTMENT_AMOUNT] = investmentValue;
    result[item_model_type::PREMIUM_AMOUNT] = premium;
    result[item_model_type::MAX_GAIN] = maxGain;
    result[item_model_type::MAX_LOSS] = maxLoss;

    // update breakeven for call
    // the TDA provided breakeven assumes naked call, we want to handle CC as well
    result[item_model_type::BREAK_EVEN_PRICE] = equitySharePrice - (premium / multiplier);

    const double ror( maxGain / maxLoss );

    result[item_model_type::ROR] = 100.0 * ror;
    result[item_model_type::ROR_WEEK] = result[item_model_type::ROR].toDouble() / timeToExpiryWeeks;
    result[item_model_type::ROR_YEAR] = numWeeksPerYear * result[item_model_type::ROR_WEEK].toDouble();
    result[item_model_type::ROR_MONTH] = result[item_model_type::ROR_YEAR].toDouble() / 12.0;

    const double roi( premium / investmentValue );

    result[item_model_type::ROI] = 100.0 * roi;
    result[item_model_type::ROI_WEEK] = result[item_model_type::ROI].toDouble() / timeToExpiryWeeks;
    result[item_model_type::ROI_YEAR] = numWeeksPerYear * result[item_model_type::ROI_WEEK].toDouble();
    result[item_model_type::ROI_MONTH] = result[item_model_type::ROI_YEAR].toDouble() / 12.0;

    const double costBasis( equitySharePrice - (premium / multiplier) );

    if ( maxGain <= 0.0 )
        result[item_model_type::PROBABILITY_PROFIT] = 0.0;
    else if ( investmentValue < 0.0 )
        result[item_model_type::PROBABILITY_PROFIT] = 100.0;
    else
        result[item_model_type::PROBABILITY_PROFIT] = 100.0 * calcProbInTheMoney( costBasis, true );

    // ---- //

    // expected value
    //   prob. of max gain minus contract fees
    // - prob. of loss when selling assigned shares below strike (plus fees to sell those shares)
    double ev( itmProb * maxGain );
    ev -= calcExpectedLoss( multiplier, 0.0, strike, costBasis, itmProb, false );

    const double ev_roi( ev / investmentValue );

    result[item_model_type::EXPECTED_VALUE] = ev;
    result[item_model_type::EXPECTED_VALUE_ROI] = 100.0 * ev_roi;
    result[item_model_type::EXPECTED_VALUE_ROI_WEEK] = result[item_model_type::EXPECTED_VALUE_ROI].toDouble() / timeToExpiryWeeks;
    result[item_model_type::EXPECTED_VALUE_ROI_YEAR] = numWeeksPerYear * result[item_model_type::EXPECTED_VALUE_ROI_WEEK].toDouble();
    result[item_model_type::EXPECTED_VALUE_ROI_MONTH] = result[item_model_type::EXPECTED_VALUE_ROI_YEAR].toDouble() / 12.0;

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

    // ensure we have probabilities
    if ( !probCurve_.contains( strike ) )
        return;

    const double itmProb( calcProbInTheMoney( strike, false ) );
    const double otmProb( 1.0 - itmProb );

    result[item_model_type::PROBABILITY_ITM] = 100.0 * itmProb;
    result[item_model_type::PROBABILITY_OTM] = 100.0 * otmProb;

    double timeToExpiryWeeks( result[item_model_type::TIME_TO_EXPIRY].toDouble() );
    timeToExpiryWeeks *= AppDatabase::instance()->numDays();
    timeToExpiryWeeks /= 7.0;

    // ---- //

    const double numWeeksPerYear( AppDatabase::instance()->numDays() / 7.0 );

    const double multiplier( result[item_model_type::MULTIPLIER].toDouble() );

    const double theoOptionValue( result[item_model_type::CALC_THEO_OPTION_VALUE].toDouble() );
    const double mark( result[item_model_type::INVESTMENT_OPTION_PRICE].toDouble() );

    result[item_model_type::INVESTMENT_OPTION_PRICE_VS_THEO] = mark - theoOptionValue;                  // when selling, favorable when mark price above theo price

    // return on investment
    const double premium( (multiplier * mark) - optionTradeCost_ );                                     // how much you get for selling the option

    const double investmentValue( (multiplier * strike) - premium );                                    // how much money is locked up
    const double maxGain( premium );                                                                    // maximum amount of money you can possibly make
    const double maxLoss( investmentValue + equityTradeCost_ );                                         // maximum amount of money you can possibly lose (i.e. underlying goes to 0.00)

    result[item_model_type::INVESTMENT_AMOUNT] = investmentValue;
    result[item_model_type::PREMIUM_AMOUNT] = premium;
    result[item_model_type::MAX_GAIN] = maxGain;
    result[item_model_type::MAX_LOSS] = maxLoss;

    const double ror( maxGain / maxLoss );

    result[item_model_type::ROR] = 100.0 * ror;
    result[item_model_type::ROR_WEEK] = result[item_model_type::ROR].toDouble() / timeToExpiryWeeks;
    result[item_model_type::ROR_YEAR] = numWeeksPerYear * result[item_model_type::ROR_WEEK].toDouble();
    result[item_model_type::ROR_MONTH] = result[item_model_type::ROR_YEAR].toDouble() / 12.0;

    const double roi( premium / investmentValue );

    result[item_model_type::ROI] = 100.0 * roi;
    result[item_model_type::ROI_WEEK] = result[item_model_type::ROI].toDouble() / timeToExpiryWeeks;
    result[item_model_type::ROI_YEAR] = numWeeksPerYear * result[item_model_type::ROI_WEEK].toDouble();
    result[item_model_type::ROI_MONTH] = result[item_model_type::ROI_YEAR].toDouble() / 12.0;

    const double costBasis( maxLoss / multiplier );

    if ( maxGain <= 0.0 )
        result[item_model_type::PROBABILITY_PROFIT] = 0.0;
    else if ( investmentValue < 0.0 )
        result[item_model_type::PROBABILITY_PROFIT] = 100.0;
    else
        result[item_model_type::PROBABILITY_PROFIT] = 100.0 * calcProbInTheMoney( costBasis, true );

    // ---- //

    // expected value
    //   prob. of max gain minus contract fees
    // - prob. of loss when selling assigned shares below strike (plus fees to sell those shares)
    double ev( otmProb * maxGain );
    ev -= calcExpectedLoss( multiplier, 0.0, strike, costBasis, otmProb, false );

    const double ev_roi( ev / investmentValue );

    result[item_model_type::EXPECTED_VALUE] = ev;
    result[item_model_type::EXPECTED_VALUE_ROI] = 100.0 * ev_roi;
    result[item_model_type::EXPECTED_VALUE_ROI_WEEK] = result[item_model_type::EXPECTED_VALUE_ROI].toDouble() / timeToExpiryWeeks;
    result[item_model_type::EXPECTED_VALUE_ROI_YEAR] = numWeeksPerYear * result[item_model_type::EXPECTED_VALUE_ROI_WEEK].toDouble();
    result[item_model_type::EXPECTED_VALUE_ROI_MONTH] = result[item_model_type::EXPECTED_VALUE_ROI_YEAR].toDouble() / 12.0;

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

    // ensure we have probabilities
    if (( !probCurve_.contains( strikeLong ) ) || ( !probCurve_.contains( strikeShort )))
        return;

    const double breakEvenPrice( result[item_model_type::BREAK_EVEN_PRICE].toDouble() );

    const double itmProb( calcProbInTheMoney( breakEvenPrice, true ) );
    const double otmProb( 1.0 - itmProb );

    result[item_model_type::PROBABILITY_ITM] = 100.0 * itmProb;
    result[item_model_type::PROBABILITY_OTM] = 100.0 * otmProb;

    double timeToExpiryWeeks( result[item_model_type::TIME_TO_EXPIRY].toDouble() );
    timeToExpiryWeeks *= AppDatabase::instance()->numDays();
    timeToExpiryWeeks /= 7.0;

    // ---- //

    const double numWeeksPerYear( AppDatabase::instance()->numDays() / 7.0 );

    const double multiplier( result[item_model_type::MULTIPLIER].toDouble() );

    const double theoOptionValue( result[item_model_type::CALC_THEO_OPTION_VALUE].toDouble() );
    const double mark( result[item_model_type::INVESTMENT_OPTION_PRICE].toDouble() );

    result[item_model_type::INVESTMENT_OPTION_PRICE_VS_THEO] = mark - theoOptionValue;              // when selling, favorable when mark price above theo price

    // return on investment
    const double premium( (multiplier * mark) - (2.0 * optionTradeCost_) );                         // how much you get for selling the option
    const double spread( strikeLong - strikeShort );                                                // difference between long and short strikes

    const double investmentValue( (multiplier * spread) - premium );                                // how much money is locked up
    const double maxGain( premium );                                                                // maximum amount of money you can possibly make
    const double maxLoss( investmentValue + (2.0 * equityTradeCost_) );                             // maximum amount of money you can possibly lose (i.e. underlying goes to 0.00)

    result[item_model_type::INVESTMENT_AMOUNT] = investmentValue;
    result[item_model_type::PREMIUM_AMOUNT] = premium;
    result[item_model_type::MAX_GAIN] = maxGain;
    result[item_model_type::MAX_LOSS] = maxLoss;

    const double ror( maxGain / maxLoss );

    result[item_model_type::ROR] = 100.0 * ror;
    result[item_model_type::ROR_WEEK] = result[item_model_type::ROR].toDouble() / timeToExpiryWeeks;
    result[item_model_type::ROR_YEAR] = numWeeksPerYear * result[item_model_type::ROR_WEEK].toDouble();
    result[item_model_type::ROR_MONTH] = result[item_model_type::ROR_YEAR].toDouble() / 12.0;

    const double roi( premium / investmentValue );

    result[item_model_type::ROI] = 100.0 * roi;
    result[item_model_type::ROI_WEEK] = result[item_model_type::ROI].toDouble() / timeToExpiryWeeks;
    result[item_model_type::ROI_YEAR] = numWeeksPerYear * result[item_model_type::ROI_WEEK].toDouble();
    result[item_model_type::ROI_MONTH] = result[item_model_type::ROI_YEAR].toDouble() / 12.0;

    const double costBasis( strikeShort + ((premium - equityTradeCost_) / multiplier) );

    if ( maxGain <= 0.0 )
        result[item_model_type::PROBABILITY_PROFIT] = 0.0;
    else if ( investmentValue < 0.0 )
        result[item_model_type::PROBABILITY_PROFIT] = 100.0;
    else
        result[item_model_type::PROBABILITY_PROFIT] = 100.0 * (1.0 - calcProbInTheMoney( costBasis, true ));

    // ---- //

    const double itmProbLong( calcProbInTheMoney( strikeLong, true ) );
    const double itmProbShort( calcProbInTheMoney( strikeShort, true ) );
    const double otmProbShort( 1.0 - itmProbShort );

    // expected value
    //   prob. of max gain minus contract fees
    // - prob. of max loss plus fees to sell those shares
    // - prob. of loss having to buy shares in between strikes and sell them at loss
    // - prob. of fees to buy those shares
    double ev( otmProbShort * maxGain );
    ev -= itmProbLong * ((multiplier * spread) + equityTradeCost_);
    ev -= calcExpectedLoss( multiplier, strikeShort, strikeLong, strikeShort, (itmProbLong + otmProbShort), true );
    ev -= itmProbShort * equityTradeCost_;

    if ( investmentValue < 0.0 )
        ev *= -1.0;

    const double ev_roi( ev / investmentValue );

    result[item_model_type::EXPECTED_VALUE] = ev;
    result[item_model_type::EXPECTED_VALUE_ROI] = 100.0 * ev_roi;
    result[item_model_type::EXPECTED_VALUE_ROI_WEEK] = result[item_model_type::EXPECTED_VALUE_ROI].toDouble() / timeToExpiryWeeks;
    result[item_model_type::EXPECTED_VALUE_ROI_YEAR] = numWeeksPerYear * result[item_model_type::EXPECTED_VALUE_ROI_WEEK].toDouble();
    result[item_model_type::EXPECTED_VALUE_ROI_MONTH] = result[item_model_type::EXPECTED_VALUE_ROI_YEAR].toDouble() / 12.0;

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

    // ensure we have probabilities
    if (( !probCurve_.contains( strikeLong ) ) || ( !probCurve_.contains( strikeShort )))
        return;

    const double breakEvenPrice( result[item_model_type::BREAK_EVEN_PRICE].toDouble() );

    const double itmProb( calcProbInTheMoney( breakEvenPrice, false ) );
    const double otmProb( 1.0 - itmProb );

    result[item_model_type::PROBABILITY_ITM] = 100.0 * itmProb;
    result[item_model_type::PROBABILITY_OTM] = 100.0 * otmProb;

    double timeToExpiryWeeks( result[item_model_type::TIME_TO_EXPIRY].toDouble() );
    timeToExpiryWeeks *= AppDatabase::instance()->numDays();
    timeToExpiryWeeks /= 7.0;

    // ---- //

    const double numWeeksPerYear( AppDatabase::instance()->numDays() / 7.0 );

    const double multiplier( result[item_model_type::MULTIPLIER].toDouble() );

    const double theoOptionValue( result[item_model_type::CALC_THEO_OPTION_VALUE].toDouble() );
    const double mark( result[item_model_type::INVESTMENT_OPTION_PRICE].toDouble() );

    result[item_model_type::INVESTMENT_OPTION_PRICE_VS_THEO] = mark - theoOptionValue;              // when selling, favorable when mark price above theo price

    // return on investment
    const double premium( (multiplier * mark) - (2.0 * optionTradeCost_) );                         // how much you get for selling the option
    const double spread( strikeShort - strikeLong );                                                // difference between long and short strikes

    const double investmentValue( (multiplier * spread) - premium );                                // how much money is locked up
    const double maxGain( premium );                                                                // maximum amount of money you can possibly make
    const double maxLoss( investmentValue + (2.0 * equityTradeCost_) );                             // maximum amount of money you can possibly lose (i.e. underlying goes to 0.00)

    result[item_model_type::INVESTMENT_AMOUNT] = investmentValue;
    result[item_model_type::PREMIUM_AMOUNT] = premium;
    result[item_model_type::MAX_GAIN] = maxGain;
    result[item_model_type::MAX_LOSS] = maxLoss;

    const double ror( maxGain / maxLoss );

    result[item_model_type::ROR] = 100.0 * ror;
    result[item_model_type::ROR_WEEK] = result[item_model_type::ROR].toDouble() / timeToExpiryWeeks;
    result[item_model_type::ROR_YEAR] = numWeeksPerYear * result[item_model_type::ROR_WEEK].toDouble();
    result[item_model_type::ROR_MONTH] = result[item_model_type::ROR_YEAR].toDouble() / 12.0;

    const double roi( premium / investmentValue );

    result[item_model_type::ROI] = 100.0 * roi;
    result[item_model_type::ROI_WEEK] = result[item_model_type::ROI].toDouble() / timeToExpiryWeeks;
    result[item_model_type::ROI_YEAR] = numWeeksPerYear * result[item_model_type::ROI_WEEK].toDouble();
    result[item_model_type::ROI_MONTH] = result[item_model_type::ROI_YEAR].toDouble() / 12.0;

    const double costBasis( strikeShort - ((premium - equityTradeCost_) / multiplier) );

    if ( maxGain <= 0.0 )
        result[item_model_type::PROBABILITY_PROFIT] = 0.0;
    else if ( investmentValue < 0.0 )
        result[item_model_type::PROBABILITY_PROFIT] = 100.0;
    else
        result[item_model_type::PROBABILITY_PROFIT] = 100.0 * (1.0 - calcProbInTheMoney( costBasis, false ));

    // ---- //

    const double itmProbLong( calcProbInTheMoney( strikeLong, false ) );
    const double itmProbShort( calcProbInTheMoney( strikeShort, false ) );
    const double otmProbShort( 1.0 - itmProbShort );

    // expected value
    //   prob. of max gain minus contract fees
    // - prob. of max loss plus fees to sell those shares
    // - prob. of loss when selling assigned shares in between strikes
    // - prob. of fees to sell those shares
    double ev( otmProbShort * maxGain );
    ev -= itmProbLong * ((multiplier * spread) + equityTradeCost_);
    ev -= calcExpectedLoss( multiplier, strikeLong, strikeShort, costBasis, (itmProbLong + otmProbShort), false );
    ev -= itmProbShort * equityTradeCost_;

    if ( investmentValue < 0.0 )
        ev *= -1.0;

    const double ev_roi( ev / investmentValue );

    result[item_model_type::EXPECTED_VALUE] = ev;
    result[item_model_type::EXPECTED_VALUE_ROI] = 100.0 * ev_roi;
    result[item_model_type::EXPECTED_VALUE_ROI_WEEK] = result[item_model_type::EXPECTED_VALUE_ROI].toDouble() / timeToExpiryWeeks;
    result[item_model_type::EXPECTED_VALUE_ROI_YEAR] = numWeeksPerYear * result[item_model_type::EXPECTED_VALUE_ROI_WEEK].toDouble();
    result[item_model_type::EXPECTED_VALUE_ROI_MONTH] = result[item_model_type::EXPECTED_VALUE_ROI_YEAR].toDouble() / 12.0;

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

        assert( 0.0 < result.timeToExpiry );

        // get risk free interest rate
        result.riskFreeRate = riskFreeRate_;

        // generate VI for bid, ask, mark prices
        AbstractOptionPricing *o( createPricingMethod( underlying_, result.riskFreeRate, result.riskFreeRate, 0.0, result.timeToExpiry, divTimes_, div_ ) );

        result.bidvi = calcImplVol( o, type, strike, bid );
        result.askvi = calcImplVol( o, type, strike, ask );
        result.markvi = calcImplVol( o, type, strike, mark );

        // check for unrealistic volatility
        if ( 0.0 < result.askvi )
        {
            if (( 0.0 < result.bidvi ) && ( result.askvi < result.bidvi ))
                result.bidvi = 0.0;

            if (( 0.0 < result.markvi ) && ( result.askvi < result.markvi ))
                result.markvi = 0.0;
        }

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

            AbstractOptionPricing *o( createPricingMethod( underlying_, g.riskFreeRate, g.riskFreeRate, 0.0, g.timeToExpiry, divTimes_, div_ ) );

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

        AbstractOptionPricing *o( createPricingMethod( underlying_, g.riskFreeRate, g.riskFreeRate, c.vi, g.timeToExpiry, divTimes_, div_ ) );

        c.price = o->optionPrice( type, strike );

        // validate price
        if (( std::isnan( c.price ) || ( std::isinf( c.price ) ) || ( c.price < 0.0 )))
        {
            destroyPricingMethod( o );

            LOG_ERROR << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << " " << (isCall ? "CALL" : "PUT") << " failed to calc option price";
            return false;
        }

        if ( !init )
            init = true;
        else if ( prev.price < c.price )
            c.price = prev.price;

        // calculate greeks
        if ( !calcGreeks( o, c.price, strike, isCall, g ) )
        {
            destroyPricingMethod( o );

            LOG_ERROR << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << " " << (isCall ? "CALL" : "PUT") << " failed to calc greeks";
            return false;
        }

        c.delta = g.delta;

        g.marketPrice = c.min + ((c.max - c.min) / 2.0);

        if ( g.ask < g.marketPrice )
            g.marketPrice = g.ask;
        else if ( g.marketPrice < g.bid )
            g.marketPrice = g.bid;

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

    // invalid min vi
    if ( c.minvi <= 0.0 )
    {
        AbstractOptionPricing *o( createPricingMethod( underlying_, g.riskFreeRate, g.riskFreeRate, 0.0001, g.timeToExpiry, divTimes_, div_ ) );

        // determine value for min
        const double v( std::ceil( 100.0 * o->optionPrice( type, strike ) ) / 100.0 );

        if (( !std::isinf( v ) ) && ( !std::isnan( v ) ) && ( v <= c.max ))
            c.minvi = calcImplVol( o, type, strike, (c.min = v) );

        destroyPricingMethod( o );

        // invalid min vi
        if ( c.minvi <= 0.0 )
        {
            LOG_WARN << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << " " << (isCall ? "CALL" : "PUT") << " invalid min " << c.min << " " << c.minvi;
            return false;
        }
    }

    // invalid max vi
    if ( c.maxvi <= 0.0 )
    {
        double vi( c.minvi );
        double a( 100.0 );

        AbstractOptionPricing *o( createPricingMethod( underlying_, g.riskFreeRate, g.riskFreeRate, 0.0, g.timeToExpiry, divTimes_, div_ ) );

        // keep increasing vi until we get a valid max
        double max( 0.0 );

        for ( ;; )
        {
            vi += a;

            // set new vi
            o->setSigma( vi );

            const double v( std::floor( 100.0 * o->optionPrice( type, strike ) ) / 100.0 );

            // found new value
            if (( !std::isinf( v ) ) && ( !std::isnan( v ) ) && ( v < c.max ) && ( vi <= 1000.0 ))
                max = v;

            // reduce
            else
            {
                vi -= a;
                a /= 10.0;

                if ( a < 0.00005 )
                    break;
            }
        }

        // valid max, use it
        if ( 0.0 < max )
            c.maxvi = calcImplVol( o, type, strike, (c.max = max) );

        destroyPricingMethod( o );

        // invalid max vi
        if ( c.maxvi <= 0.0 )
        {
            LOG_WARN << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << " " << (isCall ? "CALL" : "PUT") << " invalid max " << c.max << " " << c.maxvi;
            return false;
        }
    }

    if (( c.min < 0.0 ) || ( c.max < 0.0 ))
    {
        LOG_ERROR << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << " " << (isCall ? "CALL" : "PUT") << " negative min/max";
        return false;
    }
    else if (( 0.0 < c.max ) && ( c.max < c.min ))
    {
        LOG_ERROR << qPrintable( chains_->symbol() ) << " " << daysToExpiry_ << " " << strike << " " << (isCall ? "CALL" : "PUT") << " inverted min/max";
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

    AbstractOptionPricing *o( createPricingMethod( underlying_, g.riskFreeRate, g.riskFreeRate, 0.0, g.timeToExpiry, divTimes_, div_ ) );

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
double ExpectedValueCalculator::calcExpectedLoss( double multiplier, double priceMin, double priceMax, double costBasis, double totalProb, bool isCall ) const
{
    // when call flag set we are buying at probability and selling at cost basis
    const double z( isCall ? -1.0 : 1.0 );

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

            assert( 0.0 <= probDelta );

            // accumulate loss at this possibility
            loss += multiplier * probDelta * z * (costBasis - price);
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

        assert( 0.0 <= probDelta );

        // accumulate loss at this possibility
        loss += multiplier * probDelta * z * (costBasis - price);
        totalProb += probDelta;
    }

    assert(( 0.999 <= totalProb ) && ( totalProb <= 1.001 ));
    return loss;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ExpectedValueCalculator::populateResultModelGreeks( const Greeks& g, item_model_type::ColumnValueMap &result )
{
    result[item_model_type::BID_ASK_SPREAD] = g.spread;
    result[item_model_type::BID_ASK_SPREAD_PERCENT] = 100.0 * g.spreadPercent;

    result[item_model_type::TIME_TO_EXPIRY] = g.timeToExpiry;
    result[item_model_type::RISK_FREE_INTEREST_RATE] = 100.0 * g.riskFreeRate;

    result[item_model_type::CALC_BID_PRICE_VI] = 100.0 * g.bidvi;
    result[item_model_type::CALC_ASK_PRICE_VI] = 100.0 * g.askvi;
    result[item_model_type::CALC_MARK_VI] = 100.0 * g.markvi;

    result[item_model_type::CALC_THEO_OPTION_VALUE] = g.price;
    result[item_model_type::CALC_THEO_VOLATILITY] = 100.0 * g.vi;
    result[item_model_type::CALC_DELTA] = g.delta;
    result[item_model_type::CALC_GAMMA] = g.gamma;
    result[item_model_type::CALC_THETA] = g.theta;
    result[item_model_type::CALC_VEGA] = g.vega;
    result[item_model_type::CALC_RHO] = g.rho;

    result[item_model_type::INVESTMENT_OPTION_PRICE] = g.marketPrice;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ExpectedValueCalculator::populateResultModelGreeksSpread( const Greeks& glong, const Greeks& gshort, item_model_type::ColumnValueMap &result )
{
    const double bid( gshort.bid - glong.ask );
    const double ask( gshort.ask - glong.bid );

    result[item_model_type::BID_ASK_SPREAD] = ask - bid;
    result[item_model_type::BID_ASK_SPREAD_PERCENT] = 100.0 * (( ask - bid ) / ask);

    result[item_model_type::TIME_TO_EXPIRY] = gshort.timeToExpiry;
    result[item_model_type::RISK_FREE_INTEREST_RATE] = 100.0 * gshort.riskFreeRate;

    // net volatility
    // https://en.wikipedia.org/wiki/Net_volatility#:~:text=Net%20volatility%20refers%20to%20the,implied%20volatility%20of%20the%20spread.
    double bidviNet = ((glong.vega * glong.bidvi) - (gshort.vega * gshort.bidvi)) / (glong.vega - gshort.vega);
    double askviNet = ((glong.vega * glong.askvi) - (gshort.vega * gshort.askvi)) / (glong.vega - gshort.vega);
    double markviNet = ((glong.vega * glong.markvi) - (gshort.vega * gshort.markvi)) / (glong.vega - gshort.vega);

    result[item_model_type::CALC_BID_PRICE_VI] = 100.0 * bidviNet;
    result[item_model_type::CALC_ASK_PRICE_VI] = 100.0 * askviNet;
    result[item_model_type::CALC_MARK_VI] = 100.0 * markviNet;

    // net volatility
    // https://en.wikipedia.org/wiki/Net_volatility#:~:text=Net%20volatility%20refers%20to%20the,implied%20volatility%20of%20the%20spread.
    double viNet = ((glong.vega * glong.vi) - (gshort.vega * gshort.vi)) / (glong.vega - gshort.vega);

    result[item_model_type::CALC_THEO_OPTION_VALUE] = gshort.price - glong.price;
    result[item_model_type::CALC_THEO_VOLATILITY] = 100.0 * viNet;
    result[item_model_type::CALC_DELTA] = glong.delta - gshort.delta;
    result[item_model_type::CALC_GAMMA] = glong.gamma - gshort.gamma;
    result[item_model_type::CALC_THETA] = glong.theta - gshort.theta;
    result[item_model_type::CALC_VEGA] = glong.vega - gshort.vega;
    result[item_model_type::CALC_RHO] = glong.rho - gshort.rho;

    result[item_model_type::INVESTMENT_OPTION_PRICE] = gshort.marketPrice - glong.marketPrice;
}
