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
#include <KDebug>

#include <solid/control/networkmanager.h>

// knmdbus
#include "nm-active-connectioninterface.h"

#include "connection.h"
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

QString NetworkSettings::addConnection(const QVariantMapMap& settings)
{
    kDebug();
    QVariantMapMap::const_iterator it = settings.find(QLatin1String(NM_SETTING_CONNECTION_SETTING_NAME));
    if (it != settings.end()) {
        QVariantMap connectionSettings = it.value();
        QVariantMap::const_iterator connectionSettingsIt = connectionSettings.find(QLatin1String(NM_SETTING_CONNECTION_UUID));
        if (connectionSettingsIt != connectionSettings.end()) {
            Connection * connection = new Connection(connectionSettingsIt.value().toString(), settings, this);
            new ConnectionAdaptor(connection);
            new SecretsAdaptor(connection);
            QString objectPath = nextObjectPath();
            m_connectionMap.insert(objectPath, connection);
            QDBusConnection::systemBus().registerObject(objectPath, connection, QDBusConnection::ExportAdaptors);
            emit NewConnection(QDBusObjectPath(objectPath));
            return objectPath;
        } else {
        kDebug() << "Received connection settings map without a connection ID! " << NM_SETTING_CONNECTION_ID;
        }
    } else {
        kDebug() << "Received connection settings map without a name! " << NM_SETTING_CONNECTION_SETTING_NAME;
    }
    return QString();
}

void NetworkSettings::updateConnection(const QString & objectPath, const QVariantMapMap & settings )
{
    kDebug() << objectPath << settings;
    if (m_connectionMap.contains(objectPath)) {
        Connection * conn = m_connectionMap[objectPath];
        if (conn) {
            conn->Update(settings);
        }
    }
}

void NetworkSettings::removeConnection(const QString & objectPath)
{
    kDebug();
    Connection * conn = m_connectionMap.take(objectPath);
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
    return QString::fromLatin1("%1/%2").arg(QLatin1String(NM_DBUS_PATH_SETTINGS_CONNECTION)).arg(mNextConnectionId++);
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
        Connection * conn = m_connectionMap.value(newlyActive);
        if (conn) {
            emit connectionActivated(conn->uuid());
        }
    }
}

#include "networksettings.moc"
