/**
 * @file networkaccess.h
 * Network Access.
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

#ifndef NETWORKACCESS_H
#define NETWORKACCESS_H

#include <QList>
#include <QNetworkAccessManager>
#include <QSslError>

class QAuthenticator;
class QNetworkProxy;
class QNetworkReply;
class QSslPreSharedKeyAuthenticator;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Network Access.
class NetworkAccess : public QNetworkAccessManager
{
    Q_OBJECT

    using _Myt = NetworkAccess;
    using _Mybase = QNetworkAccessManager;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in,out] parent  parent object
     */
    NetworkAccess( QObject *parent = nullptr );

    /// Destructor.
    ~NetworkAccess();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve ignore all ssl errors.
    /**
     * @return  @c true when ignore all ssl errors, @c false otherwise
     */
    virtual bool ignoreAllSslErrors() const {return ignoreAllSslErrors_;}

    /// Set ignore all ssl errors.
    /**
     * @param[in] value  @c true to ignore all ssl errors, @c false otherwise
     */
    virtual void setIgnoreAllSslErrors( bool value ) {ignoreAllSslErrors_ = value;}

    /// Retrieve list of ssl errors ignored.
    /**
     * @return  list of ignored ssl errors
     */
    virtual QList<QSslError> ignoreSslErrors() const {return ignoreSslErrors_;}

    /// Set list of ssl errors to ignore.
    /**
     * @param[in] value  list of ssl errors to ignore
     */
    virtual void setIgnoreSslErrors( const QList<QSslError>& value ) {ignoreSslErrors_ = value;}

    // ========================================================================
    // Methods
    // ========================================================================

    /// Add credentials for authentication.
    /**
     * @param[in] host  host name
     * @param[in] user  user name
     * @param[in] pwd  password
     */
    virtual void addCredentials( const QString &host, const QString &user, const QString &pwd );

private slots:

    /// Slot for authentication required.
    void onAuthenticationRequired( QNetworkReply *reply, QAuthenticator *authenticator );

    /// Slot for finished.
    void onFinished( QNetworkReply *reply );

#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
    /// Slot for network accessible changed.
    void onNetworkAccessibleChanged( QNetworkAccessManager::NetworkAccessibility accessible );
#endif

    /// Slot for pre-shared key authentication required.
    void onPreSharedKeyAuthenticationRequired( QNetworkReply *reply, QSslPreSharedKeyAuthenticator *authenticator );

    /// Slot for proxy authentication required.
    void onProxyAuthenticationRequired( const QNetworkProxy &proxy, QAuthenticator *authenticator );

    /// Slot for ssl errors.
    void onSslErrors( QNetworkReply *reply, const QList<QSslError> &errors );

private:

    bool ignoreAllSslErrors_;

    QList<QSslError> ignoreSslErrors_;

    using UserPasswordPair = QPair<QString, QString>;

    QMap<QString, UserPasswordPair> credentials_;

    // not implemented
    NetworkAccess( const _Myt& ) = delete;

    // not implemented
    NetworkAccess( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // NETWORKACCESS_H
