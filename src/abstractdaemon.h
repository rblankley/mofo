/**
 * @file abstractdaemon.h
 * Abstract API Daemon.
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

#ifndef ABSTRACTDAEMON_H
#define ABSTRACTDAEMON_H

#include <QDate>
#include <QJsonObject>
#include <QList>
#include <QMutex>
#include <QObject>

class AppDatabase;

class QTimer;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Abstract API Daemon.
class AbstractDaemon : public QObject
{
    Q_OBJECT
    Q_PROPERTY( bool active READ isActive WRITE setActive NOTIFY activeChanged )
    Q_PROPERTY( ConnectedState connectedState READ connectedState NOTIFY connectedStateChanged )
    Q_PROPERTY( bool paused READ isPaused WRITE setPaused NOTIFY pausedChanged )
    Q_PROPERTY( bool processOutsideMarketHours READ processOutsideMarketHours WRITE setProcessOutsideMarketHours )

    using _Myt = AbstractDaemon;
    using _Mybase = QObject;

public:

    /// Daemon connected state.
    enum ConnectedState
    {
        Offline,                                    ///< Not authorized.
        Authorizing,                                ///< Authorizing.
        Online,                                     ///< Authorized.
    };

    Q_ENUM( ConnectedState )

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve connected state.
    /**
     * @return  connected state
     */
    virtual ConnectedState connectedState() const;

    /// Check if daemon active.
    /**
     * @return  @c true if active, @c false otherwise
     */
    virtual bool isActive() const;

    /// Check if daemon paused.
    /**
     * @return  @c true if paused, @c false otherwise.
     */
    virtual bool isPaused() const {return paused_;}

    /// Retrieve daemon name.
    /**
     * @return  name
     */
    virtual QString name() const;

    /// Check processing equity and option chains outside market hours.
    /**
     * @return  @c true if processing when market close, @c false otherwise
     */
    virtual bool processOutsideMarketHours() const {return queueWhenClosed_;}

    /// Process equity and option chains outside market hours.
    /**
     * @return  @c true to process when market closed, @c false otherwise
     */
    virtual void setProcessOutsideMarketHours( bool value ) {queueWhenClosed_ = value;}

    // ========================================================================
    // Methods
    // ========================================================================

    /// Retrieve accounts.
    virtual void getAccounts();

    /// Retrieve option chain.
    /**
     * @param[in] symbol  symbol
     */
    virtual void getOptionChain( const QString& symbol );

    /// Wait for connected.
    /**
     * @param[in] timeout  time to wait (ms)
     * @return  @c true if connected, @c false otherwise
     */
    virtual bool waitForConnected( int timeout = 240 * 1000 ) const;

    // ========================================================================
    // Static Methods
    // ========================================================================

    /// Retrieve global instance.
    /**
     * @warning
     * Could return @c nullptr if one hasn't been created yet.
     * @return  pointer to instance
     */
    static _Myt *instance();

public slots:

    // ========================================================================
    // Properties
    // ========================================================================

    /// Set daemon active.
    /**
     * @return  @c true to activate, @c false otherwise
     */
    virtual void setActive( bool value );

    /// Set daemon paused.
    /**
     * @return  @c true to pause, @c false otherwise.
     */
    virtual void setPaused( bool value );

    // ========================================================================
    // Methods
    // ========================================================================

    /// Daemon API Authorization.
    virtual void authorize();

signals:

    /// Signal for active changed.
    /**
     * @param[in] newValue  @c true if active, @c false otherwise
     */
    void activeChanged( bool newValue );

    /// Signal for when connected state changes.
    /**
     * @param[in] newState  new state
     */
    void connectedStateChanged( AbstractDaemon::ConnectedState newState );

    /// Signal for when option chains have updated.
    /**
     * @param[in] symbol  updated symbol
     * @param[in] expiryDates  list of expiration dates
     * @param[in] background  @c true if update occured in background processing, @c false otherwise
     */
    void optionChainUpdated( const QString& symbol, const QList<QDate>& expiryDates, bool background = false );

    /// Signal for when quotes have updated.
    /**
     * @param[in] symbols  list of updated symbols
     * @param[in] background  @c true if update occured in background processing, @c false otherwise
     */
    void quotesUpdated( const QStringList& symbols, bool background = false );

    /// Signal for paused changed.
    /**
     * @param[in] newValue  @c true if paused, @c false otherwise
     */
    void pausedChanged( bool newValue );

    /// Signal for requests pending changed.
    /**
     * @param[in] pending  number of requests pending
     */
    void requestsPendingChanged( int pending );

    /// Signal for status message changed.
    /**
     * @param[in] message  status message
     * @param[in] timeout  message timeout
     */
    void statusMessageChanged( const QString& message, int timeout = 0 );

protected:

    AppDatabase *db_;                               ///< Database.

    QJsonObject configs_;                           ///< Configuration.

    bool queueWhenClosed_;                          ///< Queue requests when closed.
    bool paused_;                                   ///< Daemon is paused.

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in,out] parent  parent object
     */
    AbstractDaemon( QObject *parent = nullptr );

    /// Destructor.
    ~AbstractDaemon();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve dequeue time.
    /**
     * @return  dequeue time (ms)
     */
    virtual int dequeueTime() const {return DEFAULT_DEQUEUE_TIME;}

    /// Retrieve equity watchlist.
    /**
     * @return  equity watchlist symbols
     */
    virtual QStringList equityWatchlist() const;

    /// Retrieve option chain expiry end date.
    /**
     * @return  number of days to fetch
     */
    virtual int optionChainExpiryEndDate() const;

    /// Retrieve option chain watchlist.
    /**
     * @return  option chain watchlist symbols
     */
    virtual QStringList optionChainWatchlist() const;

    // ========================================================================
    // Methods
    // ========================================================================

    /// Dequeue
    virtual void dequeue() {}

    /// Queue equity requests.
    virtual void queueEquityRequests() {}

    /// Queue option chain requests.
    virtual void queueOptionChainRequests() {}

private slots:

    /// Slot for configuration changed.
    void onConfigurationChanged();

    /// Slot for timeout.
    void onTimeout();

private:

    enum {DEFAULT_DEQUEUE_TIME = 100}; // 100ms

    static QMutex instanceMutex_;
    static _Myt *instance_;

    QTimer *dequeue_;

    QTimer *equity_;
    QTimer *optionChain_;

    /// Update timer interval and start/stop.
    void updateTimerInterval( QTimer *t, int interval ) const;

    /// Retrieve watchlist symbols.
    QStringList watchlistSymbols( const QString& lists ) const;

    // not implemented
    AbstractDaemon( const _Myt& ) = delete;

    // not implemented
    AbstractDaemon( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // ABSTRACTDAEMON_H
