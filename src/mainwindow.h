/**
 * @file mainwindow.h
 * Main Window.
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "abstractdaemon.h"

#include <QMainWindow>

class AppDatabase;
class OptionAnalyzer;
class OptionTradingItemModel;

class QAction;
class QComboBox;
class QLabel;
class QMenu;
class QStatusBar;

///////////////////////////////////////////////////////////////////////////////////////////////////
class MainWindow : public QMainWindow
{
    Q_OBJECT

    typedef MainWindow _Myt;
    typedef QMainWindow _Mybase;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor
    /**
     * @param[in,out] parent  parent object
     */
    MainWindow( QWidget *parent = nullptr );

    /// Destructor.
    ~MainWindow();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve size hint.
    /**
     * @return  size hint
     */
    virtual QSize sizeHint() const;

    // ========================================================================
    // Methods
    // ========================================================================

    /// Translate strings.
    virtual void translate();

public slots:

    // ========================================================================
    // Methods
    // ========================================================================

    /// Update menu state.
    virtual void updateMenuState();

private slots:

    /// Slot for about to quit.
    void onAboutToQuit();

    /// Slot for accounts changed.
    void onAccountsChanged();

    /// Slot to handle action triggered.
    void onActionTriggered();

    /// Slot for connected state changed.
    void onConnectedStateChanged( AbstractDaemon::ConnectedState newState );

    /// Slot for requests pending changed.
    void onRequestsPendingChanged( int pending );

private:

    AbstractDaemon *daemon_;
    AppDatabase *db_;

    OptionAnalyzer *analysis_;
    OptionTradingItemModel *analysisModel_;

    QMenu *fileMenu_;
    QAction *exit_;

    QMenu *viewMenu_;
    QAction *config_;
    QAction *filters_;
    QAction *layouts_;
    QAction *watchlists_;

    QMenu *marketDaemonMenu_;
    QAction *authenticate_;
    QAction *credentials_;
    QAction *refreshAccountData_;
    QAction *singleOptionChain_;
    QAction *startDaemon_;
    QAction *stopDaemon_;
    QAction *pauseDaemon_;
    QAction *runWhenMarketsClosed_;

    QMenu *results_;
    QAction *viewAnalysis_;
    QAction *customScan_;

    QMenu *helpMenu_;
    QAction *about_;
    QAction *validate_;
    QAction *testPerf_;
    QAction *testGreeks_;

    QStatusBar *statusBar_;
    QLabel *connectionState_;
    QLabel *xmit_;
    QLabel *accountsLabel_;
    QComboBox *accounts_;

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    /// Update transmit state.
    void updateTransmitState( int pending );

    // not implemented
    MainWindow( const _Myt& other ) = delete;

    // not implemented
    MainWindow( const _Myt&& other ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt& rhs ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt&& rhs ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MAINWINDOW_H
