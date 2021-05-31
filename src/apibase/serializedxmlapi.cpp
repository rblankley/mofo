/**
 * @file serializedxmlapi.cpp
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
#include "serializedxmlapi.h"

#include <QFile>
#include <QDomDocument>

// uncomment to debug content data
//#define DEBUG_XML
//#define DEBUG_XML_SAVE

static const QString APPLICATION_XML( "application/xml" );
static const QString APPLICATION_ATOM_XML( "application/atom+xml" );

///////////////////////////////////////////////////////////////////////////////////////////////////
SerializedXmlWebInterface::SerializedXmlWebInterface( QObject *parent ) :
    _Mybase( parent )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SerializedXmlWebInterface::~SerializedXmlWebInterface()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializedXmlWebInterface::handleProcessDocument( const QUuid& uuid, const QByteArray& request, const QString& requestType, int status, const QByteArray& response, const QString& responseType )
{
    if (( !responseType.contains( APPLICATION_XML ) ) && ( !responseType.contains( APPLICATION_ATOM_XML ) ))
        _Mybase::handleProcessDocument( uuid, request, requestType, status, response, responseType );

    // parse XML document
    else
    {
        QString errorMsg;

        int errorLine;
        int errorColumn;

        // parse!
        QDomDocument doc;

        if ( !doc.setContent( response, &errorMsg, &errorLine, &errorColumn ) )
        {
            LOG_WARN << "error parsing network response XML document " << qPrintable( errorMsg ) << " line " << errorLine << " column " << errorColumn;
            status = -1;
        }

#ifdef DEBUG_XML
        saveDocument( doc, "response.xml" );
#endif

        // emit!
        emit processDocumentXml( uuid, request, requestType, status, doc );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializedXmlWebInterface::downloadFile( const QUuid& uuid, const QUrl& url, const QDomDocument& request )
{
    _Mybase::downloadFile( uuid, url, request.toByteArray(), APPLICATION_XML );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializedXmlWebInterface::send( const QUuid& uuid, const QUrl& url, const QDomDocument& request, unsigned int timeout, unsigned int maxAttempts )
{
    _Mybase::send( uuid, url, request.toByteArray(), APPLICATION_XML, timeout, maxAttempts );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializedXmlWebInterface::saveDocument( const QDomDocument& doc, const QString& filename )
{
#if !defined( DEBUG_XML )
    Q_UNUSED( doc )
    Q_UNUSED( filename )
#elif !defined( DEBUG_XML_SAVE )
    Q_UNUSED( filename )
#endif

#ifdef DEBUG_XML
    const QByteArray a( doc.toByteArray( INDENT ) );

#if defined(HAVE_CLIO_H)
    LOG_TRACE << HEX_DUMP( a.constData(), a.length() );
#else
    LOG_TRACE << a;
#endif

#ifdef DEBUG_XML_SAVE
    QFile f( filename );

    if ( f.open( QFile::WriteOnly ) )
    {
        f.write( a );
        f.close();
    }
#endif
#endif
}
