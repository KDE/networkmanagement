/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.net>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "nmdbussecretagent.h"

#include <NetworkManager.h>

#include <QHash>
#include <QUuid>

#include <QDBusArgument>
#include <QDBusConnection>

#include <KDebug>

#include <connection.h>
#include <interfaceconnection.h>
#include <vpninterfaceconnection.h>

#include "secretsprovider.h"

#include "connectiondbus.h"

#include "nm-secret-agentadaptor.h"

NMDBusSecretAgent::NMDBusSecretAgent(QObject * parent)
: QObject(parent), QDBusContext(), m_secretsProvider(0)
{
    m_agent = new SecretAgentAdaptor(this);
    m_agentManager = new OrgFreedesktopNetworkManagerAgentManagerInterface(NM_DBUS_SERVICE, NM_DBUS_PATH_AGENT_MANAGER, QDBusConnection::systemBus(),this);
    m_watcher = new QDBusServiceWatcher(NM_DBUS_SERVICE, QDBusConnection::systemBus(), QDBusServiceWatcher::WatchForRegistration, this);
    connect(m_watcher, SIGNAL(serviceRegistered(const QString &)), SLOT(registerAgent()));
    registerAgent();
}

NMDBusSecretAgent::~NMDBusSecretAgent()
{
    m_agentManager->Unregister();
    delete m_agent;
    delete m_agentManager;
    delete m_watcher;
}

void NMDBusSecretAgent::registerAgent()
{
    m_agentManager->connection().registerObject(NM_DBUS_PATH_SECRET_AGENT, m_agent, QDBusConnection::ExportAllSlots);
    m_agentManager->Register("org.kde.networkmanagement");
    kDebug() << "Agent registered";
}

QVariantMapMap NMDBusSecretAgent::GetSecrets(const QVariantMapMap &connection, const QDBusObjectPath &connection_path, const QString &setting_name, const QStringList &hints, uint flags)
{
    kDebug() << connection;
    kDebug() << setting_name << flags << hints;
    QVariantMapMap map;

    QDBusMessage msg = message();
    msg.setDelayedReply(true);
    Knm::Connection * con = new Knm::Connection(QUuid(), Knm::Connection::Wired);
    ConnectionDbus * condbus = new ConnectionDbus(con);
    condbus->fromDbusMap(connection);
    QPair<QString,QDBusMessage> pair;
    pair.first = setting_name;
    pair.second = msg;
    m_connectionsToRead.insert(con->uuid(),pair);

    foreach (Knm::Setting * setting, con->settings()) {
        if (setting->name() == setting_name && m_secretsProvider) {
            m_secretsProvider->loadSecrets(con, setting_name, (SecretsProvider::GetSecretsFlags)flags);
            break;
        }
    }
    delete condbus;
    return map;
}

void NMDBusSecretAgent::SaveSecrets(const QVariantMapMap &connection, const QDBusObjectPath &connection_path)
{
    Knm::Connection * con = new Knm::Connection(QUuid(), Knm::Connection::Wired);
    ConnectionDbus * condbus = new ConnectionDbus(con);
    condbus->fromDbusMap(connection);
    kDebug()<<"Secrets are being saved for connection " << con->uuid();
    m_secretsProvider->saveSecrets(con);
    delete condbus;
}

void NMDBusSecretAgent::DeleteSecrets(const QVariantMapMap &connection, const QDBusObjectPath &connection_path)
{
    Knm::Connection * con = new Knm::Connection(QUuid(), Knm::Connection::Wired);
    ConnectionDbus * condbus = new ConnectionDbus(con);
    condbus->fromDbusMap(connection);
    kDebug() << "Deleting secrets for connection " << con->uuid();
    m_secretsProvider->deleteSecrets(con);
    delete condbus;
}

void NMDBusSecretAgent::deleteSavedConnection(Knm::Connection *con)
{
    delete con;
}

void NMDBusSecretAgent::secretsReady(Knm::Connection *con, const QString &name)
{
    QMultiHash<QString, QPair<QString,QDBusMessage> >::iterator i = m_connectionsToRead.find(con->uuid());
    while (i != m_connectionsToRead.end() && i.key() == con->uuid()) {
        if (i.value().first == name) {
            QPair<QString,QDBusMessage> pair = i.value();
            ConnectionDbus * condbus = new ConnectionDbus(con);
            QVariantMapMap secrets = condbus->toDbusSecretsMap(pair.first);

            QDBusMessage reply = pair.second.createReply();
            QVariant arg = QVariant::fromValue(secrets);
            reply << arg;
            QDBusConnection::systemBus().send(reply);

            m_connectionsToRead.erase(i);
            delete condbus;
            break;
        }
    }
    delete con;
}

void NMDBusSecretAgent::CancelGetSecrets(const QDBusObjectPath &connection_path, const QString &setting_name)
{
}

void NMDBusSecretAgent::registerSecretsProvider(SecretsProvider * provider)
{
    m_secretsProvider = provider;
    connect(m_secretsProvider,SIGNAL(connectionSaved(Knm::Connection *)),SLOT(deleteSavedConnection(Knm::Connection *)));
    connect(m_secretsProvider,SIGNAL(connectionRead(Knm::Connection *, const QString&)),SLOT(secretsReady(Knm::Connection*, const QString&)));
}
