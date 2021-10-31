/**
 * @file analysiswidget.cpp
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

#include "analysiswidget.h"
#include "optiontradingview.h"

#include <QVBoxLayout>

///////////////////////////////////////////////////////////////////////////////////////////////////
AnalysisWidget::AnalysisWidget( model_type *model, QWidget *parent ) :
    _Mybase( parent ),
    trades_( model )
{
    // init
    initialize();
    createLayout();
    translate();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
AnalysisWidget::~AnalysisWidget()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AnalysisWidget::translate()
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AnalysisWidget::initialize()
{
    tradeAnalysis_ = new OptionTradingView( trades_, this );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AnalysisWidget::createLayout()
{
    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->setContentsMargins( QMargins() );
    form->addWidget( tradeAnalysis_ );
}
