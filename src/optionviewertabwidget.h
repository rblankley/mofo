/**
 * @file optionviewertabwidget.h
 * Widget for viewing options.
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

#ifndef OPTIONVIEWERTABWIDGET_H
#define OPTIONVIEWERTABWIDGET_H

#include <QDate>
#include <QList>
#include <QTabWidget>

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Widget for viewing options.
class OptionViewerTabWidget : public QTabWidget
{
    Q_OBJECT

    using _Myt = OptionViewerTabWidget;
    using _Mybase = QTabWidget;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] parent  parent object
     */
    OptionViewerTabWidget( QWidget *parent = nullptr );

    /// Destructor.
    virtual ~OptionViewerTabWidget();

    // ========================================================================
    // Methods
    // ========================================================================

    /// Create underlying.
    /**
     * @param[in] symbol  underlying to create
     */
    virtual void createUnderlying( const QString& symbol );

    /// Translate strings.
    virtual void translate();

protected:

    // ========================================================================
    // Methods
    // ========================================================================

    /// Find an option chain underlying.
    /**
     * @param[in] symbol  option chain underlying symbol
     * @param[out]  shown  @c true if underlying shown, @c false otherwise
     * @return  pointer to widget
     */
    virtual QWidget *findUnderlying( const QString& symbol, bool& shown );

private slots:

    /// Slot for option chain updated.
    void onOptionChainUpdated( const QString& symbol, const QList<QDate>& expiryDates, bool background );

    /// Slot for tab close requested.
    void onTabCloseRequested( int index );

private:

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    // not implemented
    OptionViewerTabWidget( const _Myt& other ) = delete;

    // not implemented
    OptionViewerTabWidget( const _Myt&& other ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt& rhs ) = delete;

    // not implemented
    _Myt & operator = ( const _Myt&& rhs ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // OPTIONVIEWERTABWIDGET_H
