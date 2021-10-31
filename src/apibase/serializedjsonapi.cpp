/**
 * @file serializedjsonapi.cpp
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
#include "serializedjsonapi.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>

// uncomment to debug content data
//#define DEBUG_JSON
//#define DEBUG_JSON_SAVE

static const QString APPLICATION_JSON( "application/json" );

///////////////////////////////////////////////////////////////////////////////////////////////////
SerializedJsonWebInterface::SerializedJsonWebInterface( QObject *parent ) :
    _Mybase( parent )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SerializedJsonWebInterface::~SerializedJsonWebInterface()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializedJsonWebInterface::handleProcessDocument( const QUuid& uuid, const QByteArray& request, const QString& requestType, int status, const QByteArray& response, const QString& responseType )
{
    if ( !responseType.contains( APPLICATION_JSON ) )
        _Mybase::handleProcessDocument( uuid, request, requestType, status, response, responseType );

    // parse JSON document
    else
    {
        QJsonParseError err;

        // parse!
        const QJsonDocument doc( QJsonDocument::fromJson( response, &err ) );

        if ( QJsonParseError::NoError != err.error )
        {
            LOG_WARN << "error parsing network response JSON document " << err.error << " " << qPrintable( err.errorString() );
            status = -1;
        }

#ifdef DEBUG_JSON
        saveDocument( doc, "response.json" );
#endif

        // emit!
        emit processDocumentJson( uuid, request, requestType, status, doc );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializedJsonWebInterface::downloadFile( const QUuid& uuid, const QUrl& url, const QJsonDocument& request )
{
    _Mybase::downloadFile( uuid, url, request.toJson(), APPLICATION_JSON );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializedJsonWebInterface::send( const QUuid& uuid, const QUrl& url, const QJsonDocument& request, unsigned int timeout, unsigned int maxAttempts )
{
    _Mybase::send( uuid, url, request.toJson(), APPLICATION_JSON, timeout, maxAttempts );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializedJsonWebInterface::upload( const QUuid& uuid, const QUrl& url, const QJsonDocument& request, unsigned int timeout, unsigned int maxAttempts )
{
    _Mybase::upload( uuid, url, request.toJson(), APPLICATION_JSON, timeout, maxAttempts );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializedJsonWebInterface::saveDocument( const QJsonDocument& doc, const QString& filename )
{
#if !defined( DEBUG_JSON )
    Q_UNUSED( doc )
    Q_UNUSED( filename )
#elif !defined( DEBUG_JSON_SAVE )
    Q_UNUSED( filename )
#endif

#ifdef DEBUG_JSON
    const QByteArray a( doc.toJson() );

#if defined(HAVE_CLIO_H)
    LOG_TRACE << HEX_DUMP( a.constData(), a.length() );
#else
    LOG_TRACE << a;
#endif

#ifdef DEBUG_JSON_SAVE
    QFile f( filename );

    if ( f.open( QFile::WriteOnly ) )
    {
        f.write( a );
        f.close();
    }
#endif
#endif
}
