/*
Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>
Copyright (C) 2008 Will Stephenson <wstephenson@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KNM07_TYPES_H
#define KNM07_TYPES_H

#include <QMap>
#include <QVariant>
#include <QtDBus/QtDBus>
#include <QtDBus/QDBusArgument>

#include <NetworkManager.h>

/*
Q_DECLARE_METATYPE(QList<uint>)
Q_DECLARE_METATYPE(QList< QList<uint> >)
typedef QList< QList<uint> > UintListList;
Q_DECLARE_METATYPE(QVariantMap)
typedef QMap<QString, QMap<QString, QVariant> > QVariantMapMap;
Q_DECLARE_METATYPE(QVariantMapMap)
typedef QMap<QString, QString> QStringMap;
typedef QMapIterator<QString, QString> QStringMapIterator;
Q_DECLARE_METATYPE(QStringMap)
*/

// Device states of NM 0.9
enum NM09DeviceState {
    UnknownState = 0   /* NM_DEVICE_STATE_UNKNOWN */,
    Unmanaged    = 10  /* NM_DEVICE_STATE_UNMANAGED */,
    Unavailable  = 20  /* NM_DEVICE_STATE_UNAVAILABLE */,
    Disconnected = 30  /* NM_DEVICE_STATE_DISCONNECTED */,
    Preparing    = 40  /* NM_DEVICE_STATE_PREPARE */,
    Configuring  = 50  /* NM_DEVICE_STATE_CONFIG */,
    NeedAuth     = 60  /* NM_DEVICE_STATE_NEED_AUTH */,
    IPConfig     = 70  /* NM_DEVICE_STATE_IP_CONFIG */,
    IPCheck      = 80  /* NM_DEVICE_STATE_IP_CHECK */,
    Secondaries  = 90  /* NM_DEVICE_STATE_SECONDARIES */,
    Activated    = 100 /* NM_DEVICE_STATE_ACTIVATED */,
    Deactivating = 110 /* NM_DEVICE_STATE_DEACTIVATING */,
    Failed       = 120 /* NM_DEVICE_STATE_FAILED */
};

#endif
