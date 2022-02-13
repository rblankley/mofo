/**
 * @file usdotapi.h
 * U.S. Department of the Treasury API implementation.
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

#ifndef USDOT_H
#define USDOT_H

#include "../apibase/serializedxmlapi.h"

#include <QMutex>

///////////////////////////////////////////////////////////////////////////////////////////////////

/// U.S. Department of the Treasury API implementation.
class DeptOfTheTreasury : public SerializedXmlWebInterface
{
    Q_OBJECT

    using _Myt = DeptOfTheTreasury;
    using _Mybase = SerializedXmlWebInterface;

signals:

    /// Signal for daily treasury bill rates received.
    /**
     * @param[in] doc  data
     */
    void dailyTreasuryBillRatesReceived( const QDomDocument& doc );

    /// Signal for daily treasury yield curve rates received.
    /**
     * @param[in] doc  data
     */
    void dailyTreasuryYieldCurveRatesReceived( const QDomDocument& doc );

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in,out] parent  parent object
     */
    DeptOfTheTreasury( QObject *parent = nullptr );

    /// Destructor.
    ~DeptOfTheTreasury();

    // ========================================================================
    // Methods
    // ========================================================================

    /// Retrieve daily treasury bill rates.
    /**
     * @param[in] year  year, or 0 for current year
     * @param[in] month  month, or 0 for current month
     */
    virtual void getDailyTreasuryBillRates( int year = 0, int month = 0 );

    /// Retrieve daily treasury yield curve rates.
    /**
     * @param[in] year  year, or 0 for current year
     * @param[in] month  month, or 0 for current month
     */
    virtual void getDailyTreasuryYieldCurveRates( int year = 0, int month = 0 );

    // ========================================================================
    // Methods
    // ========================================================================

#ifdef QT_DEBUG
    /// Simulate daily treasury bill rates.
    /**
     * @param[in] doc  document
     */
    virtual void simulateDailyTreasuryBillRates( const QDomDocument& doc );

    /// Simulate daily treasury yield curve rates.
    /**
     * @param[in] doc  document
     */
    virtual void simulateDailyTreasuryYieldCurveRates( const QDomDocument& doc );
#endif

private slots:

    /// Slot to process document.
    void onProcessDocumentXml( const QUuid& uuid, const QByteArray& request, const QString& requestType, int status, const QDomDocument& response );

private:

    static constexpr int REQUEST_TIMEOUT = 30 * 1000;       // 30s
    static constexpr int REQUEST_RETRIES = 3;

    enum Endpoint
    {
        GET_DAILY_TREASURY_BILL_RATES,
        GET_DAILY_TREASURY_YIELD_CURVE_RATES,
    };

    using EndpointMap = QMap<Endpoint, QString>;

    EndpointMap endpointNames_;
    EndpointMap endpoints_;

    using PendingRequestsMap = QMap<QUuid, Endpoint>;

    mutable QMutex m_;

    PendingRequestsMap pendingRequests_;

    /// Load endpoints.
    void loadEndpoints();

    /// Parse daily treasury bill rates.
    void parseDailyTreasuryBillRatesDoc( const QDomDocument& doc );

    /// Parse daily treasury yield curve rates.
    void parseDailyTreasuryBillYieldCurveDoc( const QDomDocument& doc );

    // not implemented
    DeptOfTheTreasury( const _Myt& ) = delete;

    // not implemented
    DeptOfTheTreasury( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // USDOT_H
