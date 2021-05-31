/**
 * @file tdoauthapi.h
 * TD Ameritrade OAuth implementation.
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

#ifndef TDOAUTHAPI_H
#define TDOAUTHAPI_H

#include "../apibase/serializedjsonapi.h"

#include <QAbstractOAuth>
#include <QVariantMap>

class QOAuthHttpServerReplyHandler;
class QOAuth2AuthorizationCodeFlow;
class QTimer;
class QUrl;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// TD Ameritrade OAuth implementation.
/**
 * Add your app here:
 * https://developer.tdameritrade.com/user/me/apps/add
 *
 * Then update the 'credentials.json' file with your app information.
 */
class TDOpenAuthInterface : public SerializedJsonWebInterface
{
    Q_OBJECT
    Q_PROPERTY( ConnectedState connectedState READ connectedState NOTIFY connectedStateChanged )

    using _Myt = TDOpenAuthInterface;
    using _Mybase = SerializedJsonWebInterface;

public:

    /// OAuth connected state.
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
    virtual ConnectedState connectedState() const {return state_;}

    // ========================================================================
    // Methods
    // ========================================================================

    /// Wait for connected.
    /**
     * @param[in] timeout  time to wait (ms)
     * @return  @c true if connected, @c false otherwise
     */
    virtual bool waitForConnected( int timeout = 240 * 1000 ) const;

public slots:

    // ========================================================================
    // Methods
    // ========================================================================

    /// Authorize using OAuth flow.
    /**
     * @param[in] scope  scope of authorization
     */
    virtual void authorize( const QString& scope = QString() );

signals:

    /// Signal for when connected state changes.
    /**
     * @param[in] newState  new state
     */
    void connectedStateChanged( ConnectedState newState );

protected:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in,out] parent  parent object
     */
    TDOpenAuthInterface( QObject *parent = nullptr );

    /// Destructor.
    ~TDOpenAuthInterface();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Set connected state.
    /**
     * @param[in] newState  new state
     */
    virtual void setConnectedState( ConnectedState newState );

private slots:

    /// Slot for open auth callback receieved.
    void onAuthCallback( const QVariantMap& data );

    /// Slot to open URL.
    void onOpenUrl( const QUrl& url );

    /// Slot to process document.
    void onProcessDocumentJson( const QUuid& uuid, const QByteArray& request, const QString& requestType, int status, const QJsonDocument& response );

    /// Slot for timer timeout.
    void onTimeout();

private:

    enum {AUTH_TIMEOUT = 30 * 1000};                    // 30s
    enum {AUTH_RETRIES = 3};

    enum {TOKEN_EXPIRY_OFFSET = 2 * 60 * 1000};         // 2m

    QOAuth2AuthorizationCodeFlow *authFlow_;
    QOAuthHttpServerReplyHandler *authFlowHandler_;

    QTimer *timerAuthTimeout_;
    QTimer *timerRefreshAccessToken_;

    QUuid authRequest_;

    ConnectedState state_;

    // step 1
    QUrl authUrl_;
    QString authProviderCert_;

    QString clientId_;
    QString clientSecret_;

    QUrl redirectUrl_;

    // step 1 response
    QString authCode_;

    // step 2
    QUrl tokenUrl_;
    QString scope_;

    // step 2 response
    QString refreshToken_;
    QDateTime refreshTokenExpiry_;

    QString accessToken_;

    /// Perform web based authorization.
    void requestAuthorizationCode();

    /// Request token.
    void requestToken( bool offline = false );

    // Load credentials.
    bool loadCredentials();

    // Save credentials.
    bool saveCredentials();

    // Modify parameters callback method.
    void buildModifyParametersFunction( QAbstractOAuth::Stage stage, QVariantMap *params );

    // not implemented
    TDOpenAuthInterface( const _Myt& ) = delete;

    // not implemented
    TDOpenAuthInterface( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // TDOAUTHAPI_H
