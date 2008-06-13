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

#include "networksettings.h"
#include "nm-settingsadaptor.h"

#include <QDBusObjectPath>

#include <KDebug>

NetworkSettings::NetworkSettings(const KConfigGroup &config, QObject *parent)
    : QObject(parent),
      m_settings(config),
      m_conn(QDBusConnection::systemBus()),
      m_valid(false)
{
    //declare types
    qDBusRegisterMetaType< QList<QDBusObjectPath> >();

    new NetworkManagerSettingsAdaptor(this);

    if (m_conn.registerService(NM_DBUS_SERVICE_USER_SETTINGS)) {
        m_conn.registerObject(objectPath(), this);
        m_valid=true;
    } else {
        kDebug() << "Service is already occupied.";
        m_valid=false;
    }
}

NetworkSettings::~NetworkSettings()
{
    if (m_valid) {
        clearConnections();

        m_conn.unregisterObject(objectPath());
        m_conn.unregisterService(NM_DBUS_SERVICE_USER_SETTINGS);
    }
}

bool NetworkSettings::loadProfile(const QString &profile)
{
    kDebug() << "Loading " << profile;
    clearConnections();
    if (!m_settings.groupList().contains(profile)) { //profile does not exist so return
        kDebug() << "Profile " << profile << " does not exist.";
        return false;
    }

    KConfigGroup profileConfig(&m_settings, profile);
    QStringList interfaceNames = profileConfig.readEntry("InterfaceNameList", QStringList());
    foreach (const QString &interfaceName, interfaceNames) {
        QString connName = profile + QString('/') + interfaceName;
        QString connPath = QString(NM_DBUS_PATH_SETTINGS) + QString('/') + connName;
        kDebug() << "Adding: " << connPath;
        m_connectionMap[connName] = new Connection(m_conn,connPath, connName, profileConfig, this);
        connect(m_connectionMap[connName], SIGNAL(Removed()), this, SLOT(onConnectionRemoved()));
    }
    return true;
}

bool NetworkSettings::isValid() const
{
    return m_valid;
}

QDBusConnection NetworkSettings::dbusConnection() const
{
    return m_conn;
}

QList<QDBusObjectPath> NetworkSettings::ListConnections() const
{
    QList<QDBusObjectPath> pathList;
    kDebug() << "There are " << m_connectionMap.keys().count() << " known connections";
    foreach(const QString &connName, m_connectionMap.keys()) {
        pathList << QDBusObjectPath(m_connectionMap[connName]->objectPath());
        kDebug() << connName << " = " << m_connectionMap[connName]->objectPath();
    }
    return pathList;
}

void NetworkSettings::onConnectionRemoved()
{
    foreach (const QString connName, m_connectionMap.keys()) {
        if (sender() == m_connectionMap[connName]) {
            disconnect(m_connectionMap[connName], SIGNAL(Removed()));
            delete m_connectionMap[connName];
            m_connectionMap.remove(connName);
        }
    }
}

void NetworkSettings::clearConnections()
{
    foreach (const QString &conn, m_connectionMap.keys()) {
        m_connectionMap[conn]->Delete();
    }
}

QString NetworkSettings::objectPath() const
{
    return QString(NM_DBUS_PATH_SETTINGS);
}

#include "networksettings.moc"
