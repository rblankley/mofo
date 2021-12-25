/**
 * @file configdialog.cpp
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
#include "configdialog.h"
#include "filtersdialog.h"
#include "watchlistselectiondialog.h"

#include "db/appdb.h"

#include <QColorDialog>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

static const QString EQUITY_REFRESH_RATE( "equityRefreshRate" );
static const QString EQUITY_TRADE_COST( "equityTradeCost" );
static const QString EQUITY_TRADE_COST_NON_EXCHANGE( "equityTradeCostNonExchange" );
static const QString EQUITY_WATCH_LISTS( "equityWatchLists" );

static const QString HISTORY( "history" );
static const QString MARKET_TYPES( "marketTypes" );
static const QString NUM_DAYS( "numDays" );
static const QString NUM_TRADING_DAYS( "numTradingDays" );

static const QString OPTION_CHAIN_REFRESH_RATE( "optionChainRefreshRate" );
static const QString OPTION_CHAIN_EXPIRY_END_DATE( "optionChainExpiryEndDate" );
static const QString OPTION_CHAIN_WATCH_LISTS( "optionChainWatchLists" );
static const QString OPTION_TRADE_COST( "optionTradeCost" );
static const QString OPTION_CALC_METHOD( "optionCalcMethod" );

static const QString OPTION_ANALYSIS_FILTER( "optionAnalysisFilter" );

static const QString PALETTE( "palette" );
static const QString PALETTE_HIGHLIGHT( "paletteHighlight" );

///////////////////////////////////////////////////////////////////////////////////////////////////
ConfigurationDialog::ConfigurationDialog( QWidget *parent, Qt::WindowFlags f ) :
    _Mybase( parent, f )
{
    int i;

    // remove the question mark button
    setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    // init
    initialize();
    createLayout();
    translate();

    // retrieve config
    configs_ = AppDatabase::instance()->configs();

    history_->setText( configs_[HISTORY].toString() );
    marketTypes_->setText( configs_[MARKET_TYPES].toString() );
    numDays_->setText( configs_[NUM_DAYS].toString() );
    numTradingDays_->setText( configs_[NUM_TRADING_DAYS].toString() );
    palette_->setCurrentIndex( palette_->findData( configs_[PALETTE].toString() ) );
    paletteHighlight_->setText( configs_[PALETTE_HIGHLIGHT].toString() );

    equityRefreshRate_->setText( configs_[EQUITY_REFRESH_RATE].toString() );
    equityTradeCost_->setText( configs_[EQUITY_TRADE_COST].toString() );
    equityTradeCostNonExchange_->setText( configs_[EQUITY_TRADE_COST_NON_EXCHANGE].toString() );
    equityWatchLists_->setText( configs_[EQUITY_WATCH_LISTS].toString() );

    optionChainRefreshRate_->setText( configs_[OPTION_CHAIN_REFRESH_RATE].toString() );
    optionChainExpiryEndDate_->setText( configs_[OPTION_CHAIN_EXPIRY_END_DATE].toString() );
    optionChainWatchLists_->setText( configs_[OPTION_CHAIN_WATCH_LISTS].toString() );
    optionTradeCost_->setText( configs_[OPTION_TRADE_COST].toString() );

    if ( 0 <= (i = optionCalcMethod_->findData( configs_[OPTION_CALC_METHOD].toString() )) )
        optionCalcMethod_->setCurrentIndex( i );

    if ( 0 <= (i = optionAnalysisFilter_->findData( configs_[OPTION_ANALYSIS_FILTER].toString() )) )
        optionAnalysisFilter_->setCurrentIndex( i );

    // set focus to first widget
    history_->setFocus();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSize ConfigurationDialog::sizeHint() const
{
    return QSize( 800, 600 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ConfigurationDialog::translate()
{
    setWindowTitle( tr( "Configuration" ) );

    equityRefreshRateLabel_->setText( tr( "Equity Refresh Time (minutes)" ) );
    equityRefreshRate_->setToolTip( tr( "How often to refresh equity data. Zero to disable." ) );

    equityTradeCostLabel_->setText( tr( "Equity Trading Cost" ) );
    equityTradeCost_->setToolTip( tr( "Cost to trade an exchange traded fund." ) );

    equityTradeCostNonExchangeLabel_->setText( tr( "Equity Trading Cost, Non-Exchange" ) );
    equityTradeCostNonExchange_->setToolTip( tr( "Cost to trade a non-exchange traded fund." ) );

    equityWatchListsLabel_->setText( tr( "Equity Watchlists (comma separated)" ) );
    equityWatchLists_->setToolTip( tr( "Watchlist(s) of symbols to refresh." ) );
    equityWatchListsDialog_->setText( "..." );

    historyLabel_->setText( tr( "Keep History (days)" ) );
    history_->setToolTip( tr( "How much API historical information to keep. Zero to keep everything." ) );

    marketTypesLabel_->setText( tr( "Market Types (comma separated)" ) );
    marketTypes_->setToolTip( tr( "Market types to fetch information for (i.e. hours of operation)." ) );

    numDaysLabel_->setText( tr( "Number of Days in a Year (days)" ) );
    numDays_->setToolTip( tr( "How many days are in a year. Used for annualization and partials calculation." ) );

    numTradingDaysLabel_->setText( tr( "Number of Trading Days in a Year (days)" ) );
    numTradingDays_->setToolTip( tr( "How many trading days are in a year. Used for annualization and partials calculation." ) );

    paletteLabel_->setText( tr( "Color Scheme" ) );
    palette_->setItemText( 0, tr( "System" ) );
    palette_->setItemText( 1, tr( "Dark" ) );
    palette_->setItemText( 2, tr( "Light" ) );
    palette_->setToolTip( tr( "Which application color palette scheme to use. Requires restart of application to take effect." ) );

    paletteHighlightLabel_->setText( tr( "Selection and Highlight Color" ) );
    paletteHighlight_->setToolTip( tr( "Color to use for selection and highlights. Requires restart of application to take effect." ) );
    paletteHighlightDialog_->setText( "..." );

    optionChainRefreshRateLabel_->setText( tr( "Option Chain Refresh Time (minutes)" ) );
    optionChainRefreshRate_->setToolTip( tr( "How often to refresh option chains. Zero to disable." ) );

    optionChainExpiryEndDateLabel_->setText( tr( "Option Chain Expiration End (days)" ) );
    optionChainExpiryEndDate_->setToolTip( tr( "Maximum option chain expiration to retrieve. Expiration dates past this are not retrieved." ) );

    optionChainWatchListsLabel_->setText( tr( "Option Chain Watchlists (comma separated)" ) );
    optionChainWatchLists_->setToolTip( tr( "Watchlist(s) of symbols to refresh." ) );
    optionChainWatchListsDialog_->setText( "..." );

    optionTradeCostLabel_->setText( tr( "Option Trading Cost" ) );
    optionTradeCost_->setToolTip( tr( "Cost to trade an option contract." ) );

    optionCalcMethodLabel_->setText( tr( "Option Pricing Calculation Method" ) );
    optionCalcMethod_->setItemText( 0, tr( "Barone-Adesi and Whaley" ) );
    optionCalcMethod_->setItemText( 1, tr( "Binomial Tree (Cox Ross Rubinstein)" ) );
    optionCalcMethod_->setItemText( 2, tr( "Binomial Tree (Equal Probability)" ) );
    optionCalcMethod_->setItemText( 3, tr( "Bjerksund and Stensland (1993)" ) );
    optionCalcMethod_->setItemText( 4, tr( "Bjerksund and Stensland (2002)" ) );
    optionCalcMethod_->setItemText( 5, tr( "Black Scholes" ) );
    optionCalcMethod_->setItemText( 6, tr( "Monte Carlo" ) );
    optionCalcMethod_->setItemText( 7, tr( "Trinomial Tree (Phelim Boyle)" ) );
    optionCalcMethod_->setItemText( 8, tr( "Trinomial Tree (Alternative)" ) );
    optionCalcMethod_->setItemText( 9, tr( "Trinomial Tree (Kamrad Ritchken)" ) );
    optionCalcMethod_->setToolTip( tr( "Which option pricing methodology to use for analysis." ) );

    optionAnalysisFilterLabel_->setText( tr( "Option Analysis Filtering Method" ) );
    optionAnalysisFilter_->setItemText( 0, tr( "NONE" ) );
    optionAnalysisFilterDialog_->setText( "..." );

    okay_->setText( tr( "Okay" ) );
    cancel_->setText( tr( "Cancel" ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ConfigurationDialog::onButtonClicked()
{
    // color picker
    if ( paletteHighlightDialog_ == sender() )
    {
        QColor c;
        c.setNamedColor( paletteHighlight_->text() );

        QColorDialog d( c, this );

        if ( QDialog::Accepted == d.exec() )
            paletteHighlight_->setText( d.currentColor().name() );
    }

    // equity watchlists
    else if ( equityWatchListsDialog_ == sender() )
    {
        WatchlistSelectionDialog d( this );
        d.setSelected( equityWatchLists_->text() );

        if ( QDialog::Accepted == d.exec() )
            equityWatchLists_->setText( d.selected() );
    }

    // option chain watchlists
    else if ( optionChainWatchListsDialog_ == sender() )
    {
        WatchlistSelectionDialog d( this );
        d.setSelected( optionChainWatchLists_->text() );

        if ( QDialog::Accepted == d.exec() )
            optionChainWatchLists_->setText( d.selected() );
    }

    // filters
    else if ( optionAnalysisFilterDialog_ == sender() )
    {
        // save off existing selection
        const QString s( optionAnalysisFilter_->currentData().toString() );

        // edit
        FiltersDialog d( this );
        d.setSelected( s );
        d.setCancelButtonVisible( true );

        // prompt
        const int rc( d.exec() );

        // remove existing filters and add new ones
        while ( 1 < optionAnalysisFilter_->count() )
            optionAnalysisFilter_->removeItem( optionAnalysisFilter_->count() - 1 );

        foreach ( const QString& f, AppDatabase::instance()->filters() )
            optionAnalysisFilter_->addItem( f, f );

        // set back to existing selection; or the selected filter if they accepted the dialog
        int i;

        if ( 0 <= (i = optionAnalysisFilter_->findData( (QDialog::Accepted == rc) ? d.selected() : s )) )
            optionAnalysisFilter_->setCurrentIndex( i );
    }

    // okay
    else if ( okay_ == sender() )
    {
        saveForm();
        accept();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ConfigurationDialog::initialize()
{
    equityRefreshRateLabel_ = new QLabel( this );
    equityRefreshRate_ = new QLineEdit( this );

    equityTradeCostLabel_ = new QLabel( this );
    equityTradeCost_ = new QLineEdit( this );

    equityTradeCostNonExchangeLabel_ = new QLabel( this );
    equityTradeCostNonExchange_ = new QLineEdit( this );

    equityWatchListsLabel_ = new QLabel( this );
    equityWatchLists_ = new QLineEdit( this );
    equityWatchListsDialog_ = new QPushButton( this );

    connect( equityWatchListsDialog_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    historyLabel_ = new QLabel( this );
    history_ = new QLineEdit( this );

    marketTypesLabel_ = new QLabel( this );
    marketTypes_ = new QLineEdit( this );

    numDaysLabel_ = new QLabel( this );
    numDays_ = new QLineEdit( this );

    numTradingDaysLabel_ = new QLabel( this );
    numTradingDays_ = new QLineEdit( this );

    paletteLabel_ = new QLabel( this );
    palette_ = new QComboBox( this );

    palette_->addItem( QString(), "SYSTEM" );
    palette_->addItem( QString(), "DARK" );
    palette_->addItem( QString(), "LIGHT" );

    paletteHighlightLabel_ = new QLabel( this );
    paletteHighlight_ = new QLineEdit( this );
    paletteHighlightDialog_ = new QPushButton( this );

    connect( paletteHighlightDialog_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    optionChainRefreshRateLabel_ = new QLabel( this );
    optionChainRefreshRate_ = new QLineEdit( this );

    optionChainExpiryEndDateLabel_ = new QLabel( this );
    optionChainExpiryEndDate_ = new QLineEdit( this );

    optionChainWatchListsLabel_ = new QLabel( this );
    optionChainWatchLists_ = new QLineEdit( this );
    optionChainWatchListsDialog_ = new QPushButton( this );

    connect( optionChainWatchListsDialog_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    optionTradeCostLabel_ = new QLabel( this );
    optionTradeCost_ = new QLineEdit( this );

    optionCalcMethodLabel_ = new QLabel( this );
    optionCalcMethod_ = new QComboBox( this );

    optionCalcMethod_->addItem( QString(), "BARONEADESIWHALEY" );
    optionCalcMethod_->addItem( QString(), "BINOM" );
    optionCalcMethod_->addItem( QString(), "BINOM_EQPROB" );
    optionCalcMethod_->addItem( QString(), "BJERKSUNDSTENSLAND93" );
    optionCalcMethod_->addItem( QString(), "BJERKSUNDSTENSLAND02" );
    optionCalcMethod_->addItem( QString(), "BLACKSCHOLES" );
    optionCalcMethod_->addItem( QString(), "MONTECARLO" );
    optionCalcMethod_->addItem( QString(), "TRINOM" );
    optionCalcMethod_->addItem( QString(), "TRINOM_ALT" );
    optionCalcMethod_->addItem( QString(), "TRINOM_KR" );

    optionAnalysisFilterLabel_ = new QLabel( this );
    optionAnalysisFilter_ = new QComboBox( this );

    optionAnalysisFilter_->addItem( QString(), QString() );

    foreach ( const QString& f, AppDatabase::instance()->filters() )
        optionAnalysisFilter_->addItem( f, f );

    optionAnalysisFilterDialog_ = new QPushButton( this );

    connect( optionAnalysisFilterDialog_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    // okay
    okay_ = new QPushButton( this );
    okay_->setDefault( true );

    connect( okay_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    // cancel
    cancel_ = new QPushButton( this );

    connect( cancel_, &QPushButton::clicked, this, &_Myt::reject );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ConfigurationDialog::createLayout()
{
    QHBoxLayout *palette( new QHBoxLayout );
    palette->setContentsMargins( QMargins() );
    palette->addWidget( paletteHighlight_, 1 );
    palette->addWidget( paletteHighlightDialog_ );

    QHBoxLayout *equityWatchLists( new QHBoxLayout );
    equityWatchLists->setContentsMargins( QMargins() );
    equityWatchLists->addWidget( equityWatchLists_, 1 );
    equityWatchLists->addWidget( equityWatchListsDialog_ );

    QHBoxLayout *optionChainWatchLists( new QHBoxLayout );
    optionChainWatchLists->setContentsMargins( QMargins() );
    optionChainWatchLists->addWidget( optionChainWatchLists_, 1 );
    optionChainWatchLists->addWidget( optionChainWatchListsDialog_ );

    QHBoxLayout *optionAnalysisFilter( new QHBoxLayout );
    optionAnalysisFilter->setContentsMargins( QMargins() );
    optionAnalysisFilter->addWidget( optionAnalysisFilter_, 1 );
    optionAnalysisFilter->addWidget( optionAnalysisFilterDialog_ );

    QFormLayout *configs( new QFormLayout );
    configs->addRow( historyLabel_, history_ );
    configs->addRow( marketTypesLabel_, marketTypes_ );
    configs->addRow( numDaysLabel_, numDays_ );
    configs->addRow( numTradingDaysLabel_, numTradingDays_ );
    configs->addRow( paletteLabel_, palette_ );
    configs->addRow( paletteHighlightLabel_, palette );
    configs->addItem( new QSpacerItem( 16, 16 ) );
    configs->addRow( equityRefreshRateLabel_, equityRefreshRate_ );
    configs->addRow( equityTradeCostLabel_, equityTradeCost_ );
    configs->addRow( equityTradeCostNonExchangeLabel_, equityTradeCostNonExchange_ );
    configs->addRow( equityWatchListsLabel_, equityWatchLists );
    configs->addItem( new QSpacerItem( 16, 16 ) );
    configs->addRow( optionChainRefreshRateLabel_, optionChainRefreshRate_ );
    configs->addRow( optionChainExpiryEndDateLabel_, optionChainExpiryEndDate_ );
    configs->addRow( optionChainWatchListsLabel_, optionChainWatchLists );
    configs->addRow( optionTradeCostLabel_, optionTradeCost_ );
    configs->addRow( optionCalcMethodLabel_, optionCalcMethod_ );
    configs->addItem( new QSpacerItem( 16, 16 ) );
    configs->addRow( optionAnalysisFilterLabel_, optionAnalysisFilter );

    QHBoxLayout *buttons( new QHBoxLayout );
    buttons->addStretch();
    buttons->addWidget( cancel_ );
    buttons->addWidget( okay_ );

    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->addLayout( configs );
    form->addStretch();
    form->addLayout( buttons );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ConfigurationDialog::saveForm()
{
    checkConfigChanged( HISTORY, history_->text() );
    checkConfigChanged( MARKET_TYPES, marketTypes_->text() );
    checkConfigChanged( NUM_DAYS, numDays_->text() );
    checkConfigChanged( NUM_TRADING_DAYS, numTradingDays_->text() );
    checkConfigChanged( PALETTE, palette_->currentData().toString() );
    checkConfigChanged( PALETTE_HIGHLIGHT, paletteHighlight_->text() );

    checkConfigChanged( EQUITY_REFRESH_RATE, equityRefreshRate_->text() );
    checkConfigChanged( EQUITY_TRADE_COST, equityTradeCost_->text() );
    checkConfigChanged( EQUITY_TRADE_COST_NON_EXCHANGE, equityTradeCostNonExchange_->text() );
    checkConfigChanged( EQUITY_WATCH_LISTS, equityWatchLists_->text() );

    checkConfigChanged( OPTION_CHAIN_REFRESH_RATE, optionChainRefreshRate_->text() );
    checkConfigChanged( OPTION_CHAIN_EXPIRY_END_DATE, optionChainExpiryEndDate_->text() );
    checkConfigChanged( OPTION_CHAIN_WATCH_LISTS, optionChainWatchLists_->text() );
    checkConfigChanged( OPTION_TRADE_COST, optionTradeCost_->text() );
    checkConfigChanged( OPTION_CALC_METHOD, optionCalcMethod_->currentData().toString() );

    checkConfigChanged( OPTION_ANALYSIS_FILTER, optionAnalysisFilter_->currentData().toString() );

    // save!
    AppDatabase::instance()->setConfigs( configs_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ConfigurationDialog::checkConfigChanged( const QString& config, const QString& value )
{
    if ( configs_[config] == value )
        configs_.remove( config );
    else
        configs_[config] = value;
}
