/**
 * @file sqldb.cpp
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
#include "sqldb.h"

#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QJsonObject>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextStream>
#include <QThread>
#include <QVariant>

///////////////////////////////////////////////////////////////////////////////////////////////////
SqlDatabase::SqlDatabase( const QString& name, const QString& version, QObject *parent ) :
    _Mybase( parent ),
#if QT_VERSION < QT_VERSION_CHECK( 5, 14, 0 )
    writer_( QMutex::Recursive ),
#endif
    name_( USER_CACHE_DIR + name ),
    version_( version ),
    backupName_( USER_CACHE_DIR + name + ".old" ),
    ready_( false )
{
    // this object should only be created by application thread
    assert( QApplication::instance()->thread() == QThread::currentThread() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SqlDatabase::~SqlDatabase()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString SqlDatabase::version() const
{
    QVariant result;
    readSetting( "dbversion", result );

    return result.toString();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSqlDatabase SqlDatabase::connection() const
{
    const QString cname( connectionNameThread() );

    // check existing connection
    QSqlDatabase db( QSqlDatabase::database( cname ) );

    if ( db.isOpen() )
        return db;

    // create new connection
    if ( !db.isValid() )
    {
        db = QSqlDatabase::addDatabase( "QSQLITE", cname );
        db.setConnectOptions( "QSQLITE_ENABLE_SHARED_CACHE" );
        db.setDatabaseName( name_ );
    }

    // open database
    if ( !db.open() )
    {
        LOG_FATAL << "failed to open db connection";
        return QSqlDatabase();
    }

    LOG_DEBUG << "new connection " << qPrintable( cname );

    // execute pragmas
    QStringList pragmas;
    //pragmas.append( "PRAGMA foreign_keys = ON" ); this appears to be causing massive stalls for larger option chains
    pragmas.append( "PRAGMA journal_mode = WAL" );
    pragmas.append( "PRAGMA synchronous = OFF" );
    pragmas.append( "PRAGMA temp_store = MEMORY" );

    QSqlQuery query( db );

    foreach ( const QString& pragma, pragmas )
        if ( !query.exec( pragma ) )
        {
            const QSqlError e( query.lastError() );

            LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        }

    return db;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString SqlDatabase::connectionNameThread() const
{
    static const QString CONN_NAME( "%1_%2" );

    if ( !QApplication::instance() )
        return QString();

    // for application thread use connection name
    if ( QApplication::instance()->thread() == QThread::currentThread() )
        connectionName();

    // return unique connection name
    return CONN_NAME
        .arg( connectionName() )
        .arg( (quintptr) QThread::currentThreadId() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SqlDatabase::setVersion( const QString &version )
{
    writeSetting( "dbversion", version );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SqlDatabase::readSetting( const QString& key, QVariant& value ) const
{
    static const QString sql( "SELECT value FROM settings WHERE key=:key" );

    QSqlQuery query( connection() );
    query.setForwardOnly( true );
    query.prepare( sql );

    query.bindValue( ":key", key );

    LOG_TRACE << "read setting " << qPrintable( key );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return false;
    }
    else if ( !query.next() )
    {
        LOG_DEBUG << "missing setting " << qPrintable( key );
        return false;
    }

    value = query.value( 0 );
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SqlDatabase::writeSetting( const QString& key, const QVariant& value )
{
    static const QString sql( "REPLACE INTO settings (key,value) "
        "VALUES (:key,:value)" );

    QMutexLocker guard( &writer_ );

    QSqlQuery query( connection() );
    query.prepare( sql );

    query.bindValue( ":key", key );
    query.bindValue( ":value", value );

    LOG_TRACE << "write setting " << qPrintable( key ) << " " << qPrintable( value.toString() );

    // exec sql
    bool result;

    if ( !(result = query.exec()) )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during replace " << e.type() << " " << qPrintable( e.text() );
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SqlDatabase::bindQueryValues( QSqlQuery& query, const QJsonObject& obj ) const
{
    // iterate all json fields
    for ( QJsonObject::const_iterator f( obj.constBegin() ); f != obj.constEnd(); ++f )
        if (( f->isBool() ) || ( f->isDouble() ) || ( f->isString() ))
            query.bindValue( ":" + f.key(), f->toVariant() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SqlDatabase::create()
{
    // execute sql statements
    return execute( createFiles() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SqlDatabase::open()
{
    const QDateTime now( QDateTime::currentDateTime() );

    bool exists( QFile::exists( name_ ) );

    // open database
    QSqlDatabase db( QSqlDatabase::addDatabase( "QSQLITE", connectionNameThread() ) );
    db.setDatabaseName( name_ );

    if ( !db.open() )
    {
        const QSqlError e0( db.lastError() );

        LOG_ERROR << "failed to open database " << qPrintable( name_ ) << " " << e0.type() << " " << qPrintable( e0.text() );

        if ( exists )
        {
            LOG_INFO << "removing bad database...";

            db.close();

            // rename bad db
            QFile f( backupName_ );
            f.remove();

            QFile::rename( name_, backupName_ );
            exists = false;

            if ( !db.open() )
            {
                const QSqlError e1( db.lastError() );

                LOG_FATAL << "failed to open database (second try!) " << qPrintable( name_ ) << " " << e1.type() << " " << qPrintable( e1.text() );
                return false;
            }
        }
    }

    // create new database
    if ( !exists )
    {
        LOG_INFO << "creating database...";

        if ( create() )
            writeSetting( "created", now.toString( Qt::ISODateWithMs ) );
        else
        {
            LOG_FATAL << "failed to create database";
            db.close();

            return false;
        }
    }

    // check for upgrade
    if ( version() != version_ )
    {
        LOG_INFO << "upgrading database...";

        if ( upgrade() )
            writeSetting( "upgraded", now.toString( Qt::ISODateWithMs ) );
        else
        {
            LOG_FATAL << "database upgrade failed!";
            db.close();

            return false;
        }
    }

    if ( !writeSetting( "accessed", now.toString( Qt::ISODateWithMs ) ) )
        return false;

    return (ready_ = true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SqlDatabase::updateDefaultValue( QSqlQuery& query, const QJsonObject& obj, const QString& field )
{
    QJsonValue value;

    if ( obj.contains( field ) )
        value = obj[field];

    // use default value
    if ( value.isNull() )
    {
        QVariant v;

        if ( readSetting( field, v ) )
            query.bindValue( ":" + field, v.toString() );
    }

    // set default value
    else
    {
        writeSetting( field, value.toVariant() );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SqlDatabase::upgrade()
{
    // upgrade!
    if ( !execute( upgradeFiles( version(), version_ ) ) )
        return false;

    // set new version
    setVersion( version_ );

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SqlDatabase::execute( const QStringList& files )
{
    // execute sql statements
    foreach ( const QString &file, files )
    {
        QFile f( file );

        // open file
        if ( !f.open( QFile::ReadOnly ) )
            return false;

        // read sql
        QTextStream s( &f );

        const QString stmt( s.readAll() );

        if ( stmt.isEmpty() )
            return false;

        const QStringList sqlList( stmt.split( ";", SKIP_EMPTY_PARTS ) );

        // execute querys
        foreach ( const QString &sql, sqlList )
        {
            const QString sqlTrimmed( sql.trimmed() );

            if ( sqlTrimmed.length() )
            {
                QSqlQuery query( connection() );

                LOG_DEBUG << "exec " << qPrintable( sqlTrimmed );

                if ( !query.exec( sqlTrimmed ) )
                {
                    const QSqlError e( query.lastError() );

                    LOG_ERROR << "error executing sql " << e.type() << " " << qPrintable( e.text() );
                    return false;
                }
            }
        }

        f.close();
    }

    return true;
}
