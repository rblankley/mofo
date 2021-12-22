/**
 * @file common.h
 * Common (shared) includes.
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

#ifndef COMMON_H
#define COMMON_H

#include <QtGlobal>

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

// ============================================================================
// Paths
// ============================================================================

#if defined( Q_OS_WIN )
#define SYS_CONF_DIR "./"
#define USER_CONF_DIR "./"
#define USER_CACHE_DIR "./"
#elif defined( Q_OS_LINUX )
#define SYS_CONF_DIR "/etc/mofo/"
#define USER_CONF_DIR QString( qgetenv( "HOME" ) ) + "/.config/mofo/"
#define USER_CACHE_DIR QString( qgetenv( "HOME" ) ) + "/.cache/mofo/"
#endif

// ============================================================================
// Logging
// ============================================================================

#if defined(HAVE_CLIO_H)
#include <clio.h>

#else
#include <QDebug>
#define LOG_FATAL qCritical()
#define LOG_ERROR qCritical()
#define LOG_WARN qWarning()
#define LOG_INFO qInfo()
#define LOG_DEBUG qDebug()
#define LOG_TRACE qDebug()

#endif

// ============================================================================
// Qt Backward Compat
// ============================================================================

#if QT_VERSION_CHECK( 5, 14, 0 ) <= QT_VERSION
#define KEEP_EMPTY_PARTS Qt::KeepEmptyParts
#define SKIP_EMPTY_PARTS Qt::SkipEmptyParts
#else
#define KEEP_EMPTY_PARTS QString::KeepEmptyParts
#define SKIP_EMPTY_PARTS QString::SkipEmptyParts
#endif

// backward compatible support for streaming of QJsonObject
#if QT_VERSION < QT_VERSION_CHECK( 5, 13, 0 )

#include <QByteArray>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

inline QDataStream& operator<< ( QDataStream& stream, const QJsonDocument& doc )
{
    stream << doc.toJson(QJsonDocument::Compact);
    return stream;
}

inline QDataStream& operator>> ( QDataStream& stream, QJsonDocument& doc )
{
    QByteArray buffer;
    stream >> buffer;
    QJsonParseError parseError;
    doc = QJsonDocument::fromJson( buffer, &parseError );
    if (( parseError.error ) && ( !buffer.isEmpty() ))
        stream.setStatus( QDataStream::ReadCorruptData );
    return stream;
}

inline QDataStream& operator<< ( QDataStream& stream, const QJsonObject& object )
{
    QJsonDocument doc{object};
    stream << doc.toJson( QJsonDocument::Compact );
    return stream;
}

inline QDataStream& operator>> ( QDataStream& stream, QJsonObject& object )
{
    QJsonDocument doc;
    stream >> doc;
    object = doc.object();
    return stream;
}
#endif

// ============================================================================
// Helper Methods
// ============================================================================

#include <QString>

#include <iomanip>
#include <iostream>
#include <limits>
#include <locale>
#include <sstream>

template <typename T>
inline QString int_to_hex( T val, size_t width = sizeof(T)*2 )
{
    static std::locale l( "" );

    std::stringstream ss;
    ss.imbue( l ); // remove commas from formatting

    ss << std::setfill('0') << std::setw(width) << std::hex << (val|0);

    return QString::fromStdString( ss.str() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // COMMON_H
