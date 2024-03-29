/**
 * @file optiontradingdetailsdialog.h
 * Dialog for showing option trading details.
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

#ifndef OPTIONTRADINGDETAILSDIALOG_H
#define OPTIONTRADINGDETAILSDIALOG_H

#include <QDialog>

class CollapsibleSplitter;
class OptionChainImpliedVolatilityWidget;
class OptionChainOpenInterestWidget;
class OptionChainProbabilityWidget;
class OptionTradingItemModel;
class OptionTradingReturnsGraphWidget;
class OptionTradingReturnsViewerWidget;

class QSplitter;
class QTabWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Dialog for showing option trading details.
class OptionTradingDetailsDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY( QStringList symbols READ symbols )
    Q_PROPERTY( QString underlying READ underlying )

    using _Myt = OptionTradingDetailsDialog;
    using _Mybase = QDialog;

public:

    /// Model type.
    using model_type = OptionTradingItemModel;

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] index  trading model index
     * @param[in] model  trading model
     * @param[in,out] parent  parent widget
     * @param[in] f  window flags
     */
    OptionTradingDetailsDialog( int index, model_type *model, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    /// Destructor.
    virtual ~OptionTradingDetailsDialog();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve size hint.
    /**
     * @return  size hint
     */
    virtual QSize sizeHint() const override;

    /// Retrieve option symbols.
    /**
     * @return  option symbols
     */
    virtual QStringList symbols() const {return symbols_;}

    /// Retrieve underlying.
    /**
     * @return  underlying
     */
    virtual QString underlying() const {return underlying_;}

    // ========================================================================
    // Methods
    // ========================================================================

    /// Translate strings.
    virtual void translate();

private:

    static constexpr int SPLITTER_WIDTH = 12;

    static const QString STATE_GROUP_NAME;

    model_type *model_;
    int index_;

    QString underlying_;
    double underlyingPrice_;

    QStringList symbols_;

    QString stratDesc_;
    int strat_;

    QTabWidget *tabs_;

    CollapsibleSplitter *splitter_;
    OptionTradingReturnsGraphWidget *tradeDetailsGraph_;
    OptionTradingReturnsViewerWidget *tradeDetails_;

    OptionChainImpliedVolatilityWidget *implVol_;

    OptionChainProbabilityWidget *prob_;

    OptionChainOpenInterestWidget *openInt_;

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    /// Retrieve model data.
    QVariant modelData( int col ) const;

    /// Save dialog state.
    void saveState( QDialog *w ) const;

    /// Save splitter state.
    void saveState( QSplitter *w ) const;

    /// Restore dialog state.
    void restoreState( QDialog *w ) const;

    /// Restore splitter state.
    void restoreState( QSplitter *w ) const;

    // not implemented
    OptionTradingDetailsDialog( const _Myt& ) = delete;

    // not implemented
    OptionTradingDetailsDialog( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // OPTIONTRADINGDETAILSDIALOG_H
