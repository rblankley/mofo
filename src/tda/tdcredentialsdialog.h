/**
 * @file tdcredentialsdialog.h
 * TD Ameritrade Credentials editor.
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

#ifndef TDCREDENTIALSDIALOG_H
#define TDCREDENTIALSDIALOG_H

#include <QDialog>

class QLabel;
class QLineEdit;
class QPushButton;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// TD Ameritrade Credentials editor.
class TDCredentialsDialog : public QDialog
{
    Q_OBJECT

    using _Myt = TDCredentialsDialog;
    using _Mybase = QDialog;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in,out] parent  parent widget
     * @param[in] f  window flags
     */
    TDCredentialsDialog( QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve callback URL.
    /**
     * Callback URL from TDA site, in the form "http://localhost:8088/mofo"
     * @return  callback url
     */
    virtual QString callbackUrl() const;

    /// Retrieve consumer key id.
    /**
     * Consumer Id from TDA site, in the form "<YOUR TDA CLIENT ID>@AMER.OAUTHAP"
     * @return  consumer id
     */
    virtual QString consumerId() const;

    /// Set callback URL.
    /**
     * @param[in] value  callback url
     */
    virtual void setCallbackUrl( const QString& value );

    /// Set consumer key id.
    /**
     * @param[in] value  consumer id
     */
    virtual void setConsumerId( const QString& value );

    /// Retrieve size hint.
    /**
     * @return  size hint
     */
    virtual QSize sizeHint() const override;

    // ========================================================================
    // Methods
    // ========================================================================

    /// Translate strings.
    virtual void translate();

protected:

    QLabel *consumerIdLabel_;
    QLineEdit *consumerId_;

    QLabel *callbackUrlLabel_;
    QLineEdit *callbackUrl_;

    QLabel *tdaDeveloperInfo_;
    QLabel *tdaLink_;

    QPushButton *okay_;
    QPushButton *cancel_;

protected slots:

    // ========================================================================
    // Methods
    // ========================================================================

    /// Validate form entry fields.
    virtual bool validateForm();

private:

    /// Initialize.
    void initialize();

    /// Create layout.
    void createLayout();

    // not implemented
    TDCredentialsDialog( const _Myt& ) = delete;

    // not implemented
    TDCredentialsDialog( const _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // TDCREDENTIALSDIALOG_H
