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
    name_( USER_CACHE_DIR + name ),
    version_( version ),
    backupName_( USER_CACHE_DIR + name + ".old" )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SqlDatabase::~SqlDatabase()
{
    db_.close();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString SqlDatabase::version() const
{
    QVariant result;
    readSetting( "dbversion", result );

    return result.toString();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SqlDatabase::setVersion( const QString &version )
{
    writeSetting( "dbversion", version );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSqlDatabase SqlDatabase::openDatabaseConnection() const
{
    static const QString SUFFIX( "_%1" );

    const bool mt( QApplication::instance()->thread() == QThread::currentThread() );
    const QString cname( connectionName() + (mt ? QString() : SUFFIX.arg( (quintptr) QThread::currentThreadId() ) ) );

    QSqlDatabase db( QSqlDatabase::database( cname ) );

    if (( !db.isOpen() ) || ( !db.isValid() ))
    {
       db = QSqlDatabase::cloneDatabase( db_, cname );

       if ( !db.open() )
           LOG_FATAL << "failed to open db connection";
    }

    return db;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SqlDatabase::readSetting( const QString& key, QVariant& value ) const
{
    return readSetting( key, value, db_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SqlDatabase::readSetting( const QString& key, QVariant& value, const QSqlDatabase& conn ) const
{
    static const QString sql( "SELECT value FROM settings WHERE ?=key" );

    QSqlQuery query( conn );
    query.prepare( sql );
    query.addBindValue( key );

    LOG_TRACE << "read setting " << qPrintable( key );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( db_.lastError() );

        LOG_ERROR << "error during select " << e.type() << " " << qPrintable( e.text() );
        return false;
    }
    else if ( !query.next() )
    {
        LOG_WARN << "missing setting " << qPrintable( key );
        return false;
    }

    value = query.value( 0 );
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SqlDatabase::writeSetting( const QString& key, const QVariant& value )
{
    return writeSetting( key, value, db_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SqlDatabase::writeSetting( const QString& key, const QVariant& value, const QSqlDatabase& conn )
{
    static const QString sql( "REPLACE INTO settings (key,value) "
        "VALUES(?,?);" );

    QSqlQuery query( conn );
    query.prepare( sql );
    query.addBindValue( key );
    query.addBindValue( value );

    LOG_TRACE << "write setting " << qPrintable( key ) << " " << qPrintable( value.toString() );

    // exec sql
    if ( !query.exec() )
    {
        const QSqlError e( query.lastError() );

        LOG_ERROR << "error during replace " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    return true;
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
    db_ = QSqlDatabase::addDatabase( "QSQLITE", connectionName() );
    db_.setDatabaseName( name_ );

    if ( !db_.open() )
    {
        const QSqlError e0( db_.lastError() );

        LOG_ERROR << "failed to open database " << qPrintable( name_ ) << " " << e0.type() << " " << qPrintable( e0.text() );

        if ( exists )
        {
            LOG_INFO << "removing bad database...";

            db_.close();

            // rename bad db
            QFile f( backupName_ );
            f.remove();

            QFile::rename( name_, backupName_ );
            exists = false;

            if ( !db_.open() )
            {
                const QSqlError e1( db_.lastError() );

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
            db_.close();

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
            db_.close();

            return false;
        }
    }

    return writeSetting( "accessed", now.toString( Qt::ISODateWithMs ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SqlDatabase::updateDefaultValue( QSqlQuery& query, const QJsonObject& obj, const QString& field )
{
    updateDefaultValue( query, obj, field, db_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SqlDatabase::updateDefaultValue( QSqlQuery& query, const QJsonObject& obj, const QString& field, const QSqlDatabase& conn )
{
    QJsonValue value;

    if ( obj.contains( field ) )
        value = obj[field];

    // use default value
    if ( value.isNull() )
    {
        QVariant v;

        if ( readSetting( field, v, conn ) )
            query.bindValue( ":" + field, v.toString() );
    }

    // set default value
    else
    {
        writeSetting( field, value.toVariant(), conn );
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
                QSqlQuery query( db_ );

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

