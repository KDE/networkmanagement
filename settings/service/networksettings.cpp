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

#include <NetworkManager.h>
#include <nm-setting-connection.h>

#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QDBusMetaType>
#include <KDebug>
#include <KDebug>

#include "connection.h"

//#include "networksettingsadaptor.h"
//#include "marshallarguments.h"


NetworkSettings::NetworkSettings(QObject * parent)
: QObject(parent), mNextConnectionId(0)
{
    //declare types
    qDBusRegisterMetaType<QList<QDBusObjectPath> >();

    //new NetworkSettingsAdaptor(this);

    QDBusConnection dbus = QDBusConnection::systemBus();
    dbus.registerObject(QLatin1String(NM_DBUS_PATH_SETTINGS), this, QDBusConnection::ExportScriptableContents);
}

NetworkSettings::~NetworkSettings()
{
}

/*
bool NetworkSettings::loadSettings(const KConfigGroup &settings)
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
*/

QString NetworkSettings::addConnection(const QVariantMapMap& settings)
{
    kDebug();
    QVariantMapMap::const_iterator it = settings.find(QLatin1String(NM_SETTING_CONNECTION_SETTING_NAME));
    if (it != settings.end()) {
        QVariantMap connectionSettings = it.value();
        QVariantMap::const_iterator connectionSettingsIt = connectionSettings.find(QLatin1String(NM_SETTING_CONNECTION_ID));
        if (connectionSettingsIt != connectionSettings.end()) {
            Connection * connection = new Connection(connectionSettingsIt.value().toString(), settings, this);
            QString objectPath = nextObjectPath();
            m_connectionMap.insert(objectPath, connection);
            QDBusConnection::systemBus().registerObject(objectPath, connection, QDBusConnection::ExportScriptableContents);
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
    kDebug();
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
    //connectionMap.take(id);
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

#include "networksettings.moc"
