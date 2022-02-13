/**
 * @file tdoauthapi.cpp
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
#include "stringsoauth.h"
#include "tdoauthapi.h"

#include <QDesktopServices>
#include <QEventLoop>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QTimer>
#include <QUrlQuery>

static const QString CREDENTIALS_FILE( USER_CONF_DIR "credentials.json" );

static const QByteArray AUTHORIZATION( "Authorization" );
static const QString BEARER( "Bearer" );

static const QString APPLICTION_FORM_URLENCODED( "application/x-www-form-urlencoded" );

///////////////////////////////////////////////////////////////////////////////////////////////////
TDOpenAuthInterface::TDOpenAuthInterface( QObject *parent ) :
    _Mybase( parent ),
    authFlow_( nullptr ),
    authFlowHandler_( nullptr ),
    timerRefreshAccessToken_( nullptr ),
    state_( Offline )
{
    connect( this, &_Myt::processDocumentJson, this, &_Myt::onProcessDocumentJson, Qt::QueuedConnection );

    // create timer for auth timeout
    timerAuthTimeout_ = new QTimer( this );
    timerAuthTimeout_->setSingleShot( true );

    connect( timerAuthTimeout_, &QTimer::timeout, this, &_Myt::onTimeout );

    // create timer for refreshing access token
    timerRefreshAccessToken_ = new QTimer( this );
    timerRefreshAccessToken_->setSingleShot( true );

    connect( timerRefreshAccessToken_, &QTimer::timeout, this, &_Myt::onTimeout );

    // create oauth code flow
    authFlow_ = new QOAuth2AuthorizationCodeFlow( this );
    authFlow_->setModifyParametersFunction( std::bind( &_Myt::buildModifyParametersFunction, this, std::placeholders::_1, std::placeholders::_2 ) );
    authFlow_->setNetworkAccessManager( networkAccess_ );

    connect( authFlow_, &QOAuth2AuthorizationCodeFlow::authorizationCallbackReceived, this, &_Myt::onAuthCallback );
    connect( authFlow_, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this, &_Myt::onOpenUrl );

    // generate default credentials file if not existing
    if ( !QFile::exists( CREDENTIALS_FILE ) )
    {
        // step 1
        authUrl_= "https://auth.tdameritrade.com/auth";
        clientId_ = "<YOUR TDA CLIENT ID>@AMER.OAUTHAP";
        redirectUrl_ = "https://localhost:8088/mofo";

        // step 2
        tokenUrl_ = "https://api.tdameritrade.com/v1/oauth2/token";

        // save
        saveCredentials();
    }

    // load
    loadCredentials();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TDOpenAuthInterface::~TDOpenAuthInterface()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDOpenAuthInterface::setClientId( const QString& value )
{
    if ( value == clientId_ )
        return;

    clientId_ = value;
    saveCredentials();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDOpenAuthInterface::setRedirectUrl( const QUrl& value )
{
    if ( value == redirectUrl_ )
        return;

    redirectUrl_ = value;
    saveCredentials();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool TDOpenAuthInterface::waitForConnected( int timeout ) const
{
    if ( Offline == connectedState() )
        return false;
    else if ( Online == connectedState() )
        return true;

    // setup wait timer
    QTimer timer;
    timer.setSingleShot( true );
    timer.start( timeout );

    // wait for state change
    QEventLoop eventLoop;

    connect( this, &_Myt::connectedStateChanged, &eventLoop, &QEventLoop::quit );
    connect( &timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit );

    eventLoop.exec();

    // check state
    return (Online == connectedState());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDOpenAuthInterface::authorize( const QString& scope )
{
    if ( Offline != connectedState() )
        return;

    // save off scope
    scope_ = scope;

    // load credentials
    if ( !loadCredentials() )
        return;

    // check if we need auth code
    bool needAuthCode( false );

    LOG_DEBUG << "refresh token expiry " << qPrintable( refreshTokenExpiry_.toString() );

    // missing refresh code
    if ( refreshToken_.isEmpty() )
        needAuthCode = true;

    // expired refresh code
    else if (( refreshTokenExpiry_.isValid() ) && ( refreshTokenExpiry_ < QDateTime::currentDateTime() ))
    {
        LOG_WARN << "refresh token is expired";
        needAuthCode = true;

    }

    // perform web based auth
    if ( needAuthCode )
    {
        LOG_INFO << "requesting authorization...";
        requestAuthorizationCode();
    }

    // otherwise we can request updated token
    else
    {
        LOG_INFO << "requesting token...";
        requestToken();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDOpenAuthInterface::setConnectedState( ConnectedState newState )
{
    if ( newState == state_ )
        return;

    // emit signal
    emit connectedStateChanged( state_ = newState );

    // update auth timer
    if ( Authorizing == state_ )
        timerAuthTimeout_->start( AUTH_TIMEOUT );
    else
        timerAuthTimeout_->stop();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDOpenAuthInterface::onAuthCallback( const QVariantMap& data )
{
    LOG_INFO << "auth callback received";

    if ( !data.contains( OAUTH_CODE ) )
        LOG_WARN << "no auth code!";
    else
    {
        authCode_ = QUrl::fromPercentEncoding( data[OAUTH_CODE].toByteArray() );

        LOG_INFO << "have auth code " << qPrintable( authCode_ );

        requestToken( true );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDOpenAuthInterface::onOpenUrl( const QUrl& url )
{
    LOG_INFO << "opening url " << qPrintable( url.toString() );
    QDesktopServices::openUrl( url );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDOpenAuthInterface::onProcessDocumentJson( const QUuid& uuid, const QByteArray& request, const QString& requestType, int status, const QJsonDocument& response )
{
    Q_UNUSED( request )
    Q_UNUSED( requestType )

    // validate request
    if ( uuid != authRequest_ )
        return;

    authRequest_ = QUuid();

    // validate response
    if ( 200 != status )
    {
        LOG_WARN << "bad return code from auth " << status;

        // check for expired token
        if (( -302 == status ) && ( response.isObject() ))
        {
            const QJsonObject obj( response.object() );

            if ( obj.contains( OAUTH_ERROR ) )
                if ( obj[OAUTH_ERROR] == OAUTH_INVALID_GRANT )
                    requestAuthorizationCode();
        }

        return;
    }
    else if ( !response.isObject() )
    {
        LOG_WARN << "response not an object";
        return;
    }

    // parse response
    const QJsonObject obj( response.object() );

    if (( obj.contains( OAUTH_TOKEN_TYPE ) ) && ( obj[OAUTH_TOKEN_TYPE].isString() ))
        LOG_INFO << "have token type" << qPrintable( obj[OAUTH_TOKEN_TYPE].toString() );

    if (( obj.contains( OAUTH_SCOPE ) ) && ( obj[OAUTH_SCOPE].isString() ))
        LOG_INFO << "have scope" << qPrintable( obj[OAUTH_SCOPE].toString() );

    if (( obj.contains( OAUTH_REFRESH_TOKEN ) ) && ( obj[OAUTH_REFRESH_TOKEN].isString() ))
    {
        refreshToken_ = obj[OAUTH_REFRESH_TOKEN].toString();
        LOG_INFO << "have refresh token " << qPrintable( refreshToken_ );

        if (( obj.contains( OAUTH_REFRESH_TOKEN_EXP_IN ) ) && ( obj[OAUTH_REFRESH_TOKEN_EXP_IN].isDouble() ))
        {
            const int expiry( obj[OAUTH_REFRESH_TOKEN_EXP_IN].toInt() );

            LOG_INFO << "refresh token expires in" << obj[OAUTH_REFRESH_TOKEN_EXP_IN].toInt();

            if ( 0 < expiry )
            {
                const QDateTime now( QDateTime::currentDateTime() );

                refreshTokenExpiry_ = now.addSecs( expiry );
            }
        }

        // we have a refresh token, we can clear out auth code
        authCode_.clear();
    }

    if (( obj.contains( OAUTH_ACCESS_TOKEN ) ) && ( obj[OAUTH_ACCESS_TOKEN].isString() ))
    {
        accessToken_ = obj[OAUTH_ACCESS_TOKEN].toString();
        LOG_INFO << "have access token " << qPrintable( accessToken_ );

        // update web headers with token
        const QString auth( BEARER + " " + accessToken_ );

        HeadersMap h( headers() );
        h[AUTHORIZATION] = auth.toLatin1();

        setHeaders( h );

        // set timer for token refresh
        if (( obj.contains( OAUTH_ACCESS_TOKEN_EXP_IN ) ) && ( obj[OAUTH_ACCESS_TOKEN_EXP_IN].isDouble() ))
        {
            int expiry( obj[OAUTH_ACCESS_TOKEN_EXP_IN].toInt() );

            LOG_INFO << "access token expires in " << expiry << " seconds";
            expiry *= 1000;

            // refresh a few minutes earlier than expiry
            if ( TOKEN_EXPIRY_OFFSET < expiry )
                timerRefreshAccessToken_->start( expiry - TOKEN_EXPIRY_OFFSET );
        }

        // access grated!
        LOG_INFO << "GRANTED";

        setConnectedState( Online );
    }

    saveCredentials();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDOpenAuthInterface::onTimeout()
{
    if ( timerAuthTimeout_ == sender() )
    {
        LOG_WARN << "authorization timed out";
        setConnectedState( Offline );
    }
    else if ( timerRefreshAccessToken_ == sender() )
    {
        LOG_INFO << "refreshing access token...";
        requestToken();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDOpenAuthInterface::requestAuthorizationCode()
{
    if ( scope_.length() )
    {
        LOG_INFO << "setting auth scope to " << qPrintable( scope_ );
        authFlow_->setScope( scope_ );
    }

    authFlow_->setAuthorizationUrl( authUrl_ );
    authFlow_->setClientIdentifier( QUrl::toPercentEncoding( clientId_ ) );

    if ( clientSecret_.length() )
        authFlow_->setClientIdentifierSharedKey( QUrl::toPercentEncoding(clientSecret_ ) );

    authFlow_->setAccessTokenUrl( tokenUrl_ );

    // set reply handler
    if ( !authFlowHandler_ )
    {
        // check for redirect uri
        if ( redirectUrl_.isValid() )
        {
            // check specific port
            const quint16 serverPort( redirectUrl_.port() );
            const QString serverPath( redirectUrl_.path() );

            LOG_DEBUG << "opening server reply hander on port " << serverPort << " path " << qPrintable( serverPath );

            authFlowHandler_ = new QOAuthHttpServerReplyHandler( serverPort, this );
            authFlowHandler_->setCallbackPath( serverPath );
        }

        // use default port
        if ( !authFlowHandler_ )
            authFlowHandler_ = new QOAuthHttpServerReplyHandler( this );

        authFlow_->setReplyHandler( authFlowHandler_ );
    }

    // update state
    setConnectedState( Authorizing );

    // grant!
    authFlow_->grant();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDOpenAuthInterface::requestToken( bool offline )
{
    QString query;
    query.append( OAUTH_CLIENT_ID + "=" + QUrl::toPercentEncoding( clientId_ ) );

    if ( offline )
        query.append( "&" + OAUTH_ACCESS_TYPE + "=" + OAUTH_OFFLINE );

    if ( authCode_.length() )
    {
        query.append( "&" + OAUTH_GRANT_TYPE + "=" + OAUTH_AUTH_CODE );
        query.append( "&" + OAUTH_CODE + "=" + QUrl::toPercentEncoding( authCode_ ) );
        query.append( "&" + OAUTH_REDIRECT_URI + "=" + QUrl::toPercentEncoding( redirectUrl_.toString() ) );
    }

    // request access token
    else
    {
        query.append( "&" + OAUTH_GRANT_TYPE + "=" + OAUTH_REFRESH_TOKEN );
        query.append( "&" + OAUTH_REFRESH_TOKEN + "=" + QUrl::toPercentEncoding( refreshToken_ ) );
    }

    // remove any existing token
    HeadersMap h( headers() );

    if ( h.contains( AUTHORIZATION ) )
    {
        h.remove( AUTHORIZATION );
        setHeaders( h );
    }

    // update state
    setConnectedState( Authorizing );

    // request!
    send( (authRequest_ = QUuid::createUuid()), tokenUrl_, query.toLatin1(), APPLICTION_FORM_URLENCODED, AUTH_TIMEOUT, AUTH_RETRIES );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool TDOpenAuthInterface::loadCredentials()
{
    // open credentials file
    if ( !QFile::exists( CREDENTIALS_FILE ) )
    {
        LOG_WARN << "credentials file does not exist";
        return false;
    }

    QFile f( CREDENTIALS_FILE );

    if ( !f.open( QFile::ReadOnly | QFile::Text ) )
    {
        LOG_WARN << "error opening credentials file";
        return false;
    }

    const QByteArray a( f.readAll() );

    f.close();

    // parse json document
    QJsonParseError e;

    const QJsonDocument doc( QJsonDocument::fromJson( a, &e ) );

    if ( QJsonParseError::NoError != e.error )
    {
        LOG_WARN << "error parsing credentials " << e.error << " " << qPrintable( e.errorString() );
        return false;
    }
    else if ( !doc.isObject() )
    {
        LOG_WARN << "credentials not an object";
        return false;
    }

    const QJsonObject credentials( doc.object() );

    // validate credentials
    if (( !credentials.contains( OAUTH_WEB ) ) || ( !credentials[OAUTH_WEB].isObject() ))
        return false;

    const QJsonObject web( credentials[OAUTH_WEB].toObject() );

    // step 1
    if (( web.contains( OAUTH_AUTH_URI ) ) && ( web[OAUTH_AUTH_URI].isString() ))
        authUrl_ = web[OAUTH_AUTH_URI].toString();

    if (( web.contains( OAUTH_AUTH_PROVIDER_CERT ) ) && ( web[OAUTH_AUTH_PROVIDER_CERT].isString() ))
        authProviderCert_ = web[OAUTH_AUTH_PROVIDER_CERT].toString();

    if (( web.contains( OAUTH_CLIENT_ID ) ) && ( web[OAUTH_CLIENT_ID].isString() ))
        clientId_ = web[OAUTH_CLIENT_ID].toString();

    if (( web.contains( OAUTH_CLIENT_SECRET ) ) && ( web[OAUTH_CLIENT_SECRET].isString() ))
        clientSecret_ = web[OAUTH_CLIENT_SECRET].toString();

    if (( web.contains( OAUTH_REDIRECT_URIS ) ) && ( web[OAUTH_REDIRECT_URIS].isArray() ))
    {
        const QJsonArray redirectUris( web[OAUTH_REDIRECT_URIS].toArray() );

        if ( redirectUris.size() )
            redirectUrl_ = redirectUris[0].toString();
    }

    // step 2
    if (( web.contains( OAUTH_TOKEN_URI ) ) && ( web[OAUTH_TOKEN_URI].isString() ))
        tokenUrl_ = web[OAUTH_TOKEN_URI].toString();

    // step 2 response
    if (( web.contains( OAUTH_REFRESH_TOKEN ) ) && ( web[OAUTH_REFRESH_TOKEN].isString() ))
        refreshToken_ = web[OAUTH_REFRESH_TOKEN].toString();

    if (( web.contains( OAUTH_REFRESH_TOKEN_EXPIRY ) ) && ( web[OAUTH_REFRESH_TOKEN_EXPIRY].isString() ))
        refreshTokenExpiry_ = QDateTime::fromString( web[OAUTH_REFRESH_TOKEN_EXPIRY].toString(), Qt::ISODateWithMs );

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool TDOpenAuthInterface::saveCredentials()
{
    static const QJsonValue nullVal( QJsonValue::Null );

    // generate credentials
    QJsonArray redirectUris;
    redirectUris.append( redirectUrl_.toString() );

    QJsonObject web;
    web[OAUTH_AUTH_URI] = authUrl_.toString();
    web[OAUTH_AUTH_PROVIDER_CERT] = authProviderCert_.isEmpty() ? nullVal : QJsonValue( authProviderCert_ );
    web[OAUTH_CLIENT_ID] = clientId_;
    web[OAUTH_CLIENT_SECRET] = clientSecret_.isEmpty() ? nullVal : QJsonValue( clientSecret_ );
    web[OAUTH_REDIRECT_URIS] = redirectUris;

    web[OAUTH_TOKEN_URI] = tokenUrl_.toString();

    if ( refreshToken_.length() )
        web[OAUTH_REFRESH_TOKEN] = refreshToken_;

    if ( refreshTokenExpiry_.isValid() )
        web[OAUTH_REFRESH_TOKEN_EXPIRY] = refreshTokenExpiry_.toString( Qt::ISODateWithMs );

    QJsonObject credentials;
    credentials[OAUTH_WEB] = web;

    // save file
    QFile f( CREDENTIALS_FILE );

    if ( !f.open( QFile::WriteOnly | QFile::Text ) )
    {
        LOG_WARN << "error opening credentials file";
        return false;
    }

    const QJsonDocument doc( credentials );

    f.write( doc.toJson( QJsonDocument::Indented ) );
    f.close();

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDOpenAuthInterface::buildModifyParametersFunction( QAbstractOAuth::Stage stage, ModifyParametersMap *params )
{
    // modify redirect uri
    if ( QAbstractOAuth::Stage::RequestingAuthorization == stage )
    {
        if (( params->contains( OAUTH_REDIRECT_URI ) ) && ( redirectUrl_.isValid() ))
        {
            LOG_DEBUG << "using redirect uri " << qPrintable( redirectUrl_.toString() );
            params->insert( OAUTH_REDIRECT_URI, QUrl::toPercentEncoding( redirectUrl_.toString() ) );
        }
    }
}
