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

//#include <nm-setting-wired.h>
//#include <nm-setting-wireless.h>

Connection::Connection(const QDBusConnection &connection, const QString &connPath, const QString &connName, const KConfigGroup &config, QObject *parent)
    : QObject(parent),
      conn(connection)
      //wired(0)
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

    int nameIndex = ifaceNames.indexOf(connName.split("/").last());
    if (nameIndex != -1) {
        if (ifaceTypes[nameIndex] == "Wired") {
            connType = Wired;
            //wired = new WiredConnectionSetting(config, this);
        } else if (ifaceTypes[nameIndex] == "Wireless") {
            connType = Wireless;
        }
    } else {
        kDebug() << "Could not find interface name.";
    }
}

Connection::~Connection()
{
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
            retVal["802-3-ethernet"]["name"] = QVariant("802-3-ethernet");
            break;
        default:
            break;
    }
    return retVal;
}

QVariantMap Connection::GetSecrets(const QString &setting_name, const QStringList &hints, bool request_new)
{
    Q_UNUSED(setting_name)
    Q_UNUSED(hints)
    Q_UNUSED(request_new)

    return QVariantMap();
}

#include "connection.moc"
