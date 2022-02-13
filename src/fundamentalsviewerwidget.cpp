/**
 * @file fundamentalsviewerwidget.cpp
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
#include "fundamentalsviewerwidget.h"

#include "db/appdb.h"
#include "db/fundamentalstablemodel.h"

#include <cmath>

#include <QFormLayout>
#include <QLabel>
#include <QLocale>
#include <QSpacerItem>
#include <QVBoxLayout>

///////////////////////////////////////////////////////////////////////////////////////////////////
FundamentalsViewerWidget::FundamentalsViewerWidget( const QString& symbol, double price, QWidget *parent ) :
    _Mybase( parent ),
    symbol_( symbol ),
    price_( price )
{
    // setup model
    model_ = new model_type( symbol, QDateTime(), this );

    connect( model_, &model_type::dataChanged, this, &_Myt::refreshData );
    connect( model_, &model_type::modelReset, this, &_Myt::refreshData );

    // init
    initialize();
    createLayout();
    translate();

    // refresh model
    model_->refreshData();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
FundamentalsViewerWidget::~FundamentalsViewerWidget()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FundamentalsViewerWidget::translate()
{
    avgVolumeLabel_->setText( tr( "Avg Vol (10-day)" ) );
    yearRangeLabel_->setText( tr( "52-Wk Range" ) );
    percentBelowHighLabel_->setText( tr( "% Below High" ) );
    divLabel_->setText( tr( "Annual Dividend/Yield" ) );
    divDateLabel_->setText( tr( "Ex-Dividend Date" ) );
    divPayDateLabel_->setText( tr( "Dividend Pay Date" ) );
    betaLabel_->setText( tr( "Beta" ) );
    shortIntLabel_->setText( tr( "Short Interest" ) );

    // share values
    epsLabel_->setText( tr( "Earnings Per Share - TTM" ) );
    dpsLabel_->setText( tr( "Dividends Per Share - TTM" ) );
    bpsLabel_->setText( tr( "Book Value Per Share" ) );
    cfpsLabel_->setText( tr( "Cash Flow Per Share" ) );
    fcfpsLabel_->setText( tr( "Free Cash Flow Per Share" ) );
    spsLabel_->setText( tr( "Sales Per Share" ) );

    // profitability
    roeLabel_->setText( tr( "Return on Equity (ROE)" ) );
    roaLabel_->setText( tr( "Return on Assets (ROA)" ) );
    grossProfitMarginLabel_->setText( tr( "Gross Profit Margin" ) );
    operProfitMarginLabel_->setText( tr( "Operating Profit Margin" ) );
    taxRateLabel_->setText( tr( "Tax Rate" ) );
    intRateLabel_->setText( tr( "Interest Rate - Estimated Average" ) );
    netProfitMarginLabel_->setText( tr( "Net Profit Margin" ) );

    // activity ratios
    totalAssetTurnoverLabel_->setText( tr( "Total Asset Turnover" ) );
    inventoryTurnoverLabel_->setText( tr( "Inventory Turnover" ) );

    // financial ratios
    ltDebtToCapitalLabel_->setText( tr( "Long-term Debt to Capital" ) );
    financialLeverageLabel_->setText( tr( "Financial Leverage (Assets/Equity)" ) );
    fixedChargeCoverageRatioLabel_->setText( tr( "Fixed Charge Coverage Ratio" ) );
    divPayoutRatioLabel_->setText( tr( "Dividend Payout (% of Earnings)" ) );
    quickRatioLabel_->setText( tr( "Quick Ratio" ) );
    currentRatioLabel_->setText( tr( "Current Ratio" ) );

    // valuation
    peRatioLabel_->setText( tr( "Price / Earnings Ratio" ) );
    pcfRatioLabel_->setText( tr( "Price / Cash Flow Ratio" ) );
    pbRatioLabel_->setText( tr( "Price / Book Value Ratio" ) );
    marketCapRatioLabel_->setText( tr( "Market Capitalization / Common Equity Ratio" ) );
    divYieldLabel_->setText( tr( "Dividend Yield" ) );
    divPayoutPerShareLabel_->setText( tr( "Dividend Payout Per Share (% of EPS)" ) );

    sharesOutstandingLabel_->setText( tr( "Total Shares Outstanding" ) );
    marketCapLabel_->setText( tr( "Market Cap" ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FundamentalsViewerWidget::refreshData()
{
    if ( !model_->rowCount() )
        return;

    const QLocale l( QLocale::system() );

    // ---- //

    double temp;

    const double marketCap( 1000000.0 * model_->tableData( model_type::MARKET_CAP ).toDouble() );
    const unsigned long long sharesOutstanding( model_->tableData( model_type::SHARES_OUTSTANDING ).toULongLong() );

    // compute price per share (as per api)
    double pricePerShare( 0.0 );
    double pricePerShareCount( 0.0 );

    // compute from P/E ratio
    temp = model_->tableData( model_type::PE_RATIO ).toDouble() * model_->tableData( model_type::EPS_TTM ).toDouble();

    if ( std::isnormal( temp ) )
    {
        pricePerShare += temp;
        pricePerShareCount += 1.0;
    }
/*
    // do *NOT* use P/B ratio... the book value per share we are given is wrong
    temp = model_->tableData( model_type::PB_RATIO ).toDouble() * model_->tableData( model_type::BOOK_VALUE_PER_SHARE ).toDouble();

    if ( std::isnormal( temp ) )
    {
        pricePerShare += temp;
        pricePerShareCount += 1.0;
    }
*/
    // compute from PEG ratio
    temp = model_->tableData( model_type::PEG_RATIO ).toDouble() * model_->tableData( model_type::EPS_CHANGE_PERCENT_TTM ).toDouble() * model_->tableData( model_type::EPS_TTM ).toDouble();

    if ( std::isnormal( temp ) )
    {
        pricePerShare += temp;
        pricePerShareCount += 1.0;
    }

    // if all else fails, use passed in market price
    if ( 0.0 < pricePerShareCount )
        pricePerShare /= pricePerShareCount;
    else
        pricePerShare = price_;

    LOG_DEBUG << "share price " << qPrintable( symbol() ) << " " << pricePerShare << " (current market share price " << price_ << ")";

    const double earningsPerShare( pricePerShare / model_->tableData( model_type::PE_RATIO ).toDouble() );
    const double bookValuePerShare( pricePerShare / model_->tableData( model_type::PB_RATIO ).toDouble() );
    const double revenuePerShare( pricePerShare / model_->tableData( model_type::PR_RATIO ).toDouble() );

    // compute revenue
    double totalRevenue( 0.0 );
    totalRevenue += revenuePerShare * sharesOutstanding;
    totalRevenue += marketCap / model_->tableData( model_type::PR_RATIO ).toDouble();
    totalRevenue /= 2.0;

    const double grossProfit( model_->tableData( model_type::GROSS_MARGIN_TTM ).toDouble() * totalRevenue / 100.0 );
    const double costOfRevenue( totalRevenue - grossProfit );

    // net income continuous operations
    const double netIncome( model_->tableData( model_type::NET_PROFIT_MARGIN_TTM ).toDouble() * totalRevenue / 100.0 );

    // total operating income as reported
    const double totalOperatingIncome( model_->tableData( model_type::OPERATING_MARGIN_TTM ).toDouble() * totalRevenue / 100.0 );

    // earnings
    const double earnings( model_->tableData( model_type::EPS_TTM ).toDouble() * sharesOutstanding );

    // preferred stock dividends
    const double preferredStockDividends( netIncome - earnings );

    // net income common stockholders
    const double netIncomeCommonStockholders( netIncome - preferredStockDividends );

    LOG_DEBUG << "total revenue " << qPrintable( symbol() ) << " " << qPrintable( l.toString( totalRevenue / 1000.0, 'f', 0 ) );
    LOG_DEBUG << "cost of revenue " << qPrintable( symbol() ) << " " << qPrintable( l.toString( costOfRevenue / 1000.0, 'f', 0 ) );
    LOG_DEBUG << "gross profit " << qPrintable( symbol() ) << " " << qPrintable( l.toString( grossProfit / 1000.0, 'f', 0 ) );

    LOG_DEBUG << "net income common stockholders " << qPrintable( symbol() ) << " " << qPrintable( l.toString( netIncomeCommonStockholders / 1000.0, 'f', 0 ) );
    LOG_DEBUG << "  net income " << qPrintable( symbol() ) << " " << qPrintable( l.toString( netIncome / 1000.0, 'f', 0 ) );
    LOG_DEBUG << "  preferred stock dividends " << qPrintable( symbol() ) << " " << qPrintable( l.toString( preferredStockDividends / 1000.0, 'f', 0 ) );
    LOG_DEBUG << "earnings " << qPrintable( symbol() ) << " " << qPrintable( l.toString( earnings / 1000.0, 'f', 0 ) );

    LOG_DEBUG << "total operating income (reported) " << qPrintable( symbol() ) << " " << qPrintable( l.toString( totalOperatingIncome / 1000.0, 'f', 0 ) );

    // total assets
    const double totalAssets( netIncome / (model_->tableData( model_type::RETURN_ON_ASSETS ).toDouble() / 100.0) );

    // shareholders equity
    const double shareholdersEquity( netIncomeCommonStockholders / (model_->tableData( model_type::RETURN_ON_EQUITY ).toDouble() / 100.0) );

    // total liabilities
    const double totalLiabilities( totalAssets - shareholdersEquity );

    // compute total debt (total liabilities)
    const double totalDebt( (model_->tableData( model_type::TOTAL_DEBT_TO_EQUITY ).toDouble() / 100.0) * shareholdersEquity );
    const double longTermDebt( (model_->tableData( model_type::LT_DEBT_TO_EQUITY ).toDouble() / 100.0) * shareholdersEquity );
    const double currentDebt( totalDebt - longTermDebt );

    const double totalCapital( totalDebt / (model_->tableData( model_type::TOTAL_DEBT_TO_CAPITAL ).toDouble() / 100.0) );
    const double interestBearingDebt( totalCapital - shareholdersEquity );

    const double currentLiabilities( totalLiabilities - interestBearingDebt );
    const double currentAssets( model_->tableData( model_type::CURRENT_RATIO ).toDouble() * currentLiabilities );

    const double inventoryAndPrepaidExpenses( currentAssets - model_->tableData( model_type::QUICK_RATIO ).toDouble() * currentLiabilities );

    const double longTermDeptToCapital( longTermDebt / totalCapital );

    LOG_DEBUG << "total assets " << qPrintable( symbol() ) << " " << qPrintable( l.toString( totalAssets / 1000.0, 'f', 0 ) );
    LOG_DEBUG << "  current assets " << qPrintable( symbol() ) << " " << qPrintable( l.toString( currentAssets / 1000.0, 'f', 0 ) );
    LOG_DEBUG << "    inventory " << qPrintable( symbol() ) << " " << qPrintable( l.toString( inventoryAndPrepaidExpenses / 1000.0, 'f', 0 ) );
    LOG_DEBUG << "total liabilities " << qPrintable( symbol() ) << " " << qPrintable( l.toString( totalLiabilities / 1000.0, 'f', 0 ) );
    LOG_DEBUG << "  current liabilities " << qPrintable( symbol() ) << " " << qPrintable( l.toString( currentLiabilities / 1000.0, 'f', 0 ) );
    LOG_DEBUG << "    current debt " << qPrintable( symbol() ) << " " << qPrintable( l.toString( currentDebt / 1000.0, 'f', 0 ) );
    LOG_DEBUG << "  non-current liabilities " << qPrintable( symbol() ) << " " << qPrintable( l.toString( interestBearingDebt / 1000.0, 'f', 0 ) );
    LOG_DEBUG << "    long term debt " << qPrintable( symbol() ) << " " << qPrintable( l.toString( longTermDebt / 1000.0, 'f', 0 ) );
    LOG_DEBUG << "total capitalization " << qPrintable( symbol() ) << " " << qPrintable( l.toString( totalCapital / 1000.0, 'f', 0 ) );
    LOG_DEBUG << "common stock equity " << qPrintable( symbol() ) << " " << qPrintable( l.toString( shareholdersEquity / 1000.0, 'f', 0 ) );
    LOG_DEBUG << "total debt " << qPrintable( symbol() ) << " " << qPrintable( l.toString( totalDebt / 1000.0, 'f', 0 ) );

    // cash flow
    const double operatingCashFlow( (pricePerShare * sharesOutstanding) / model_->tableData( model_type::PCF_RATIO ).toDouble() );

    const double cashFlowPerShare( (operatingCashFlow - preferredStockDividends) / sharesOutstanding );

    // ---- //

    setLabelText( avgVolume_,
        l.toString( model_->tableData( model_type::VOL_10DAY_AVG ).toULongLong() ),
        "0" );

    setLabelText( yearRange_,
        QString( "%0 - %1" )
            .arg( l.toString( model_->tableData( model_type::LOW52 ).toDouble(), 'f', 2 ) )
            .arg( l.toString( model_->tableData( model_type::HIGH52 ).toDouble(), 'f', 2 ) ),
        "0.00 - 0.00" );

    setLabelText( percentBelowHigh_,
        l.toString( 100.0 * (1.0 - (price_ / model_->tableData( model_type::HIGH52 ).toDouble())), 'f', 2 ),
        "0.00" );

    setLabelText( div_,
        QString( "%0/%1%" )
            .arg( l.toString( model_->tableData( model_type::DIV_AMOUNT ).toDouble(), 'f', 2 ) )
            .arg( l.toString( model_->tableData( model_type::DIV_YIELD ).toDouble(), 'f', 2 ) ),
        "0.00/0.00%" );

    const QDateTime divDate( QDateTime::fromString( model_->tableData( model_type::DIV_DATE ).toString(), Qt::ISODateWithMs ) );
    const QString divFreq( model_->tableData( model_type::DIV_FREQUENCY ).toString() );

    QString divDateStr( divDate.date().toString() );

    if ( divFreq.length() )
        divDateStr.append( QString( " (%0)" ).arg( divFreq ) );

    setLabelText( divDate_, divDateStr );

    const QDateTime divPayDate( QDateTime::fromString( model_->tableData( model_type::DIV_PAY_DATE ).toString(), Qt::ISODateWithMs ) );
    const QString divPayDateStr( divPayDate.date().toString() );

    setLabelText( divPayDate_, divPayDateStr );

    setLabelText( beta_,
        l.toString( model_->tableData( model_type::BETA ).toDouble(), 'f', 2 ),
        "0.00" );

    setLabelText( shortInt_,
        l.toString( model_->tableData( model_type::SHORT_INT_TO_FLOAT ).toDouble(), 'f', 2 ),
        "0.00" );

    //
    // share values
    //

    setLabelText( eps_,
        l.toString( model_->tableData( model_type::EPS_TTM ).toDouble(), 'f', 2 ),
        "0.00" );

    setLabelText( dps_,
        l.toString( model_->tableData( model_type::DIV_AMOUNT ).toDouble(), 'f', 2 ),
        "0.00" );

    setLabelText( bps_,
        l.toString( bookValuePerShare, 'f', 2 ),
        "0.00" );

    setLabelText( cfps_,
        l.toString( cashFlowPerShare, 'f', 2 ),
        "0.00" );

    setLabelText( sps_,
        l.toString( revenuePerShare, 'f', 2 ),
        "0.00" );

    //
    // profitability
    //

    setLabelText( roe_,
        l.toString( model_->tableData( model_type::RETURN_ON_EQUITY ).toDouble(), 'f', 2 ),
        "0.00" );

    setLabelText( roa_,
        l.toString( model_->tableData( model_type::RETURN_ON_ASSETS ).toDouble(), 'f', 2 ),
        "0.00" );

    setLabelText( grossProfitMargin_,
        l.toString( model_->tableData( model_type::GROSS_MARGIN_TTM ).toDouble(), 'f', 2 ),
        "0.00" );

    setLabelText( operProfitMargin_,
        l.toString( model_->tableData( model_type::OPERATING_MARGIN_TTM ).toDouble(), 'f', 2 ),
        "0.00" );

    setLabelText( netProfitMargin_,
        l.toString( model_->tableData( model_type::NET_PROFIT_MARGIN_TTM ).toDouble(), 'f', 2 ),
        "0.00" );

    //
    // activity ratios
    //

    //
    // financial ratios
    //

    setLabelText( ltDebtToCapital_,
        l.toString( longTermDeptToCapital * 100.0, 'f', 4 ),
        "0.0000" );

    setLabelText( financialLeverage_,
        l.toString( totalAssets / shareholdersEquity, 'f', 4 ),
        "0.0000" );

    setLabelText( divPayoutRatio_,
        l.toString( 100.0 * model_->tableData( model_type::DIV_AMOUNT ).toDouble() * sharesOutstanding / netIncomeCommonStockholders, 'f', 4 ),
        "0.0000" );

    setLabelText( quickRatio_,
        l.toString( model_->tableData( model_type::QUICK_RATIO ).toDouble(), 'f', 4 ),
        "0.0000" );

    setLabelText( currentRatio_,
        l.toString( model_->tableData( model_type::CURRENT_RATIO ).toDouble(), 'f', 4 ),
        "0.0000" );

    //
    // valuation
    //

    setLabelText( peRatio_,
        l.toString( model_->tableData( model_type::PE_RATIO ).toDouble(), 'f', 4 ),
        "0.0000" );

    setLabelText( pcfRatio_,
        l.toString( model_->tableData( model_type::PCF_RATIO ).toDouble(), 'f', 4 ),
        "0.0000" );

    setLabelText( pbRatio_,
        l.toString( model_->tableData( model_type::PB_RATIO ).toDouble(), 'f', 4 ),
        "0.0000" );

    setLabelText( marketCapRatio_,
        l.toString( marketCap / shareholdersEquity, 'f', 4 ),
        "0.0000" );

    setLabelText( divYield_,
        l.toString( (100.0 * model_->tableData( model_type::DIV_AMOUNT ).toDouble()) / pricePerShare ),
        "0.0000" );

    setLabelText( divPayoutPerShare_,
        l.toString( (100.0 * model_->tableData( model_type::DIV_AMOUNT ).toDouble()) / earningsPerShare ),
        "0.0000" );

    setLabelText( sharesOutstanding_,
        l.toString( model_->tableData( model_type::SHARES_OUTSTANDING ).toULongLong() ),
        "0" );

    setLabelText( marketCap_,
        QString( "%0 M" )
            .arg( l.toString( (unsigned int) std::round( model_->tableData( model_type::MARKET_CAP ).toDouble() ) ) ),
        "0 M" );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FundamentalsViewerWidget::initialize()
{
    avgVolumeLabel_ = new QLabel( this );
    avgVolume_ = new QLabel( this );

    yearRangeLabel_ = new QLabel( this );
    yearRange_ = new QLabel( this );

    percentBelowHighLabel_ = new QLabel( this );
    percentBelowHigh_ = new QLabel( this );

    divLabel_ = new QLabel( this );
    div_ = new QLabel( this );

    divDateLabel_ = new QLabel( this );
    divDate_ = new QLabel( this );

    divPayDateLabel_ = new QLabel( this );
    divPayDate_ = new QLabel( this );

    betaLabel_ = new QLabel( this );
    beta_ = new QLabel( this );

    shortIntLabel_ = new QLabel( this );
    shortInt_ = new QLabel( this );

    // share values
    epsLabel_ = new QLabel( this );
    eps_ = new QLabel( this );

    dpsLabel_ = new QLabel( this );
    dps_ = new QLabel( this );

    bpsLabel_ = new QLabel( this );
    bps_ = new QLabel( this );

    cfpsLabel_ = new QLabel( this );
    cfps_ = new QLabel( this );

    fcfpsLabel_ = new QLabel( this );
    fcfps_ = new QLabel( this );

    spsLabel_ = new QLabel( this );
    sps_ = new QLabel( this );

    // profitability
    roeLabel_ = new QLabel( this );
    roe_ = new QLabel( this );

    roaLabel_ = new QLabel( this );
    roa_ = new QLabel( this );

    grossProfitMarginLabel_ = new QLabel( this );
    grossProfitMargin_ = new QLabel( this );

    operProfitMarginLabel_ = new QLabel( this );
    operProfitMargin_ = new QLabel( this );

    taxRateLabel_ = new QLabel( this );
    taxRate_ = new QLabel( this );

    intRateLabel_ = new QLabel( this );
    intRate_ = new QLabel( this );

    netProfitMarginLabel_ = new QLabel( this );
    netProfitMargin_ = new QLabel( this );

    // activity ratios
    totalAssetTurnoverLabel_ = new QLabel( this );
    totalAssetTurnover_ = new QLabel( this );

    inventoryTurnoverLabel_ = new QLabel( this );
    inventoryTurnover_ = new QLabel( this );

    // financial ratios
    ltDebtToCapitalLabel_ = new QLabel( this );
    ltDebtToCapital_ = new QLabel( this );

    financialLeverageLabel_ = new QLabel( this );
    financialLeverage_ = new QLabel( this );

    fixedChargeCoverageRatioLabel_ = new QLabel( this );
    fixedChargeCoverageRatio_ = new QLabel( this );

    divPayoutRatioLabel_ = new QLabel( this );
    divPayoutRatio_ = new QLabel( this );

    quickRatioLabel_ = new QLabel( this );
    quickRatio_ = new QLabel( this );

    currentRatioLabel_ = new QLabel( this );
    currentRatio_ = new QLabel( this );

    // valuation
    peRatioLabel_ = new QLabel( this );
    peRatio_ = new QLabel( this );

    pcfRatioLabel_ = new QLabel( this );
    pcfRatio_ = new QLabel( this );

    pbRatioLabel_ = new QLabel( this );
    pbRatio_ = new QLabel( this );

    marketCapRatioLabel_ = new QLabel( this );
    marketCapRatio_ = new QLabel( this );

    divYieldLabel_ = new QLabel( this );
    divYield_ = new QLabel( this );

    divPayoutPerShareLabel_ = new QLabel( this );
    divPayoutPerShare_ = new QLabel( this );

    sharesOutstandingLabel_ = new QLabel( this );
    sharesOutstanding_ = new QLabel( this );

    marketCapLabel_ = new QLabel( this );
    marketCap_ = new QLabel( this );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FundamentalsViewerWidget::createLayout()
{
    QFormLayout *fields( new QFormLayout() );
    fields->setContentsMargins( QMargins() );

    fields->addRow( avgVolumeLabel_, avgVolume_ );
    fields->addRow( yearRangeLabel_, yearRange_ );
    fields->addRow( percentBelowHighLabel_, percentBelowHigh_ );
    fields->addRow( divLabel_, div_ );
    fields->addRow( divDateLabel_, divDate_ );
    fields->addRow( divPayDateLabel_, divPayDate_ );
    fields->addRow( betaLabel_, beta_ );
    fields->addRow( shortIntLabel_, shortInt_ );

    //
    // share values
    //
    fields->addItem( new QSpacerItem( 16, 16 ) );
    fields->addRow( epsLabel_, eps_ );
    fields->addRow( dpsLabel_, dps_ );
    fields->addRow( bpsLabel_, bps_ );
    fields->addRow( cfpsLabel_, cfps_ );

    // hide fields that are not computable
    //fields->addRow( fcfpsLabel_, fcfps_ );
    fcfpsLabel_->hide();
    fcfps_->hide();

    fields->addRow( spsLabel_, sps_ );

    //
    // profitability
    //
    fields->addItem( new QSpacerItem( 16, 16 ) );
    fields->addRow( roeLabel_, roe_ );
    fields->addRow( roaLabel_, roa_ );
    fields->addRow( grossProfitMarginLabel_, grossProfitMargin_ );
    fields->addRow( operProfitMarginLabel_, operProfitMargin_ );

    // hide fields that are not computable
    //fields->addRow( taxRateLabel_, taxRate_ );
    taxRateLabel_->hide();
    taxRate_->hide();

    // hide fields that are not computable
    //fields->addRow( intRateLabel_, intRate_ );
    intRateLabel_->hide();
    intRate_->hide();

    fields->addRow( netProfitMarginLabel_, netProfitMargin_ );

    //
    // profitability
    //

    // hide fields that are not computable
    //fields->addItem( new QSpacerItem( 16, 16 ) );

    // hide fields that are not computable
    //fields->addRow( totalAssetTurnoverLabel_, totalAssetTurnover_ );
    totalAssetTurnoverLabel_->hide();
    totalAssetTurnover_->hide();

    // hide fields that are not computable
    //fields->addRow( inventoryTurnoverLabel_, inventoryTurnover_ );
    inventoryTurnoverLabel_->hide();
    inventoryTurnover_->hide();

    //
    // financial ratios
    //
    fields->addItem( new QSpacerItem( 16, 16 ) );
    fields->addRow( ltDebtToCapitalLabel_, ltDebtToCapital_ );
    fields->addRow( financialLeverageLabel_, financialLeverage_ );

    // hide fields that are not computable
    //fields->addRow( fixedChargeCoverageRatioLabel_, fixedChargeCoverageRatio_ );
    fixedChargeCoverageRatioLabel_->hide();
    fixedChargeCoverageRatio_->hide();

    fields->addRow( divPayoutRatioLabel_, divPayoutRatio_ );
    fields->addRow( quickRatioLabel_, quickRatio_ );
    fields->addRow( currentRatioLabel_, currentRatio_ );

    //
    // valuation
    //
    fields->addItem( new QSpacerItem( 16, 16 ) );
    fields->addRow( peRatioLabel_, peRatio_ );
    fields->addRow( pcfRatioLabel_, pcfRatio_ );
    fields->addRow( pbRatioLabel_, pbRatio_ );
    fields->addRow( marketCapRatioLabel_, marketCapRatio_ );
    fields->addRow( divYieldLabel_, divYield_ );
    fields->addRow( divPayoutPerShareLabel_, divPayoutPerShare_ );

    fields->addItem( new QSpacerItem( 16, 16 ) );
    fields->addRow( sharesOutstandingLabel_, sharesOutstanding_ );
    fields->addRow( marketCapLabel_, marketCap_ );

    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->addLayout( fields );
    form->addStretch();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FundamentalsViewerWidget::setLabelText( QLabel *l, const QString text, const QString emptyText )
{
    const bool isEmpty( text == emptyText );

    if (( isEmpty ) || ( "inf" == text ) || ( "nan" == text ))
        l->setText( "-" );
    else
        l->setText( text );

    l->setAlignment( Qt::AlignCenter );
}
