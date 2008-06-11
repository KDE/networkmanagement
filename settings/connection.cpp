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
#include "marshallarguments.h"

#include <nm-setting-wired.h>
#include <nm-setting-wireless.h>

Connection::Connection(const QString profile, const QString &interfaceName, const KConfigGroup &config, QObject *parent)
    : QObject(parent),
      wired(0)
{
    qDBusRegisterMetaType< QMap<QString, QMap<QString, QVariant> > >();
    qDBusRegisterMetaType< QMap<QString, QVariant> >();

    new ConnectionAdaptor(this);

    //connection name = "profile - interfaceName"
    QString connName = profile + '-';
    connName += interfaceName;
    QDBusConnection::systemBus().registerObject(objectPath(), this);

    QStringList ifaceTypes = config.readEntry("InterfaceTypeList", QStringList()).toStringList();
    QStringList ifaceNames = config.readEntry("InterfaceNameList", QStringList()).toStringList();

    int nameIndex = ifaceNames.indexOf(interfaceName);
    if (nameIndex != -1) {
        if (ifaceTypes[nameIndex] == "Wired") {
            connType = Wired;
            wired = new WiredConnectionSetting(config, this);
        } else if (ifaceTypes[nameIndex] == "Wireless") {
            connType = Wireless;
        }
}

Connection::~Connection()
{
    QDBusConnection::systemBus().unregisterObject(objectPath());
}

QString Connection::objectPath()
{
    return QString(NM_DBUS_PATH_SETTINGS_CONNECTION + "/" + connectionName);
}

QString Connection::GetID() const
{
    return connectionName;
}

void Connection::Update(QMap<QString, QMap<QString, QVariant> > updates)
{
    if (updates.exists(NM_SETTING_WIRED_SETTING_NAME)) {
        if (wired == 0) {
            wired = new WiredConnectionSetting(setting, this);
        }
        if (wired) {
            wired->update(updates[NM_SETTING_WIRED_SETTING_NAME]);
        }
    }
    /*if (updates.exists(NM_SETTING_WIRELESS_SETTING_NAME)) {
    }*/
}

void Connection::Delete()
{
    emit Removed()
}

QMap<QString, QMap<QString, QVariant> > Connection::GetSettings() const
{
    QMap<QString, QMap<QString, QVariant> > retVal;
    switch (connType) {
        case Wired:

    }
}

#include "connection.moc"
