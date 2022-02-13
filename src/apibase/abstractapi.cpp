/**
 * @file abstractapi.cpp
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

#include "abstractapi.h"
#include "common.h"

#include <QEventLoop>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

#include <QtConcurrent>

// uncomment to debug content data
//#define DEBUG_CONTENT_DATA
//#define DEBUG_CONTENT_DATA_SAVE

///////////////////////////////////////////////////////////////////////////////////////////////////
AbstractWebInterface::AbstractWebInterface( QObject *parent ) :
    _Mybase( parent ),
    networkAccess_( nullptr )
{
    // create network access
    networkAccess_ = new QNetworkAccessManager( this );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
AbstractWebInterface::~AbstractWebInterface()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AbstractWebInterface::setNetworkAccessManager( QNetworkAccessManager *value )
{
    if ( networkAccess_ )
        networkAccess_->deleteLater();

    networkAccess_ = value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AbstractWebInterface::onDownloadProgress( qint64 bytesReceived, qint64 bytesTotal )
{
    LOG_DEBUG << "download progress " << bytesReceived << " of " << bytesTotal;

    // get reply
    QNetworkReply *reply( qobject_cast<QNetworkReply*>( sender() ) );

    // nothing to do
    if ( !reply )
    {
        LOG_WARN << "bad reply";
        return;
    }

    // read control block
    const RequestControl rc( readRequestControl( reply ) );

    // we are getting data back so reset any active timeout
    if ( rc.timeout )
        rc.timeout->start();

    // emit!
    emit replyDownloadProgress( reply, bytesReceived, bytesTotal, rc.start.msecsTo( QDateTime::currentDateTime() ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AbstractWebInterface::onFinished()
{
    LOG_DEBUG << "request finished";

    // get reply
    QNetworkReply *reply( qobject_cast<QNetworkReply*>( sender() ) );

    // nothing to do
    if ( !reply )
    {
        LOG_WARN << "bad reply";
        return;
    }

    // process reply
    QFuture<void> f;

#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
    f = QtConcurrent::run( &_Myt::parseNetworkReply, this, reply, true );
#else
    f = QtConcurrent::run( this, &_Myt::parseNetworkReply, reply, true );
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AbstractWebInterface::createRequestControl( QNetworkReply *reply, unsigned int timeout )
{
    RequestControl rc;
    rc.start = QDateTime::currentDateTime();

    if ( !timeout )
        rc.timeout = nullptr;
    else
    {
        QTimer *t( new QTimer( this ) );
        t->setSingleShot( true );
        t->setInterval( timeout );
        t->start();

        connect( t, &QTimer::timeout, reply, &QNetworkReply::abort );

        LOG_DEBUG << "timeout of " << timeout << " ms";
        rc.timeout = t;
    }

    QMutexLocker guard( &m_ );
    pending_[reply] = rc;

    emit requestsPendingChanged( pending_.size() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
AbstractWebInterface::RequestControl AbstractWebInterface::readRequestControl( QNetworkReply *reply ) const
{
    RequestControl rc;

    QMutexLocker guard( &m_ );

    if ( !pending_.contains( reply ) )
        LOG_WARN << "reply not found in pending!";
    else
    {
        rc = pending_[reply];
    }

    return rc;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
AbstractWebInterface::RequestControl AbstractWebInterface::destroyRequestControl( QNetworkReply *reply )
{
    RequestControl rc;

    QMutexLocker guard( &m_ );

    if ( !pending_.contains( reply ) )
        LOG_WARN << "reply not found in pending!";
    else
    {
        rc = pending_[reply];
        rc.stop = QDateTime::currentDateTime();

        if ( rc.timeout )
            rc.timeout->deleteLater();

        pending_.remove( reply );
        emit requestsPendingChanged( pending_.size() );
    }

    LOG_DEBUG << "requests pending " << pending_.size();
    return rc;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QNetworkReply *AbstractWebInterface::handleRequest( Method m, const QUrl &url, bool blocking, unsigned int timeout, const QByteArray &content, const QString& contentType )
{
    // create request
    QNetworkRequest request( url );

    const QString urlString( url.toString() );
    LOG_INFO << "request type " << m << " url " << qPrintable( urlString );

    if ( contentType.length() )
    {
        request.setHeader( QNetworkRequest::ContentTypeHeader, contentType );

#ifdef DEBUG_CONTENT_DATA
        saveContent( content, "request.raw" );
#endif
    }

    // set headers
    for ( HeadersMap::const_iterator i( headers_.constBegin() ); i != headers_.constEnd(); ++i )
    {
        LOG_TRACE << "header info " << qPrintable( i.key() ) << " " << qPrintable( i.value() );
        request.setRawHeader( i.key(), i.value() );
    }

    // post!
    QNetworkReply *reply( nullptr );

    if ( DELETE_RESOURCE == m )
        reply = networkAccess_->deleteResource( request );
    else if ( GET == m )
        reply = networkAccess_->get( request );
    else if ( POST == m )
        reply = networkAccess_->post( request, content );
    else if ( PUT == m )
        reply = networkAccess_->put( request, content );

    if ( !reply )
        LOG_WARN << "bad reply";
    else
    {
        if ( !timeout )
        {
            LOG_DEBUG << "using default timeout";
            timeout = DEFAULT_TIMEOUT;
        }

        // create request
        createRequestControl( reply, timeout );

        // wait for response if blocking
        if ( !blocking )
        {
            LOG_DEBUG << "non-blocking request";

            connect( reply, &QNetworkReply::downloadProgress, this, &_Myt::onDownloadProgress );
            connect( reply, &QNetworkReply::finished, this, &_Myt::onFinished );
        }
        else
        {
            LOG_DEBUG << "wait for response " << timeout << " ms timeout...";

            QEventLoop eventLoop;
            connect( reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit );

            // wait...

            LOG_TRACE << "exec...";
            eventLoop.exec();

            LOG_TRACE << "exec... complete";

            parseNetworkReply( reply );
        }
    }

    LOG_TRACE << "done";
    return reply;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AbstractWebInterface::parseNetworkReply( QNetworkReply *reply, bool deleteReply )
{
    const RequestControl rc( destroyRequestControl( reply ) );

    const unsigned int elapsed( rc.start.msecsTo( rc.stop ) );

    bool valid( false );
    int statusCode( 0 );

    // process reply
    if ( QNetworkReply::NoError != reply->error() )
    {
        LOG_WARN << "network reply error " << reply->error() << " " << qPrintable( reply->errorString() );

        // give network reply error as negative status code
        statusCode = -reply->error();
    }
    else
    {
        // check status code
        const QVariant statusCodeAttr( reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ) );

        statusCode = statusCodeAttr.toInt();
        valid = true;
    }

    // check content
    const QByteArray content( reply->readAll() );
    const QString contentType( reply->header( QNetworkRequest::ContentTypeHeader ).toString() );

    LOG_DEBUG << "content length " << content.size() << " type " << qPrintable( contentType );

#ifdef DEBUG_CONTENT_DATA
    if ( content.size() )
        saveContent( content, "response.raw" );
#endif

    // emit!

    LOG_TRACE << "reply received...";
    emit replyReceived( reply, valid, statusCode, content, contentType, elapsed );

    LOG_TRACE << "reply received... done";

    // delete reply
    if ( deleteReply )
        reply->deleteLater();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AbstractWebInterface::saveContent( const QByteArray& a, const QString& filename )
{
#if !defined( DEBUG_CONTENT_DATA )
    Q_UNUSED( a )
    Q_UNUSED( filename )
#elif !defined( DEBUG_CONTENT_DATA_SAVE )
    Q_UNUSED( filename )
#endif

#ifdef DEBUG_CONTENT_DATA
#if defined(HAVE_CLIO_H)
    LOG_TRACE << HEX_DUMP( a.constData(), a.length() );
#else
    LOG_TRACE << qPrintable( a );
#endif

#ifdef DEBUG_CONTENT_DATA_SAVE
    QFile f( filename );

    if ( f.open( QFile::WriteOnly ) )
    {
        f.write( a );
        f.close();
    }
#endif
#endif
}
