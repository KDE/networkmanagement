/*  This file is part of the KDE project
    Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>
    Copyright (C) 2008,2009 Will Stephenson <wstephenson@kde.org>

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
#include <solid/control/networkinterface.h>

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
    // watch status on all devices
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceAdded(const QString&)),
            this, SLOT(networkInterfaceAdded(const QString&)));

    Solid::Control::NetworkInterfaceList allInterfaces = Solid::Control::NetworkManager::networkInterfaces();
    foreach (Solid::Control::NetworkInterface * interface, allInterfaces) {
        connect(interface, SIGNAL(connectionStateChanged(int)), this, SLOT(networkInterfaceConnectionStateChanged(int)));
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

void NetworkSettings::clearConnections()
{
    foreach (const QString &conn, m_connectionMap.keys()) {
        m_connectionMap[conn]->Delete();
    }
}

void NetworkSettings::networkInterfaceAdded(const QString& uni)
{
    Solid::Control::NetworkInterface * interface = Solid::Control::NetworkManager::findNetworkInterface(uni);
    disconnect(interface, SIGNAL(connectionStateChanged(int)), this, SLOT(networkInterfaceConnectionStateChanged(int)));
    connect(interface, SIGNAL(connectionStateChanged(int)), this, SLOT(networkInterfaceConnectionStateChanged(int)));
}

void NetworkSettings::networkInterfaceConnectionStateChanged(int state)
{
    Solid::Control::NetworkInterface * device = qobject_cast<Solid::Control::NetworkInterface *>(sender());
    if (device && device->connectionState() == Solid::Control::NetworkInterface::Activated) {
        QStringList active(Solid::Control::NetworkManager::activeConnections());
        foreach (QString activePath, active) {
            OrgFreedesktopNetworkManagerConnectionActiveInterface activeIface(QLatin1String(NM_DBUS_SERVICE), activePath, QDBusConnection::systemBus(), 0);
            if (activeIface.serviceName() == QLatin1String(NM_DBUS_SERVICE_USER_SETTINGS)) {
                QList<QDBusObjectPath> activeDevices = activeIface.devices();
                if (activeDevices.contains(QDBusObjectPath(device->uni()))) {
                    kDebug() << "Found connection for device";
                    BusConnection * bc = m_connectionMap.value(activeIface.connection().path());
                    if (bc) {
                        emit connectionActivated(bc->uuid());
                    }
                }
            }
        }
    }
}

#include "networksettings.moc"
