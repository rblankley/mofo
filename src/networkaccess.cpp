/**
 * @file networkaccess.cpp
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
#include "networkaccess.h"

#include <QAuthenticator>
#include <QDebug>
#include <QNetworkReply>
#include <QSslCertificate>
#include <QSslConfiguration>

///////////////////////////////////////////////////////////////////////////////////////////////////
NetworkAccess::NetworkAccess( QObject *parent ) :
    _Mybase( parent ),
    ignoreAllSslErrors_( false )
{
    // connect slots
    connect( this, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), SLOT(onAuthenticationRequired(QNetworkReply*,QAuthenticator*)) );
    connect( this, SIGNAL(finished(QNetworkReply*)), SLOT(onFinished(QNetworkReply*)) );
    connect( this, SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)), SLOT(onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)) );
    connect( this, SIGNAL(preSharedKeyAuthenticationRequired(QNetworkReply*,QSslPreSharedKeyAuthenticator*)), SLOT(onPreSharedKeyAuthenticationRequired(QNetworkReply*,QSslPreSharedKeyAuthenticator*)) );
    connect( this, SIGNAL(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)), SLOT(onProxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)) );
    connect( this, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), SLOT(onSslErrors(QNetworkReply*,QList<QSslError>)) );

    // check access
    if ( QNetworkAccessManager::Accessible == networkAccessible() )
        LOG_DEBUG << "READY";
    else
        LOG_WARN << "network not accessible:" << networkAccessible();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
NetworkAccess::~NetworkAccess()
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NetworkAccess::addCredentials( const QString &host, const QString &user, const QString &pwd )
{
    credentials_[host] = UserPasswordPair( user, pwd );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NetworkAccess::onAuthenticationRequired( QNetworkReply *reply, QAuthenticator *authenticator )
{
    const QUrl url( reply->url() );

    const QString host( url.host() );
    LOG_WARN << "authentication required " << qPrintable( host );

    // check if we have credentials for this host
    if (( credentials_.contains( url.host() ) ) && ( authenticator ))
    {
        LOG_DEBUG << "using saved credentials";

        authenticator->setUser( credentials_[url.host()].first );
        authenticator->setPassword( credentials_[url.host()].second );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NetworkAccess::onFinished( QNetworkReply *reply )
{
    Q_UNUSED( reply )
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NetworkAccess::onNetworkAccessibleChanged( QNetworkAccessManager::NetworkAccessibility accessible )
{
    LOG_DEBUG << "network accessible changed: " << accessible;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NetworkAccess::onPreSharedKeyAuthenticationRequired( QNetworkReply *reply, QSslPreSharedKeyAuthenticator *authenticator )
{
    Q_UNUSED( reply )
    Q_UNUSED( authenticator )

    LOG_DEBUG << "pre-shared key authentication required";
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NetworkAccess::onProxyAuthenticationRequired( const QNetworkProxy &proxy, QAuthenticator *authenticator )
{
    Q_UNUSED( proxy )
    Q_UNUSED( authenticator )

    LOG_DEBUG << "proxy authentication required";
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NetworkAccess::onSslErrors( QNetworkReply *reply, const QList<QSslError> &errors )
{
    Q_UNUSED( reply )

    // show list of errors
    foreach( const QSslError &error, errors )
    {
        const QString errorString( error.errorString() );

        LOG_DEBUG << "ssl error: " << error.error() << " " << qPrintable( errorString );
    }

    // check ignore the error
    if ( ignoreAllSslErrors_ )
        reply->ignoreSslErrors();
    else if ( ignoreSslErrors_.size() )
        reply->ignoreSslErrors( ignoreSslErrors_ );
    else
    {
        LOG_TRACE << "not ignoring errors";

        // show list of configured ssl certificates
        const QSslConfiguration conf( reply->request().sslConfiguration() );

        foreach ( const QSslCertificate& cert, conf.caCertificates() )
        {
            const QStringList orgs( cert.subjectInfo( QSslCertificate::Organization ) );

            LOG_TRACE << "cert serial number: " << cert.serialNumber().constData();

            foreach ( const QString& org, orgs )
                LOG_TRACE << "    org " << qPrintable( org );
        }
    }
}
