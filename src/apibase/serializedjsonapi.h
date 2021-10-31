/**
 * @file serializedjsonapi.h
 * Serialized JSON API Interface.
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

#ifndef SERIALIZEDJSONAPI_H
#define SERIALIZEDJSONAPI_H

#include "serializedapi.h"

class QJsonDocument;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Serialized Json API Interface.
class SerializedJsonWebInterface : public SerializedWebInterface
{
    Q_OBJECT

    using _Myt = SerializedJsonWebInterface;
    using _Mybase = SerializedWebInterface;

signals:

    /// Signal for document processing.
    /**
     * @param[in] uuid  request id
     * @param[in] request  request
     * @param[in] requestType  request type
     * @param[in] status  request status
     * @param[in] response  response
     */
    void processDocumentJson( const QUuid& uuid, const QByteArray& request, const QString& requestType, int status, const QJsonDocument& response );

public slots:

    // ========================================================================
    // Methods
    // ========================================================================

    /// Download file.
    /**
     * @param[in] uuid  uuid
     * @param[in] url  url to download
     */
    virtual void downloadFile( const QUuid& uuid, const QUrl& url ) override {
        _Mybase::downloadFile( uuid, url );
    }

    /// Download file.
    /**
     * @param[in] uuid  uuid
     * @param[in] url  url to download
     * @param[in] request  request
     */
    virtual void downloadFile( const QUuid& uuid, const QUrl& url, const QJsonDocument& request );

    /// Download file.
    /**
     * @param[in] uuid  uuid
     * @param[in] url  url to download
     * @param[in] request  request
     * @param[in] requestType  request type
     */
    virtual void downloadFile( const QUuid& uuid, const QUrl& url, const QByteArray& request, const QString& requestType ) override {
        _Mybase::downloadFile( uuid, url, request, requestType );
    }

    /// Send request.
    /**
     * @param[in] uuid  uuid
     * @param[in] url  url to download
     * @param[in] timeout  >0 to for request timeout (ms)
     * @param[in] maxAttempts  number of attempts
     */
    virtual void send( const QUuid& uuid, const QUrl& url, unsigned int timeout, unsigned int maxAttempts ) override {
        _Mybase::send( uuid, url, timeout, maxAttempts );
    }

    /// Send request.
    /**
     * @param[in] uuid  uuid
     * @param[in] url  url to download
     * @param[in] request  request
     * @param[in] timeout  >0 to for request timeout (ms)
     * @param[in] maxAttempts  number of attempts
     */
    virtual void send( const QUuid& uuid, const QUrl& url, const QJsonDocument& request, unsigned int timeout, unsigned int maxAttempts );

    /// Send request.
    /**
     * @param[in] uuid  uuid
     * @param[in] url  url to download
     * @param[in] request  request
     * @param[in] requestType  request type
     * @param[in] timeout  >0 to for request timeout (ms)
     * @param[in] maxAttempts  number of attempts
     */
    virtual void send( const QUuid& uuid, const QUrl& url, const QByteArray& request, const QString& requestType, unsigned int timeout, unsigned int maxAttempts ) override {
        _Mybase::send( uuid, url, request, requestType, timeout, maxAttempts );
    }

    /// Upload resource.
    /**
     * @param[in] uuid  uuid
     * @param[in] url  url to download
     * @param[in] request  request
     * @param[in] timeout  >0 to for request timeout (ms)
     * @param[in] maxAttempts  number of attempts
     */
    virtual void upload( const QUuid& uuid, const QUrl& url, const QJsonDocument& request, unsigned int timeout, unsigned int maxAttempts );

    /// Upload resource.
    /**
     * @param[in] uuid  uuid
     * @param[in] url  url to download
     * @param[in] request  request
     * @param[in] requestType  request type
     * @param[in] timeout  >0 to for request timeout (ms)
     * @param[in] maxAttempts  number of attempts
     */
    virtual void upload( const QUuid& uuid, const QUrl& url, const QByteArray& request, const QString& requestType, unsigned int timeout, unsigned int maxAttempts ) override {
        _Mybase::upload( uuid, url, request, requestType, timeout, maxAttempts );
    }

protected:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in,out] parent  parent object
     */
    SerializedJsonWebInterface( QObject *parent = nullptr );

    /// Destructor.
    ~SerializedJsonWebInterface();

    // ========================================================================
    // Methods
    // ========================================================================

    /// Handle process document.
    /**
     * @param[in] uuid  request id
     * @param[in] request  request
     * @param[in] requestType  request type
     * @param[in] status  request status
     * @param[in] response  response
     * @param[in] responseType  response type
     */
    virtual void handleProcessDocument( const QUuid& uuid, const QByteArray& request, const QString& requestType, int status, const QByteArray& response, const QString& responseType ) override;

private:

    /// Save document for debugging purposes.
    static void saveDocument( const QJsonDocument& content, const QString& filename );

    // not implemented
    SerializedJsonWebInterface( const _Myt& ) = delete;

    // not implemented
    SerializedJsonWebInterface( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // SERIALIZEDJSONAPI_H
