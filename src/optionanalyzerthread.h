/**
 * @file optionanalyzerthread.h
 * Stock option analysis thread.
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

#ifndef OPTIONANALYZERTHREAD_H
#define OPTIONANALYZERTHREAD_H

#include <QDate>
#include <QString>
#include <QThread>

class OptionTradingItemModel;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Stock option analysis thread.
class OptionAnalyzerThread : public QThread
{
    Q_OBJECT
    Q_PROPERTY( QString filter READ filter WRITE setFilter )

    using _Myt = OptionAnalyzerThread;
    using _Mybase = QThread;

public:

    /// Model type.
    using model_type = OptionTradingItemModel;

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] symbol  underlying symbol
     * @param[in] expiryDates  expiration dates
     * @param[in] model  trading model
     * @param[in] parent  parent object
     */
    OptionAnalyzerThread( const QString& symbol, const QList<QDate>& expiryDates, model_type *model, QObject *parent = nullptr );

    /// Destructor.
    virtual ~OptionAnalyzerThread();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve filter name.
    /**
     * @return  filter name
     */
    virtual QString filter() const {return filter_;}

    /// Set filter.
    /**
     * @param[in] value  filter name
     */
    virtual void setFilter( const QString& value ) {filter_ = value;}

protected:

    model_type *analysis_;                          ///< Trading model.

    QString symbol_;                                ///< Symbol.
    QList<QDate> expiryDates_;                      ///< Expiration dates.

    QString filter_;                                ///< Custom filter.

    // ========================================================================
    // Methods
    // ========================================================================

    /// Thread run method.
    virtual void run() override;

private:

    // not implemented
    OptionAnalyzerThread( const _Myt& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // OPTIONANALYZERTHREAD_H
