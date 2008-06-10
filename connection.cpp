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
#include "connectionadaptor.h"
#include "marshallarguments.h"

#include <NetworkManager.h>

Connection::Connection(QObject *parent)
    : QObject(parent)
{
    qDBusRegisterMetaType< QMap<QString, QMap<QString, QVariant> > >();
    qDBusRegisterMetaType< QMap<QString, QVariant> >();

    new ConnectionAdaptor(this);

    QDBusConnection::systemBus().registerObject(objectPath(), this);
}

Connection::~Connection()
{
    QDBusConnection::systemBus().unregisterObject(objectPath());
}

QString Connection::objectPath()
{
    return QString(NM_DBUS_PATH_SETTINGS_CONNECTION);
}

QString Connection::GetID() const
{
    return settingsMap["name"];
}

void Connection::Update(QMap<QString, QMap<QString, QVariant> > changedParameters)
{
    foreach (const QString &key1, changedParameters.keys()) {
        foreach (const QString &key2, changedParameters[key1].keys()) {
            connectionMap[key1][key2] = changedParameters[key1][key2];
        }
    }
    emit Updated(changedParameters);
}

void Connection::Delete()
{
    emit Removed()
}

QMap<QString, QMap<QString, QVariant> > Connection::GetSettings()
{
    return settingsMap;
}

#include "connection.moc"
