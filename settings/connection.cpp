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
#include "nm-settings-connectionadaptor.h"

#include <KDebug>

Connection::Connection(const QDBusConnection &connection, const QString &connPath, const QString &connName, const KConfigGroup &config, QObject *parent)
    : QObject(parent),
      conn(connection),
      wired(0),
      wireless(0)
{
    qDBusRegisterMetaType<QVariantMapMap>();
    qDBusRegisterMetaType<QVariantMap>();

    new ConnectionAdaptor(this);
    new SecretsAdaptor(this);

    this->connPath = connPath;
    this->connName = connName;
    conn.registerObject(objectPath(), this);

    QStringList ifaceTypes = config.readEntry("InterfaceTypeList", QStringList());
    QStringList ifaceNames = config.readEntry("InterfaceNameList", QStringList());

    connectionMap["name"] = "connection";
    connectionMap["id"] = connName;
    connectionMap["autoconnect"] = true;

    int nameIndex = ifaceNames.indexOf(connName.split("/").last());
    if (nameIndex != -1) {
        if (ifaceTypes[nameIndex] == "Wired") {
            connType = Wired;
            wired = new WiredConnectionSetting(config, this);
        } else if (ifaceTypes[nameIndex] == "Wireless") {
            connType = Wireless;
            wireless = new WirelessConnectionSetting(config, this);
        }
    } else {
        kDebug() << "Could not find interface name.";
    }
}

Connection::~Connection()
{
    delete wired;
    conn.unregisterObject(objectPath());
}

QString Connection::objectPath() const
{
    return connPath;
}

QString Connection::GetID() const
{
    return connName;
}

void Connection::Update(QVariantMapMap updates)
{
    Q_UNUSED(updates)
    /*if (updates.exists(NM_SETTING_WIRED_SETTING_NAME)) {
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
    emit Removed();
}

QVariantMapMap Connection::GetSettings() const
{
    QVariantMapMap retVal;
    kDebug() << "Retrieving settings.";

    switch (connType) {
        case Wired:
            kDebug() << "Retrieving wired settings.";
            if (wired) {
                retVal["802-3-ethernet"] = wired->settingsMap();
            } else {
                kDebug() << "Mode is ethernet but a settings object has not been initialized.";
            }
            break;
        case Wireless:
            kDebug() << "Retrieving wireless settings.";
            if (wireless) {
                retVal["802-11-wireless"] = wireless->settingsMap();
                retVal["802-11-wireless-security"] = wireless->secretsKeyMap();
            } else {
                kDebug() << "Mode is wireless but a settings object has not been initialized.";
            }
            break;
        default:
            break;
    }
    retVal["connection"] = connectionMap;
    kDebug() << "Returning: " << retVal;
    return retVal;
}

QVariantMapMap Connection::GetSecrets(const QString &setting_name, const QStringList &hints, bool request_new)
{
    Q_UNUSED(hints)
    Q_UNUSED(request_new)

    QVariantMapMap retVal;

    if (setting_name == "802-11-wireless-security") {
        if (wireless) {
            retVal["802-11-wireless"] = wireless->settingsMap();
        } else {
            kDebug() << "Mode is wireless but a settings object has not been initialized.";
        }
    }
    retVal["connection"] = connectionMap;
    kDebug() << "Returning: " << retVal;
    return retVal;
}

#include "connection.moc"
