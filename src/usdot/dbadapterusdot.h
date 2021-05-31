/**
 * @file dbadapterusdot.h
 * US Dept. of the Treasury database adpater.
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

#ifndef DBADAPTERUSDOT_H
#define DBADAPTERUSDOT_H

#include <QMap>
#include <QObject>

class QDomDocument;
class QJsonDocument;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// US Dept. of the Treasury database adpater.
/**
 * Transform US DotT ATOM XML responses into JSON format used by app database.
 */
class DeptOfTheTreasuryDatabaseAdapter : public QObject
{
    Q_OBJECT

    using _Myt = DeptOfTheTreasuryDatabaseAdapter;
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
    DeptOfTheTreasuryDatabaseAdapter( QObject *parent = nullptr );

    /// Destructor.
    ~DeptOfTheTreasuryDatabaseAdapter();

public slots:

    // ========================================================================
    // Methods
    // ========================================================================

    /// Transform daily treasury bill rates to database format.
    /**
     * @param[in] doc  data
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool transformDailyTreasuryBillRates( const QDomDocument& doc ) const;

    /// Transform daily treasury yield curve rates to database format.
    /**
     * @param[in] doc  data
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool transformDailyTreasuryYieldCurveRates( const QDomDocument& doc ) const;

private:

    /// Rate map type.
    using RateMap = QMap<QString, int>;

    RateMap yieldCurveRates_;

    /// Transform json object complete.
    void complete( const QJsonObject& obj ) const;

    /// Save object.
    static void saveObject( const QJsonObject& obj, const QString& filename );

    // not implemented
    DeptOfTheTreasuryDatabaseAdapter( const _Myt& ) = delete;

    // not implemented
    DeptOfTheTreasuryDatabaseAdapter( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // DBADAPTERUSDOT_H
