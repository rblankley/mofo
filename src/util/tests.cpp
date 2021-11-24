/**
 * @file tests.cpp
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

#include "baroneadesiwhaley.h"
#include "bisection.h"
#include "bjerksundstensland.h"
#include "blackscholes.h"
#include "coxrossrubinstein.h"
#include "equalprobbinomial.h"
#include "montecarlo.h"
#include "newtonraphson.h"
#include "phelimboyle.h"
#include "rollgeskewhaley.h"
#include "tests.h"

#include <common.h>

#include <QDateTime>
#include <QStringList>

#if defined( QT_DEBUG )

void cbnd_validate();

///////////////////////////////////////////////////////////////////////////////////////////////////
void validateOptionPricing()
{
    cbnd_validate();

    BaroneAdesiWhaley::validate();
    BjerksundStensland::validate();
    BinomialTree::validate();
    BlackScholes::validate();
    CoxRossRubinstein::validate();
    EqualProbBinomialTree::validate();
    MonteCarlo::validate();
    NewtonRaphson::validate();
    PhelimBoyle::validate();
    RollGeskeWhaley::validate();

    double S = 9.98;                // Spot Price
    double K0 = 9.5;                // Strike Price
    double T = 9.0 / 365.25;        // Years to maturity
    double r = 0.01;                // Risk Free Rate
    double q = 0.0;

    double v_call = 0.3569;
    double op_call = 0.55;

    double v_put = 0.3788;
    double op_put = 0.08;

    const QString format8( "%1 %2 %3 %4 %5 %6 %7 %8" );
    const QString format5( "%1 %2 %3 %4 %5" );

    QStringList results;

    {
        BaroneAdesiWhaley baw( S, r, r-q, v_call, T );
        BjerksundStensland bjs( S, r, r-q, v_call, T );
        BlackScholes bs( S, r, r-q, v_call, T );
        CoxRossRubinstein crr( S, r, r-q, v_call, T, 1000 );
        EqualProbBinomialTree eqpb( S, r, r-q, v_call, T, 1000 );
        MonteCarlo mc( S, r, r-q, v_call, T, 10000000 );
        PhelimBoyle pb( S, r, r-q, v_call, T, 500 );
        RollGeskeWhaley rgw( S, r, v_call, T, 0.0, T );

        results << format8
            .arg( "Call Price", 10 )
            .arg( K0-6.0, 12 )
            .arg( K0-4.0, 12 )
            .arg( K0-2.0, 12 )
            .arg( K0    , 12 )
            .arg( K0+2.0, 12 )
            .arg( K0+4.0, 12 )
            .arg( K0+6.0, 12 );

        results << format8
            .arg( "BAW", 10 )
            .arg( baw.optionPrice( OptionType::Call, K0-6.0 ), 12 )
            .arg( baw.optionPrice( OptionType::Call, K0-4.0 ), 12 )
            .arg( baw.optionPrice( OptionType::Call, K0-2.0 ), 12 )
            .arg( baw.optionPrice( OptionType::Call, K0     ), 12 )
            .arg( baw.optionPrice( OptionType::Call, K0+2.0 ), 12 )
            .arg( baw.optionPrice( OptionType::Call, K0+4.0 ), 12 )
            .arg( baw.optionPrice( OptionType::Call, K0+6.0 ), 12 );

        results << format8
            .arg( "BJS", 10 )
            .arg( bjs.optionPrice( OptionType::Call, K0-6.0 ), 12 )
            .arg( bjs.optionPrice( OptionType::Call, K0-4.0 ), 12 )
            .arg( bjs.optionPrice( OptionType::Call, K0-2.0 ), 12 )
            .arg( bjs.optionPrice( OptionType::Call, K0     ), 12 )
            .arg( bjs.optionPrice( OptionType::Call, K0+2.0 ), 12 )
            .arg( bjs.optionPrice( OptionType::Call, K0+4.0 ), 12 )
            .arg( bjs.optionPrice( OptionType::Call, K0+6.0 ), 12 );

        results << format8
            .arg( "BS", 10 )
            .arg( bs.optionPrice( OptionType::Call, K0-6.0 ), 12 )
            .arg( bs.optionPrice( OptionType::Call, K0-4.0 ), 12 )
            .arg( bs.optionPrice( OptionType::Call, K0-2.0 ), 12 )
            .arg( bs.optionPrice( OptionType::Call, K0     ), 12 )
            .arg( bs.optionPrice( OptionType::Call, K0+2.0 ), 12 )
            .arg( bs.optionPrice( OptionType::Call, K0+4.0 ), 12 )
            .arg( bs.optionPrice( OptionType::Call, K0+6.0 ), 12 );

        results << format8
            .arg( "CRR Binom", 10 )
            .arg( crr.optionPrice( OptionType::Call, K0-6.0 ), 12 )
            .arg( crr.optionPrice( OptionType::Call, K0-4.0 ), 12 )
            .arg( crr.optionPrice( OptionType::Call, K0-2.0 ), 12 )
            .arg( crr.optionPrice( OptionType::Call, K0     ), 12 )
            .arg( crr.optionPrice( OptionType::Call, K0+2.0 ), 12 )
            .arg( crr.optionPrice( OptionType::Call, K0+4.0 ), 12 )
            .arg( crr.optionPrice( OptionType::Call, K0+6.0 ), 12 );

        results << format8
            .arg( "EQP Binom", 10 )
            .arg( eqpb.optionPrice( OptionType::Call, K0-6.0 ), 12 )
            .arg( eqpb.optionPrice( OptionType::Call, K0-4.0 ), 12 )
            .arg( eqpb.optionPrice( OptionType::Call, K0-2.0 ), 12 )
            .arg( eqpb.optionPrice( OptionType::Call, K0     ), 12 )
            .arg( eqpb.optionPrice( OptionType::Call, K0+2.0 ), 12 )
            .arg( eqpb.optionPrice( OptionType::Call, K0+4.0 ), 12 )
            .arg( eqpb.optionPrice( OptionType::Call, K0+6.0 ), 12 );

        results << format8
            .arg( "PB Trinom", 10 )
            .arg( pb.optionPrice( OptionType::Call, K0-6.0 ), 12 )
            .arg( pb.optionPrice( OptionType::Call, K0-4.0 ), 12 )
            .arg( pb.optionPrice( OptionType::Call, K0-2.0 ), 12 )
            .arg( pb.optionPrice( OptionType::Call, K0     ), 12 )
            .arg( pb.optionPrice( OptionType::Call, K0+2.0 ), 12 )
            .arg( pb.optionPrice( OptionType::Call, K0+4.0 ), 12 )
            .arg( pb.optionPrice( OptionType::Call, K0+6.0 ), 12 );

        results << format8
            .arg( "RGW", 10 )
            .arg( rgw.optionPrice( OptionType::Call, K0-6.0 ), 12 )
            .arg( rgw.optionPrice( OptionType::Call, K0-4.0 ), 12 )
            .arg( rgw.optionPrice( OptionType::Call, K0-2.0 ), 12 )
            .arg( rgw.optionPrice( OptionType::Call, K0     ), 12 )
            .arg( rgw.optionPrice( OptionType::Call, K0+2.0 ), 12 )
            .arg( rgw.optionPrice( OptionType::Call, K0+4.0 ), 12 )
            .arg( rgw.optionPrice( OptionType::Call, K0+6.0 ), 12 );

        results << format8
            .arg( "M Carlo", 10 )
            .arg( mc.optionPrice( OptionType::Call, K0-6.0 ), 12 )
            .arg( mc.optionPrice( OptionType::Call, K0-4.0 ), 12 )
            .arg( mc.optionPrice( OptionType::Call, K0-2.0 ), 12 )
            .arg( mc.optionPrice( OptionType::Call, K0     ), 12 )
            .arg( mc.optionPrice( OptionType::Call, K0+2.0 ), 12 )
            .arg( mc.optionPrice( OptionType::Call, K0+4.0 ), 12 )
            .arg( mc.optionPrice( OptionType::Call, K0+6.0 ), 12 );

        results << "";
    }

    {
        BaroneAdesiWhaley baw( S, r, r-q, v_put, T );
        BjerksundStensland bjs( S, r, r-q, v_put, T );
        BlackScholes bs( S, r, r-q, v_put, T );
        CoxRossRubinstein crr( S, r, r-q, v_put, T, 1000 );
        EqualProbBinomialTree eqpb( S, r, r-q, v_put, T, 1000 );
        MonteCarlo mc( S, r, r-q, v_put, T, 10000000 );
        PhelimBoyle pb( S, r, r-q, v_put, T, 500 );
        RollGeskeWhaley rgw( S, r, v_put, T, 0.0, T );

        results << format8
            .arg( "Put Price", 10 )
            .arg( K0-6.0, 12 )
            .arg( K0-4.0, 12 )
            .arg( K0-2.0, 12 )
            .arg( K0    , 12 )
            .arg( K0+2.0, 12 )
            .arg( K0+4.0, 12 )
            .arg( K0+6.0, 12 );

        results << format8
            .arg( "BAW", 10 )
            .arg( baw.optionPrice( OptionType::Put, K0-6.0 ), 12 )
            .arg( baw.optionPrice( OptionType::Put, K0-4.0 ), 12 )
            .arg( baw.optionPrice( OptionType::Put, K0-2.0 ), 12 )
            .arg( baw.optionPrice( OptionType::Put, K0     ), 12 )
            .arg( baw.optionPrice( OptionType::Put, K0+2.0 ), 12 )
            .arg( baw.optionPrice( OptionType::Put, K0+4.0 ), 12 )
            .arg( baw.optionPrice( OptionType::Put, K0+6.0 ), 12 );

        results << format8
            .arg( "BJS", 10 )
            .arg( bjs.optionPrice( OptionType::Put, K0-6.0 ), 12 )
            .arg( bjs.optionPrice( OptionType::Put, K0-4.0 ), 12 )
            .arg( bjs.optionPrice( OptionType::Put, K0-2.0 ), 12 )
            .arg( bjs.optionPrice( OptionType::Put, K0     ), 12 )
            .arg( bjs.optionPrice( OptionType::Put, K0+2.0 ), 12 )
            .arg( bjs.optionPrice( OptionType::Put, K0+4.0 ), 12 )
            .arg( bjs.optionPrice( OptionType::Put, K0+6.0 ), 12 );

        results << format8
            .arg( "BS", 10 )
            .arg( bs.optionPrice( OptionType::Put, K0-6.0 ), 12 )
            .arg( bs.optionPrice( OptionType::Put, K0-4.0 ), 12 )
            .arg( bs.optionPrice( OptionType::Put, K0-2.0 ), 12 )
            .arg( bs.optionPrice( OptionType::Put, K0     ), 12 )
            .arg( bs.optionPrice( OptionType::Put, K0+2.0 ), 12 )
            .arg( bs.optionPrice( OptionType::Put, K0+4.0 ), 12 )
            .arg( bs.optionPrice( OptionType::Put, K0+6.0 ), 12 );

        results << format8
            .arg( "CRR Binom", 10 )
            .arg( crr.optionPrice( OptionType::Put, K0-6.0 ), 12 )
            .arg( crr.optionPrice( OptionType::Put, K0-4.0 ), 12 )
            .arg( crr.optionPrice( OptionType::Put, K0-2.0 ), 12 )
            .arg( crr.optionPrice( OptionType::Put, K0     ), 12 )
            .arg( crr.optionPrice( OptionType::Put, K0+2.0 ), 12 )
            .arg( crr.optionPrice( OptionType::Put, K0+4.0 ), 12 )
            .arg( crr.optionPrice( OptionType::Put, K0+6.0 ), 12 );

        results << format8
            .arg( "EQP Binom", 10 )
            .arg( eqpb.optionPrice( OptionType::Put, K0-6.0 ), 12 )
            .arg( eqpb.optionPrice( OptionType::Put, K0-4.0 ), 12 )
            .arg( eqpb.optionPrice( OptionType::Put, K0-2.0 ), 12 )
            .arg( eqpb.optionPrice( OptionType::Put, K0     ), 12 )
            .arg( eqpb.optionPrice( OptionType::Put, K0+2.0 ), 12 )
            .arg( eqpb.optionPrice( OptionType::Put, K0+4.0 ), 12 )
            .arg( eqpb.optionPrice( OptionType::Put, K0+6.0 ), 12 );

        results << format8
            .arg( "PB Trinom", 10 )
            .arg( pb.optionPrice( OptionType::Put, K0-6.0 ), 12 )
            .arg( pb.optionPrice( OptionType::Put, K0-4.0 ), 12 )
            .arg( pb.optionPrice( OptionType::Put, K0-2.0 ), 12 )
            .arg( pb.optionPrice( OptionType::Put, K0     ), 12 )
            .arg( pb.optionPrice( OptionType::Put, K0+2.0 ), 12 )
            .arg( pb.optionPrice( OptionType::Put, K0+4.0 ), 12 )
            .arg( pb.optionPrice( OptionType::Put, K0+6.0 ), 12 );

        results << format8
            .arg( "RGW", 10 )
            .arg( rgw.optionPrice( OptionType::Put, K0-6.0 ), 12 )
            .arg( rgw.optionPrice( OptionType::Put, K0-4.0 ), 12 )
            .arg( rgw.optionPrice( OptionType::Put, K0-2.0 ), 12 )
            .arg( rgw.optionPrice( OptionType::Put, K0     ), 12 )
            .arg( rgw.optionPrice( OptionType::Put, K0+2.0 ), 12 )
            .arg( rgw.optionPrice( OptionType::Put, K0+4.0 ), 12 )
            .arg( rgw.optionPrice( OptionType::Put, K0+6.0 ), 12 );

        results << format8
            .arg( "M Carlo", 10 )
            .arg( mc.optionPrice( OptionType::Put, K0-6.0 ), 12 )
            .arg( mc.optionPrice( OptionType::Put, K0-4.0 ), 12 )
            .arg( mc.optionPrice( OptionType::Put, K0-2.0 ), 12 )
            .arg( mc.optionPrice( OptionType::Put, K0     ), 12 )
            .arg( mc.optionPrice( OptionType::Put, K0+2.0 ), 12 )
            .arg( mc.optionPrice( OptionType::Put, K0+4.0 ), 12 )
            .arg( mc.optionPrice( OptionType::Put, K0+6.0 ), 12 );

        results << "";
    }

    {
        BaroneAdesiWhaley baw( S, r, r-q, 0.0, T );
        BjerksundStensland bjs( S, r, r-q, 0.0, T );
        BlackScholes bs( S, r, r-q, 0.0, T );
        CoxRossRubinstein crr( S, r, r-q, 0.0, T, 1000 );
        EqualProbBinomialTree eqpb( S, r, r-q, 0.0, T, 1000 );
        MonteCarlo mc( S, r, r-q, 0.0, T, 10000000 );
        PhelimBoyle pb( S, r, r-q, 0.0, T, 500 );
        RollGeskeWhaley rgw( S, r, 0.0, T, 0.0, T );

        results << format5
            .arg( "IV", 10 )
            .arg( "Bi Call", 14 )
            .arg( "NR Call", 14 )
            .arg( "Bi Put", 14 )
            .arg( "NR Put", 14 );

        results << format5
            .arg( "BAW", 10 )
            .arg( Bisection::calcImplVol( &baw, OptionType::Call, K0, op_call ), 14 )
            .arg( NewtonRaphson::calcImplVol( &baw, OptionType::Call, K0, op_call ), 14 )
            .arg( Bisection::calcImplVol( &baw, OptionType::Put, K0, op_put ), 14 )
            .arg( NewtonRaphson::calcImplVol( &baw, OptionType::Put, K0, op_put ), 14 );

        results << format5
            .arg( "BJS", 10 )
            .arg( Bisection::calcImplVol( &bjs, OptionType::Call, K0, op_call ), 14 )
            .arg( NewtonRaphson::calcImplVol( &bjs, OptionType::Call, K0, op_call ), 14 )
            .arg( Bisection::calcImplVol( &bjs, OptionType::Put, K0, op_put ), 14 )
            .arg( NewtonRaphson::calcImplVol( &bjs, OptionType::Put, K0, op_put ), 14 );

        results << format5
            .arg( "BS", 10 )
            .arg( Bisection::calcImplVol( &bs, OptionType::Call, K0, op_call ), 14 )
            .arg( NewtonRaphson::calcImplVol( &bs, OptionType::Call, K0, op_call ), 14 )
            .arg( Bisection::calcImplVol( &bs, OptionType::Put, K0, op_put ), 14 )
            .arg( NewtonRaphson::calcImplVol( &bs, OptionType::Put, K0, op_put ), 14 );

        results << format5
            .arg( "CRR Binom", 10 )
            .arg( Bisection::calcImplVol( &crr, OptionType::Call, K0, op_call ), 14 )
            .arg( NewtonRaphson::calcImplVol( &crr, OptionType::Call, K0, op_call ), 14 )
            .arg( Bisection::calcImplVol( &crr, OptionType::Put, K0, op_put ), 14 )
            .arg( NewtonRaphson::calcImplVol( &crr, OptionType::Put, K0, op_put ), 14 );

        results << format5
            .arg( "EQP Binom", 10 )
            .arg( Bisection::calcImplVol( &eqpb, OptionType::Call, K0, op_call ), 14 )
            .arg( NewtonRaphson::calcImplVol( &eqpb, OptionType::Call, K0, op_call ), 14 )
            .arg( Bisection::calcImplVol( &eqpb, OptionType::Put, K0, op_put ), 14 )
            .arg( NewtonRaphson::calcImplVol( &eqpb, OptionType::Put, K0, op_put ), 14 );

        results << format5
            .arg( "PB Trinom", 10 )
            .arg( Bisection::calcImplVol( &pb, OptionType::Call, K0, op_call ), 14 )
            .arg( NewtonRaphson::calcImplVol( &pb, OptionType::Call, K0, op_call ), 14 )
            .arg( Bisection::calcImplVol( &pb, OptionType::Put, K0, op_put ), 14 )
            .arg( NewtonRaphson::calcImplVol( &pb, OptionType::Put, K0, op_put ), 14 );

        results << format5
            .arg( "RGW", 10 )
            .arg( Bisection::calcImplVol( &rgw, OptionType::Call, K0, op_call ), 14 )
            .arg( NewtonRaphson::calcImplVol( &rgw, OptionType::Call, K0, op_call ), 14 )
            .arg( Bisection::calcImplVol( &rgw, OptionType::Put, K0, op_put ), 14 )
            .arg( NewtonRaphson::calcImplVol( &rgw, OptionType::Put, K0, op_put ), 14 );
/*
        results << format5
            .arg( "M Carlo", 10 )
            .arg( Bisection::calcImplVol( &mc, OptionType::Call, K0, op_call ), 14 )
            .arg( NewtonRaphson::calcImplVol( &mc, OptionType::Call, K0, op_call ), 14 )
            .arg( Bisection::calcImplVol( &mc, OptionType::Put, K0, op_put ), 14 )
            .arg( NewtonRaphson::calcImplVol( &mc, OptionType::Put, K0, op_put ), 14 );
*/
        results << "";
    }

    // ---- //

    foreach ( const QString& result, results )
        LOG_ERROR << qPrintable( result );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void optionPricingPerf( size_t loops )
{
    double S = 9.98;             // Spot Price
    double K0 = 9.5;             // Strike Price
    double T = 9.0 / 365.25;     // Years to maturity
    double r = 0.01;             // Risk Free Rate
    double q = 0.0;

    double v_call = 0.3569;
    //double op_call = 0.55;

    double v_put = 0.3788;
    //double op_put = 0.08;

    QDateTime dt;

    dt = QDateTime::currentDateTime();

    for ( size_t n( loops ); n--; )
    {
        CoxRossRubinstein crr_call( S, r, r-q, v_call, T, 500 );
        crr_call.optionPrice( OptionType::Call, K0 );

        CoxRossRubinstein crr_put( S, r, r-q, v_put, T, 500 );
        crr_put.optionPrice( OptionType::Put, K0 );
    }

    LOG_ERROR << "time N=500 " << dt.msecsTo( QDateTime::currentDateTime() );

    dt = QDateTime::currentDateTime();

    for ( size_t n( loops ); n--; )
    {
        CoxRossRubinstein crr_call( S, r, r-q, v_call, T, 10'000 );
        crr_call.optionPrice( OptionType::Call, K0 );

        CoxRossRubinstein crr_put( S, r, r-q, v_put, T, 10'000 );
        crr_put.optionPrice( OptionType::Put, K0 );
    }

    LOG_ERROR << "time N=10k " << dt.msecsTo( QDateTime::currentDateTime() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void calculatePartials()
{
    double S = 26.82;           // Spot Price
    double X = 28.5;             // Strike Price
    double T = 7.0 / 365.25;     // Years to maturity
    double r = 0.1;             // Risk Free Rate
    double q = 0.0;

    double vi;
    double delta;
    double gamma;
    double theta;
    double vega;
    double rho;

    //double vi_bid;
    //double vi_ask;
    //double vi_mid;
    bool okay;

    CoxRossRubinstein crr( S, r, r-q, 0.0, T, 1024 );

    //vi_bid = NewtonRaphson::calcImplVol( &crr, OptionType::Call, X, 0.26, &okay );
    //vi_ask = NewtonRaphson::calcImplVol( &crr, OptionType::Call, X, 0.30, &okay );
    //vi_mid = vi_bid + (vi_ask - vi_bid) / 2.0;
    vi = NewtonRaphson::calcImplVol( &crr, OptionType::Call, X, 0.28, &okay );

    if ( okay )
    {
        crr.partials( OptionType::Call, X, delta, gamma, theta, vega, rho );
        theta /= 365.25;
        vega /= 100.0;
        rho /= 100.0;

        LOG_ERROR << "CRR Call " << vi << " " << delta << " " << gamma << " " << theta << " " << vega << " " << rho;
    }

    //vi_bid = NewtonRaphson::calcImplVol( &crr, OptionType::Put, X, 1.91, &okay );
    //vi_ask = NewtonRaphson::calcImplVol( &crr, OptionType::Put, X, 2.09, &okay );
    //vi_mid = vi_bid + (vi_ask - vi_bid) / 2.0;
    vi = NewtonRaphson::calcImplVol( &crr, OptionType::Put, X, 1.985, &okay );

    if ( okay )
    {
        crr.partials( OptionType::Put, X, delta, gamma, theta, vega, rho );
        theta /= 365.25;
        vega /= 100.0;
        rho /= 100.0;

        LOG_ERROR << "CRR Put " << vi << " " << delta << " " << gamma << " " << theta << " " << vega << " " << rho;
    }

    BlackScholes bs( S, r, r-q, 0.0, T );

    //vi_bid = NewtonRaphson::calcImplVol( &bs, OptionType::Call, X, 0.26, &okay );
    //vi_ask = NewtonRaphson::calcImplVol( &bs, OptionType::Call, X, 0.30, &okay );
    //vi_mid = vi_bid + (vi_ask - vi_bid) / 2.0;
    vi = NewtonRaphson::calcImplVol( &bs, OptionType::Call, X, 0.28, &okay );

    if ( okay )
    {
        bs.partials( OptionType::Call, X, delta, gamma, theta, vega, rho );
        theta /= 365.25;
        vega /= 100.0;
        rho /= 100.0;

        LOG_ERROR << "BS Call " << vi << " " << delta << " " << gamma << " " << theta << " " << vega << " " << rho;
    }

    //vi_bid = NewtonRaphson::calcImplVol( &bs, OptionType::Put, X, 1.91, &okay );
    //vi_ask = NewtonRaphson::calcImplVol( &bs, OptionType::Put, X, 2.09, &okay );
    //vi_mid = vi_bid + (vi_ask - vi_bid) / 2.0;
    vi = NewtonRaphson::calcImplVol( &bs, OptionType::Put, X, 1.985, &okay );

    if ( okay )
    {
        bs.partials( OptionType::Put, X, delta, gamma, theta, vega, rho );
        theta /= 365.25;
        vega /= 100.0;
        rho /= 100.0;

        LOG_ERROR << "BS Put " << vi << " " << delta << " " << gamma << " " << theta << " " << vega << " " << rho;
    }
}

#endif
