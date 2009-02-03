/*  This file is part of the KDE project
    Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>
    Copyright (C) 2008 Will Stephenson <wstephenson@kde.org>

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

#include <NetworkManager.h>
#include <nm-setting-connection.h>

#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QDBusMetaType>
#include <KDebug>

#include <solid/control/networkmanager.h>

// knmdbus
#include "nm-active-connectioninterface.h"

#include "connection.h"
#include "connectiondbus.h"
#include "busconnection.h"
#include "exportedconnection.h"

NetworkSettings::NetworkSettings(QObject * parent)
: QObject(parent), mNextConnectionId(0)
{
    //declare types
    qDBusRegisterMetaType<QList<QDBusObjectPath> >();

    QDBusConnection dbus = QDBusConnection::systemBus();
    if (!dbus.registerObject(QLatin1String(NM_DBUS_PATH_SETTINGS), this, QDBusConnection::ExportScriptableContents)) {
        kDebug() << "Unable to register settings object " << NM_DBUS_PATH_SETTINGS;
    } else {
        kDebug() << "Registered settings object " << NM_DBUS_PATH_SETTINGS;
    }

    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(activeConnectionsChanged()),
            SLOT(activeConnectionsChanged()));
    // build the list of active connections
    foreach (QString activePath, Solid::Control::NetworkManager::activeConnections()) {
        OrgFreedesktopNetworkManagerConnectionActiveInterface activeIface(QLatin1String(NM_DBUS_SERVICE), activePath, QDBusConnection::systemBus(), 0);
        if (activeIface.serviceName() == QLatin1String(NM_DBUS_SERVICE_USER_SETTINGS)) {
            m_ourActiveConnections.append(activeIface.connection().path());
        }
    }
}

NetworkSettings::~NetworkSettings()
{
}

QString NetworkSettings::addConnection(Knm::Connection * connection)
{
    kDebug();
    QString objectPath;
    if (connection) {
        BusConnection * busConn = new BusConnection(connection, this);
        new ConnectionAdaptor(busConn);
        new SecretsAdaptor(busConn);
        objectPath = nextObjectPath();
        m_connectionMap.insert(objectPath, busConn);
        QDBusConnection::systemBus().registerObject(objectPath, busConn, QDBusConnection::ExportAdaptors);
        emit NewConnection(QDBusObjectPath(objectPath));
        kDebug() << "NewConnection" << objectPath;
    }
    return objectPath;
}

void NetworkSettings::updateConnection(const QString & objectPath, Knm::Connection * connection)
{
    kDebug() << objectPath << connection->uuid();
    if (m_connectionMap.contains(objectPath)) {
        BusConnection * busConn = m_connectionMap[objectPath];
        if (busConn) {
            busConn->updateInternal(connection);
        }
    }
}

void NetworkSettings::removeConnection(const QString & objectPath)
{
    kDebug() << objectPath;
    BusConnection * conn = m_connectionMap.take(objectPath);
    conn->Delete();
}

QList<QDBusObjectPath> NetworkSettings::ListConnections() const
{
    QList<QDBusObjectPath> pathList;
    kDebug() << "There are " << m_connectionMap.keys().count() << " known connections";
    foreach(const QString &connPath, m_connectionMap.keys()) {
        pathList << QDBusObjectPath(connPath);
    }
    return pathList;
}

QString NetworkSettings::nextObjectPath()
{
    return QString::fromLatin1("%1/%2").arg(QLatin1String(NM_DBUS_PATH_SETTINGS)).arg(mNextConnectionId++);
}

void NetworkSettings::onConnectionRemoved()
{
#if 0
    foreach (const QString connName, m_connectionMap.keys()) {
        if (sender() == m_connectionMap[connName]) {
            disconnect(m_connectionMap[connName], SIGNAL(Removed()));
            delete m_connectionMap[connName];
            m_connectionMap.remove(connName);
        }
    }
#endif
}

void NetworkSettings::clearConnections()
{
    foreach (const QString &conn, m_connectionMap.keys()) {
        m_connectionMap[conn]->Delete();
    }
}

void NetworkSettings::activeConnectionsChanged()
{
    kDebug();
    QStringList existingConnections = m_ourActiveConnections;
    QStringList newlyActiveConnections;

    m_ourActiveConnections.clear();

    foreach (QString activePath, Solid::Control::NetworkManager::activeConnections()) {
        OrgFreedesktopNetworkManagerConnectionActiveInterface activeIface(QLatin1String(NM_DBUS_SERVICE), activePath, QDBusConnection::systemBus(), 0);
        if (activeIface.serviceName() == QLatin1String(NM_DBUS_SERVICE_USER_SETTINGS)) {
            QString localConnectionPath = activeIface.connection().path();
            if (!existingConnections.contains(localConnectionPath)) {
                newlyActiveConnections.append(localConnectionPath);
            }
            m_ourActiveConnections.append(activeIface.connection().path());
        }
    }

    foreach (QString newlyActive, newlyActiveConnections) {
        BusConnection * conn = m_connectionMap.value(newlyActive);
        if (conn) {
            emit connectionActivated(conn->uuid());
        }
    }
}

#include "networksettings.moc"
