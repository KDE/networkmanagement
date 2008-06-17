/*  This file is part of the KDE project
    Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "connection.h"

#include <QDBusMetaType>
#include <KDebug>

//#include "connectionadaptor.h"
//#include "marshallarguments.h"
//#include <nm-setting-wireless.h>
typedef QMap<QString,QVariantMap> QVariantMapMap;
//Q_DECLARE_METATYPE(QVariantMapMap)

Connection::Connection(QObject *parent)
    : QObject(parent)
{
    qDBusRegisterMetaType<QVariantMapMap>();
}

Connection::~Connection()
{
    emit Removed();
}

QString Connection::id() const
{
    return mId;
}

QString Connection::GetID() const
{
    return id();
}

void Connection::Update(QVariantMapMap updates)
{
#if 0
    foreach (const QString &key1, changedParameters.keys()) {
        foreach (const QString &key2, changedParameters[key1].keys()) {
            connectionMap[key1][key2] = changedParameters[key1][key2];
        }
        if (wired) {
            wired->update(updates[NM_SETTING_WIRED_SETTING_NAME]);
        }
    }
    /*if (updates.exists(NM_SETTING_WIRELESS_SETTING_NAME)) {
    }*/
#endif
}

void Connection::Delete()
{
    deleteLater();
}

QVariantMapMap Connection::GetSettings() const
{
    return mSettingsMap;
}

QVariantMap Connection::GetSecrets(const QString &setting_name, const QStringList &hints, bool request_new)
{
    Q_UNUSED(setting_name)
    Q_UNUSED(hints)
    Q_UNUSED(request_new)

    return QVariantMap();
}

#include "connection.moc"
