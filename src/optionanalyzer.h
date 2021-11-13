/**
 * @file optionanalyzer.h
 * Stock option analysis object.
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

#ifndef OPTIONANALYZER_H
#define OPTIONANALYZER_H

#include <QDate>
#include <QList>
#include <QObject>

class OptionTradingItemModel;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Stock option analysis object.
class OptionAnalyzer : public QObject
{
    Q_OBJECT
    Q_PROPERTY( bool active READ isActive NOTIFY activeChanged )

    using _Myt = OptionAnalyzer;
    using _Mybase = QObject;

public:

    /// Model type.
    using model_type = OptionTradingItemModel;

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] model  trading model
     * @param[in] parent  parent object
     */
    OptionAnalyzer( model_type *model, QObject *parent = nullptr );

    /// Destructor.
    virtual ~OptionAnalyzer();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve analysis is active.
    /**
     * @return  @c true if active, @c false otherwise
     */
    virtual bool isActive() const;

    /// Retrieve anlysis trading model object.
    /**
     * @return  model
     */
    virtual model_type *model() const {return analysis_;}

    /// Set custom filter.
    /**
     * @param[in] value  filter name
     */
    virtual void setCustomFilter( const QString& value ) {customFilter_ = value;}

public slots:

    // ========================================================================
    // Methods
    // ========================================================================

    /// Reset custom filter.
    virtual void resetCustomFilter() {customFilter_.clear();}

signals:

    /// Signal for active changed.
    /**
     * @param[in] newValue  @c true if active, @c false otherwise
     */
    void activeChanged( bool newValue );

    /// Signal for analysis complete.
    void complete();

    /// Signal for status message changed.
    /**
     * @param[in] message  status message
     * @param[in] timeout  message timeout
     */
    void statusMessageChanged( const QString& message, int timeout = 0 );

protected:

    bool active_;                                   ///< Active flag.
    QStringList symbols_;                           ///< Symbols being processed.

    model_type *analysis_;                          ///< Trading model.

    QString customFilter_;                          ///< Custom filter.

private slots:

    /// Slot for when option chain background process goes active (or deactive).
    void onOptionChainBackgroundProcess( bool active, const QStringList& symbols );

    /// Slot for option chain updated.
    void onOptionChainUpdated( const QString& symbol, const QList<QDate>& expiryDates, bool background );

    /// Slot for worker finished.
    void onWorkerFinished();

private:

    static constexpr bool THROTTLE = true;
    static constexpr int THROTTLE_YIELD_TIME = 128;

    int symbolsTotal_;

    int numThreads_;
    int numThreadsComplete_;

    double progress_;

    int workers_;
    int maxWorkers_;

    QDateTime start_;
    QDateTime stop_;

    // not implemented
    OptionAnalyzer( const _Myt& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // OPTIONANALYZER_H
