/**
 * @file analysiswidget.h
 * Widget for viewing option analysis.
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

#ifndef ANALYSISWIDGET_H
#define ANALYSISWIDGET_H

#include <QWidget>

class OptionTradingItemModel;
class OptionTradingView;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Widget for viewing option analysis.
class AnalysisWidget : public QWidget
{
    Q_OBJECT

    using _Myt = AnalysisWidget;
    using _Mybase = QWidget;

public:

    /// Model type.
    using model_type = OptionTradingItemModel;

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] model  trading model
     * @param[in] parent  parent object
     */
    AnalysisWidget( model_type *model, QWidget *parent = nullptr );

    /// Destructor.
    virtual ~AnalysisWidget();

    // ========================================================================
    // Methods
    // ========================================================================

    /// Translate strings.
    virtual void translate();

private:

    model_type *trades_;

    // ---- //

    OptionTradingView *tradeAnalysis_;

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    // not implemented
    AnalysisWidget( const _Myt& other ) = delete;

    // not implemented
    AnalysisWidget( const _Myt&& other ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt& rhs ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt&& rhs ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // ANALYSISWIDGET_H
