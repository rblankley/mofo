/**
 * @file stringsoauth.h
 * String values for TDA OAuth flow.
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

#ifndef STRINGSOAUTH_H
#define STRINGSOAUTH_H

#include <QString>

///////////////////////////////////////////////////////////////////////////////////////////////////

inline static const QString OAUTH_WEB                       = "web";

inline static const QString OAUTH_CLIENT_ID                 = "client_id";
inline static const QString OAUTH_CLIENT_SECRET             = "client_secret";

inline static const QString OAUTH_REDIRECT_URIS             = "redirect_uris";
inline static const QString OAUTH_REDIRECT_URI              = "redirect_uri";

inline static const QString OAUTH_AUTH_URI                  = "auth_uri";
inline static const QString OAUTH_AUTH_PROVIDER_CERT        = "auth_provider_x509_cert_url";
inline static const QString OAUTH_TOKEN_URI                 = "token_uri";

inline static const QString OAUTH_CODE                      = "code";

inline static const QString OAUTH_REFRESH_TOKEN             = "refresh_token";
inline static const QString OAUTH_REFRESH_TOKEN_EXPIRY      = "refresh_token_expiry";
inline static const QString OAUTH_ACCESS_TOKEN              = "access_token";

inline static const QString OAUTH_TOKEN_TYPE                = "token_type";
inline static const QString OAUTH_SCOPE                     = "scope";

inline static const QString OAUTH_REFRESH_TOKEN_EXP_IN      = "refresh_token_expires_in";
inline static const QString OAUTH_ACCESS_TOKEN_EXP_IN       = "expires_in";

inline static const QString OAUTH_ACCESS_TYPE               = "access_type";
inline static const QString OAUTH_OFFLINE                   = "offline";

inline static const QString OAUTH_GRANT_TYPE                = "grant_type";
inline static const QString OAUTH_AUTH_CODE                 = "authorization_code";

inline static const QString OAUTH_ERROR                     = "error";
inline static const QString OAUTH_INVALID_GRANT             = "invalid_grant";


///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // STRINGSOAUTH_H
