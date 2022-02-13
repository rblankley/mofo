/**
 * @file sqldb.h
 * Sql Database Base Class.
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

#ifndef SQLDB_H
#define SQLDB_H

#include <QMutex>
#include <QObject>
#include <QSqlDatabase>

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Sql Database Base Class.
class SqlDatabase : public QObject
{
    Q_OBJECT
    Q_PROPERTY( bool ready READ isReady )
    Q_PROPERTY( QString version READ version STORED true )

    using _Myt = SqlDatabase;
    using _Mybase = QObject;

public:

    // ========================================================================
    // Properties
    // ========================================================================

    /// Check if database is ready.
    /**
     * @return  @c true if ready, @c false otherwise
     */
    virtual bool isReady() const {return ready_;}

    /// Retrieve database version.
    /**
     * @return  version
     */
    virtual QString version() const;

protected:

#if QT_VERSION_CHECK( 5, 14, 0 ) <= QT_VERSION
    QRecursiveMutex writer_;                        ///< Mutex for database writes.
#else
    QMutex writer_;                                 ///< Mutex for database writes.
#endif

    QString name_;                                  ///< Database name.
    QString version_;                               ///< Database version.

    QString backupName_;                            ///< Database backup name.

    bool ready_;                                    ///< Database ready flag.

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] name  database filename
     * @param[in] version  database version
     * @param[in] parent  parent
     */
    SqlDatabase( const QString& name, const QString& version, QObject *parent = nullptr );

    /// Destructor.
    ~SqlDatabase();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve database connection.
    /**
     * @return  connection
     */
    virtual QSqlDatabase connection() const;

    /// Retrieve database connection name.
    /**
     * @return  connection name
     */
    virtual QString connectionName() const {return "default";}

    /// Retrieve thread specific connection name.
    /**
     * @return  thread specific connection name
     */
    virtual QString connectionNameThread() const;

    /// Retrieve sql files to create database.
    /**
     * @return  list of files to parse
     */
    virtual QStringList createFiles() const = 0;

    /// Set database version.
    /**
     * @param[in] version  version
     */
    virtual void setVersion( const QString& version );

    /// Retrieve sql files to upgrade database.
    /**
     * @param[in] from  current database version
     * @param[in] to  desired database version
     * @return  list of files to parse
     */
    virtual QStringList upgradeFiles( const QString& from, const QString& to ) const = 0;

    // ========================================================================
    // Methods
    // ========================================================================

    /// Bind query values.
    /**
     * @param[in,out] query  query to bind
     * @param[in] obj  object to parse
     */
    virtual void bindQueryValues( QSqlQuery& query, const QJsonObject& obj ) const;

    /// Create database.
    /**
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool create();

    /// Execute SQL statements.
    /**
     * @param[in] files  files to execute
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool execute( const QStringList& files );

    /// Open database.
    /**
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool open();

    /// Read setting from db.
    /**
     * @param[in] key  setting to read
     * @param[out] value  setting value
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool readSetting( const QString& key, QVariant& value ) const;

    /// Update default field value.
    /**
     * Set global default value when @a field is populated, otherwise take value from global default.
     * @param[in,out] query to update
     * @param[in] obj  object to parse
     * @param[in] field  field to populate
     */
    virtual void updateDefaultValue( QSqlQuery& query, const QJsonObject& obj, const QString& field );

    /// Upgrade database.
    /**
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool upgrade();

    /// Write setting to db.
    /**
     * @param[in] key  setting to write
     * @param[in] value  setting value
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool writeSetting( const QString& key, const QVariant& value );

private:

    // not implemented
    SqlDatabase( const _Myt& ) = delete;

    // not implemented
    SqlDatabase( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // SQLDB_H
