/**
 * @file serializedapi.cpp
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
#include "serializedapi.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QTemporaryFile>
#include <QThread>

static const QString TEMP_FILE_CACHE_DIR( "cache" );
static const QString TEMP_FILE( TEMP_FILE_CACHE_DIR + QDir::separator() + "download-XXXXXX.tmp" );

static const QString TEMP_FILE_FILTER( "download-*.tmp" );

///////////////////////////////////////////////////////////////////////////////////////////////////
SerializedWebInterface::SerializedWebInterface( QObject *parent ) :
    _Mybase( parent ),
    blocking_( false )
{
    // connect signals
    connect( this, &_Myt::replyDownloadProgress, this, &_Myt::onReplyDownloadProgress );
    connect( this, &_Myt::replyReceived, this, &_Myt::onReplyReceived );

    // delete all stale downloads
    QDir d;

    if ( !d.exists( TEMP_FILE_CACHE_DIR ) )
    {
        if ( !d.mkdir( TEMP_FILE_CACHE_DIR ) )
            LOG_FATAL << "error creating cache dir " << qPrintable( TEMP_FILE_CACHE_DIR );
    }
    else if ( !d.cd( TEMP_FILE_CACHE_DIR ) )
        LOG_WARN << "could not cd to " << qPrintable( TEMP_FILE_CACHE_DIR );
    else
    {
        QStringList names;
        names.append( TEMP_FILE_FILTER );

        // retrieve list of files to remove
        const QStringList files( d.entryList( names, QDir::Files ) );

        foreach ( const QString& file, files )
        {
            LOG_DEBUG << "removing cache file " << qPrintable( file );

            if ( !QFile::remove( d.absoluteFilePath( file ) ) )
                LOG_WARN << "could not remove cache file " << qPrintable( file );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SerializedWebInterface::~SerializedWebInterface()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializedWebInterface::downloadFile( const QUuid& uuid, const QUrl& url )
{
    downloadFile( uuid, url, QByteArray(), QString() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializedWebInterface::downloadFile( const QUuid& uuid, const QUrl& url, const QByteArray& request, const QString& requestType )
{
    const Method m( request.isNull() ? GET : POST );

    LOG_DEBUG << "download file " << qPrintable( uuid.toString() ) << " " << qPrintable( url.toString() );
    processRequestControl( createFileRequestControl( uuid, url, m, request, requestType ) );

    if ( isBlocking() )
        waitForResponse( uuid );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializedWebInterface::remove( const QUuid& uuid, const QUrl& url, unsigned int timeout, unsigned int maxAttempts )
{
    LOG_DEBUG << "remove " << qPrintable( uuid.toString() ) << " " << qPrintable( url.toString() );
    processRequestControl( createDocumentRequestControl( uuid, url, DELETE_RESOURCE, QByteArray(), QString(), timeout, maxAttempts ) );

    if ( isBlocking() )
        waitForResponse( uuid );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializedWebInterface::send( const QUuid& uuid, const QUrl& url, unsigned int timeout, unsigned int maxAttempts )
{
    send( uuid, url, QByteArray(), QString(), timeout, maxAttempts );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializedWebInterface::send( const QUuid& uuid, const QUrl& url, const QByteArray& request, const QString& requestType, unsigned int timeout, unsigned int maxAttempts )
{
    const Method m( request.isNull() ? GET : POST );

    LOG_DEBUG << "send " << qPrintable( uuid.toString() ) << " " << qPrintable( url.toString() );
    processRequestControl( createDocumentRequestControl( uuid, url, m, request, requestType, timeout, maxAttempts ) );

    if ( isBlocking() )
        waitForResponse( uuid );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializedWebInterface::upload( const QUuid& uuid, const QUrl& url, const QByteArray& request, const QString& requestType, unsigned int timeout, unsigned int maxAttempts )
{
    if ( request.isEmpty() )
        LOG_WARN << "empty request; nothing to upload";
    else if ( requestType.isEmpty() )
        LOG_WARN << "empty request type";
    else
    {
        LOG_DEBUG << "upload " << qPrintable( uuid.toString() ) << " " << qPrintable( url.toString() );
        processRequestControl( createDocumentRequestControl( uuid, url, PUT, request, requestType, timeout, maxAttempts ) );

        if ( isBlocking() )
            waitForResponse( uuid );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SerializedWebInterface::retryRequest( const QUuid& uuid, const QByteArray& request, const QString& requestType, int statusCode ) const
{
    Q_UNUSED( uuid )
    Q_UNUSED( request )
    Q_UNUSED( requestType )
    Q_UNUSED( statusCode )

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializedWebInterface::handleProcessDocument( const QUuid& uuid, const QByteArray& request, const QString& requestType, int status, const QByteArray& response, const QString& responseType )
{
    LOG_TRACE << "emit process document...";
    emit processDocument( uuid, request, requestType, status, response, responseType );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializedWebInterface::waitForResponse( const QUuid& uuid ) const
{
    static const int PROCESS_EVENTS_TIME( 16 );
    static const int SLEEP_TIME( 4 );

    LOG_DEBUG << "waiting for response " << qPrintable( uuid.toString() ) << "...";

    do
    {
        // process pending events
        QCoreApplication::processEvents( QEventLoop::AllEvents, PROCESS_EVENTS_TIME );

        // do not consume 100% cpu resources
        QThread::msleep( SLEEP_TIME );

    } while ( requestControlExists( uuid ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializedWebInterface::onReplyDownloadProgress( QNetworkReply *reply, qint64 bytesReceived, qint64 bytesTotal, unsigned int elapsed )
{
    Q_UNUSED( bytesReceived )
    Q_UNUSED( bytesTotal )
    Q_UNUSED( elapsed )

    // nothing to do
    if ( !bytesReceived )
        return;

    // read control block
    const RequestControl rc( readRequestControl( reply ) );

    if ( rc.uuid.isNull() )
        LOG_WARN << "invalid control block!";
    else if ( !rc.file )
        LOG_TRACE << "not a file";
    else
    {
        QFile f( rc.location );

        // append downloaded data to file
        if ( !f.open( QFile::WriteOnly | QFile::Append ) )
            LOG_WARN << "error opening file " << qPrintable( rc.location );
        else
        {
            f.write( reply->readAll() );
            f.close();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializedWebInterface::onReplyReceived( QNetworkReply *reply, bool valid, int statusCode, const QByteArray& content, const QString& contentType, unsigned int elapsed )
{
    Q_UNUSED( elapsed )

    // read control block
    RequestControl rc( readRequestControl( reply ) );

    if ( rc.uuid.isNull() )
    {
        LOG_WARN << "invalid control block!";
        return;
    }

    // increment attempts
    ++rc.attempts;

    LOG_INFO << "processing response for request " << qPrintable( rc.uuid.toString() );

    bool done( false );

    // process status code
    if ( valid )
    {
        // update control block
        rc.stop = QDateTime::currentDateTime();

        rc.status = statusCode;

        // control block done
        done = true;
    }
    else
    {
        // check if we should retry this request (non-files)
        if (( !rc.file ) && ( !retryRequest( rc.uuid, rc.request, rc.requestType, valid ? statusCode : -1 ) ))
            done = true;

        // too many attempts
        else if ( rc.maxAttempts <= rc.attempts )
        {
            LOG_WARN << "request " << qPrintable( rc.uuid.toString() ) << " failed ";
            done = true;
        }

        // request failure but we are done
        if ( done )
        {
            // update control block
            rc.stop = QDateTime::currentDateTime();

            rc.status = statusCode;
        }
    }

    LOG_DEBUG << "request status " << rc.status << " " << done;

    if ( done )
    {
        const qint64 elapsedTotal( rc.start.msecsTo( rc.stop ) );

        // log result
        LOG_INFO << "request " << qPrintable( rc.uuid.toString() ) << " took " << elapsedTotal << "ms (" << rc.status << ")";

        // emit!
        if ( !rc.file )
        {
            handleProcessDocument( rc.uuid, rc.request, rc.requestType, rc.status, content, contentType );
        }
        else
        {
            LOG_TRACE << "emit process file...";
            emit processFile( rc.uuid, rc.request, rc.requestType, rc.status, rc.location );

            // remove any cache file
            if (( rc.location.length() ) && ( QFile::exists( rc.location ) ))
            {
                LOG_DEBUG << "removing cache file " << qPrintable( rc.location );

                if ( !QFile::remove( rc.location ) )
                    LOG_WARN << "could not remove cache file " << qPrintable( rc.location );
            }
        }
    }
    else
    {
        LOG_DEBUG << "attempting request " << qPrintable( rc.uuid.toString() ) << " again " << rc.attempts << " " << rc.maxAttempts;
        processRequestControl( rc );
    }

    // destroy control block
    destroyRequestControl( reply );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QNetworkReply *SerializedWebInterface::processRequestControl( const RequestControl& rc )
{
    QNetworkReply *reply( nullptr );

    if ( DELETE_RESOURCE == rc.method )
        reply = deleteResource( rc.url, false, rc.timeout );
    else if ( GET == rc.method )
        reply = get( rc.url, false, rc.timeout );
    else if ( POST == rc.method )
        reply = post( rc.url, rc.request, rc.requestType, false, rc.timeout );
    else if ( PUT == rc.method )
        reply = put( rc.url, rc.request, rc.requestType, false, rc.timeout );

    if ( reply )
        writeRequestControl( reply, rc );
    else
    {
        LOG_WARN << "process request " << qPrintable( rc.uuid.toString() ) << " failed!";
    }

    return reply;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SerializedWebInterface::RequestControl SerializedWebInterface::createDocumentRequestControl( const QUuid& uuid, const QUrl& url, Method m, const QByteArray& request, const QString& requestType, unsigned int timeout, unsigned int maxAttempts )
{
    // create request control
    RequestControl rc;
    rc.start = QDateTime::currentDateTime();
    rc.timeout = timeout;
    rc.maxAttempts = maxAttempts;
    rc.attempts = 0;
    rc.uuid = uuid;
    rc.url = url;
    rc.request = request;
    rc.requestType = requestType;
    rc.method = m;
    rc.file = false;

    return rc;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SerializedWebInterface::RequestControl SerializedWebInterface::createFileRequestControl( const QUuid& uuid, const QUrl& url, Method m, const QByteArray& request, const QString& requestType )
{
    QTemporaryFile f( TEMP_FILE );
    QString location;

    // generate a temporary file
    if ( !f.open() )
        LOG_WARN << "error creating temporary file!";
    else
    {
        location = f.fileName();
        f.close();

        LOG_INFO << "temp file for " << qPrintable( uuid.toString() ) << " created " << qPrintable( location );
    }

    // create request control
    RequestControl rc;
    rc.start = QDateTime::currentDateTime();
    rc.timeout = 0;
    rc.maxAttempts = 1;
    rc.attempts = 0;
    rc.uuid = uuid;
    rc.url = url;
    rc.request = request;
    rc.requestType = requestType;
    rc.method = m;
    rc.file = true;
    rc.location = location;

    return rc;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SerializedWebInterface::RequestControl SerializedWebInterface::readRequestControl( QNetworkReply *reply ) const
{
    RequestControl rc;

    QMutexLocker guard( &m_ );

    if ( !pending_.contains( reply ) )
        LOG_WARN << "uuid not found in pending!";
    else
    {
        rc = pending_[reply];
    }

    return rc;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializedWebInterface::writeRequestControl( QNetworkReply *reply, const RequestControl& rc )
{
    QMutexLocker guard( &m_ );
    pending_[reply] = rc;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializedWebInterface::destroyRequestControl( QNetworkReply *reply )
{
    QMutexLocker guard( &m_ );

    if ( !pending_.contains( reply ) )
        LOG_WARN << "uuid not found in pending!";
    else
    {
        pending_.remove( reply );
    }

    LOG_DEBUG << "requests pending " << pending_.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SerializedWebInterface::requestControlExists( const QUuid& uuid ) const
{
    QMutexLocker guard( &m_ );

    foreach ( const RequestControl& rc, pending_ )
        if ( rc.uuid == uuid )
            return true;

    return false;
}
