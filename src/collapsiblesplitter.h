/**
 * @file collapsiblesplitter.h
 * Splitter that can collapse.
 *
 * @copyright Copyright (C) 2022 Randy Blankley. All rights reserved.
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

#ifndef COLLAPSIBLESPLITTER_H
#define COLLAPSIBLESPLITTER_H

#include <QMap>
#include <QSplitter>

class QToolButton;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Splitter that can collapse.
class CollapsibleSplitter : public QSplitter
{
    Q_OBJECT
    Q_PROPERTY( Qt::Edge buttonLocation READ buttonLocation WRITE setButtonLocation )

    using _Myt = CollapsibleSplitter;
    using _Mybase = QSplitter;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] orientation  splitter orientation
     * @param[in,out] parent  parent
     */
    explicit CollapsibleSplitter( Qt::Orientation orientation, QWidget *parent = nullptr );

    /// Constructor.
    /**
     * @param[in,out] parent  parent
     */
    explicit CollapsibleSplitter( QWidget *parent = nullptr );

    /// Destructor.
    virtual ~CollapsibleSplitter();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve button location.
    /**
     * @return  button location
     */
    virtual Qt::Edge buttonLocation() const {return loc_;}

    /// Set button location.
    /**
     * @param[in] location  button location
     */
    virtual void setButtonLocation( Qt::Edge location ) {loc_ = location;}

    // ========================================================================
    // Methods
    // ========================================================================

    /// Add widget to splitter.
    /**
     * @brief addWidget
     * @param widget
     */
    virtual void addWidget( QWidget *widget );

private slots:

    /// Slot for button pressed.
    void onButtonPressed();

private:

    Qt::Edge loc_;

    QMap<QToolButton*, int> buttons_;

    QList<int> sizes_;

    // not implemented
    CollapsibleSplitter( const _Myt& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // COLLAPSIBLESPLITTER_H
