/**
 * @file optionviewerwidget.h
 * Widget for viewing option chains and underlying information.
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

#ifndef OPTIONVIEWERWIDGET_H
#define OPTIONVIEWERWIDGET_H

#include <QDate>
#include <QList>
#include <QMap>
#include <QWidget>

class OptionTradingItemModel;
class OptionTradingView;
class QuoteTableModel;

class QLabel;
class QSplitter;
class QTabWidget;
class QToolButton;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Widget for viewing option chains and underlying information.
class OptionViewerWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString symbol READ symbol )

    using _Myt = OptionViewerWidget;
    using _Mybase = QWidget;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] symbol  symbol
     * @param[in] parent  parent object
     */
    OptionViewerWidget( const QString& symbol, QWidget *parent = nullptr );

    /// Destructor.
    virtual ~OptionViewerWidget();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve symbol.
    /**
     * @return  symbol
     */
    virtual QString symbol() const {return symbol_;}

    // ========================================================================
    // Methods
    // ========================================================================

    /// Translate strings.
    virtual void translate();

public slots:

    /// Refresh underlying data.
    virtual void refreshData();

private slots:

    /// Slot for button clicked.
    void onButtonClicked();

    /// Slot for option chain updated.
    void onOptionChainUpdated( const QString& symbol, const QList<QDate>& expiryDates, bool background );

    /// Slot for quotes updated.
    void onQuotesUpdated( const QStringList& symbols, bool background );

    /// Slot for tab changed.
    void onTabCurrentChanged( int index );

private:

    static const QString STATE_GROUP_NAME;
    static const QString STATE_NAME;

    QuoteTableModel *model_;
    OptionTradingItemModel *tradingModel_;

    QString symbol_;

    int chartTab_;

    // ---- //

    QLabel *description_;
    QLabel *exchange_;
    QLabel *stamp_;

    QLabel *lastLabel_;
    QLabel *last_;

    QLabel *lastChangeLabel_;
    QLabel *lastChange_;

    QLabel *bidLabel_;
    QLabel *bid_;

    QLabel *askLabel_;
    QLabel *ask_;

    QLabel *sizeLabel_;
    QLabel *size_;

    QLabel *markLabel_;
    QLabel *mark_;

    QLabel *markChangeLabel_;
    QLabel *markChange_;

    QLabel *volumeLabel_;
    QLabel *volume_;

    QLabel *openLabel_;
    QLabel *open_;

    QLabel *closeLabel_;
    QLabel *close_;

    QLabel *dayRangeLabel_;
    QLabel *dayRange_;

    QLabel *yearRangeLabel_;
    QLabel *yearRange_;

    QLabel *divLabel_;
    QLabel *div_;

    QLabel *divDateLabel_;
    QLabel *divDate_;

    QToolButton *clear_;
    QToolButton *analysisOne_;
    QToolButton *analysisAll_;
    QToolButton *refresh_;

    QSplitter *splitter_;

    QTabWidget *expiryDates_;

    OptionTradingView *tradeAnalysis_;

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    /// Save splitter state.
    void saveState( QSplitter *w ) const;

    /// Restore splitter state.
    void restoreState( QSplitter *w ) const;

    // not implemented
    OptionViewerWidget( const _Myt& other ) = delete;

    // not implemented
    OptionViewerWidget( const _Myt&& other ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt& rhs ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt&& rhs ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // OPTIONVIEWERWIDGET_H
