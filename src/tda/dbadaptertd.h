/**
 * @file dbadaptertd.h
 * TD Ameritrade database adpater.
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

#ifndef DBADAPTERTD_H
#define DBADAPTERTD_H

#include <QDateTime>
#include <QJsonObject>
#include <QMap>
#include <QObject>

class QMutex;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// TD Ameritrade database adpater.
/**
 * Transform TDA JSON responses into JSON format used by app database. Because this software was
 * written against the TDA API the format is very similar.
 */
class TDAmeritradeDatabaseAdapter : public QObject
{
    Q_OBJECT

    using _Myt = TDAmeritradeDatabaseAdapter;
    using _Mybase = QObject;

signals:

    /// Signal for transform complete.
    /**
     * @param[in] obj  data
     */
    void transformComplete( const QJsonObject& obj ) const;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in, out] parent  parent object
     */
    TDAmeritradeDatabaseAdapter( QObject *parent = nullptr );

    /// Destructor.
    ~TDAmeritradeDatabaseAdapter();

public slots:

    // ========================================================================
    // Methods
    // ========================================================================

    /// Transform accounts to database format.
    /**
     * @param[in] a  data objects
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool transformAccounts( const QJsonArray& a ) const;

    /// Transform market hours to database format.
    /**
     * @param[in] obj  data
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool transformMarketHours( const QJsonObject& obj ) const;

    /// Transform option chain to database format.
    /**
     * @param[in] obj  data
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool transformOptionChain( const QJsonObject& obj ) const;

    /// Transform price history to database format.
    /**
     * @param[in] obj  data
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool transformPriceHistory( const QJsonObject& obj ) const;

    /// Transform quotes to database format.
    /**
     * @param[in] obj  data
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool transformQuotes( const QJsonObject& obj ) const;

private:

    /// Field map type.
    using FieldMap = QMap<QString, QString>;

    QStringList dateColumns_;
    QStringList dateTimeColumns_;

    FieldMap accountFields_;
    FieldMap marketHoursFields_;
    FieldMap optionChainFields_;
    FieldMap priceHistoryFields_;
    FieldMap quoteFields_;

    FieldMap balances_;
    FieldMap sessionHours_;

    /// Transform json object.
    void transform( const QJsonObject& obj, const FieldMap& mapping, QJsonObject& result ) const;

    /// Transform json object complete.
    void complete( const QJsonObject& obj ) const;

    /// Parse account.
    QJsonObject parseAccount( const QJsonObject& obj ) const;

    /// Parse market hours.
    void parseMarketHours( const QJsonObject& obj, QJsonArray *result ) const;

    /// Parse options (concurrent version).
    void parseOptionChain( const QJsonObject::const_iterator& it, double underlyingPrice, QJsonArray *result, QMutex *m ) const;

    /// Parse price history.
    QJsonArray parsePriceHistory( const QJsonArray& a ) const;

    /// Parse quote.
    QJsonObject parseQuote( const QJsonObject& obj, const QDateTime& stamp = QDateTime() ) const;

    /// Parse session hours.
    QJsonObject parseSessionHours( const QJsonObject& obj ) const;

    /// Save object.
    static void saveObject( const QJsonObject& obj, const QString& filename );

    // not implemented
    TDAmeritradeDatabaseAdapter( const _Myt& ) = delete;

    // not implemented
    TDAmeritradeDatabaseAdapter( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // DBADAPTERTD_H
