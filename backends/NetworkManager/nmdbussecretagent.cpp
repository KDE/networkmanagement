/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.net>
Copyright 2011-2012 Lamarque V. Souza <lamarque@kde.org>

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

class NMDBusSecretAgentPrivate
{
public:
    SecretsProvider *secretsProvider;
    SecretAgentAdaptor *agent;
    OrgFreedesktopNetworkManagerAgentManagerInterface *agentManager;
    QDBusServiceWatcher *watcher;

    QHash <QString,QPair<QString, QDBusMessage> > connectionsToRead;
    QList <QString> objectPaths;
};

NMDBusSecretAgent::NMDBusSecretAgent(QObject * parent)
: QObject(parent), QDBusContext(), d_ptr(new NMDBusSecretAgentPrivate)
{
    Q_D(NMDBusSecretAgent);
    d->secretsProvider = 0;
    d->agent = new SecretAgentAdaptor(this);
    d->agentManager = new OrgFreedesktopNetworkManagerAgentManagerInterface(NM_DBUS_SERVICE, NM_DBUS_PATH_AGENT_MANAGER, QDBusConnection::systemBus(),this);
    d->watcher = new QDBusServiceWatcher(NM_DBUS_SERVICE, QDBusConnection::systemBus(), QDBusServiceWatcher::WatchForRegistration, this);
    connect(d->watcher, SIGNAL(serviceRegistered(QString)), SLOT(registerAgent()));
    registerAgent();
}

NMDBusSecretAgent::~NMDBusSecretAgent()
{
    Q_D(NMDBusSecretAgent);
    d->agentManager->Unregister();
    delete d->agent;
    delete d->agentManager;
    delete d->watcher;
    delete d;
}

void NMDBusSecretAgent::registerAgent()
{
    Q_D(NMDBusSecretAgent);
    d->agentManager->connection().registerObject(NM_DBUS_PATH_SECRET_AGENT, d->agent, QDBusConnection::ExportAllSlots);
    d->agentManager->Register("org.kde.networkmanagement");
    kDebug() << "Agent registered";
}

QVariantMapMap NMDBusSecretAgent::GetSecrets(const QVariantMapMap &connection, const QDBusObjectPath &connection_path, const QString &setting_name, const QStringList &hints, uint flags)
{
    Q_D(NMDBusSecretAgent);
    // WARNING: do not commit this uncommented.
    //kDebug() << connection;
    kDebug() << setting_name << flags << hints;

    QDBusMessage msg = message();
    msg.setDelayedReply(true);
    Knm::Connection * con = new Knm::Connection(QUuid(), Knm::Connection::Wired);
    ConnectionDbus condbus(con);
    condbus.fromDbusMap(connection);
    condbus.fromDbusSecretsMap(connection);
    QPair<QString,QDBusMessage> pair;
    pair.first = connection_path.path();
    pair.second = msg;

    // remove stale request to prevent crashes.
    // https://bugs.kde.org/show_bug.cgi?id=283105
    if (d->connectionsToRead.contains(con->uuid() + setting_name)) {
        d->connectionsToRead.remove(con->uuid() + setting_name);
    }
    d->connectionsToRead.insert(con->uuid() + setting_name, pair);

    if (!d->objectPaths.contains(connection_path.path() + setting_name)) {
        d->objectPaths.append(connection_path.path() + setting_name);
    }

    if (d->secretsProvider) {
        foreach (Knm::Setting * setting, con->settings()) {
            if (setting->name() == setting_name) {
                d->secretsProvider->loadSecrets(con, setting_name, (SecretsProvider::GetSecretsFlags)flags);
                break;
            }
        }
    }

    return connection;
}

void NMDBusSecretAgent::SaveSecrets(const QVariantMapMap &connection, const QDBusObjectPath &connection_path)
{
    Q_UNUSED(connection_path)
    Q_D(NMDBusSecretAgent);
    Knm::Connection * con = new Knm::Connection(QUuid(), Knm::Connection::Wired);
    ConnectionDbus condbus(con);
    condbus.fromDbusMap(connection);
    if (d->secretsProvider) {
        ConnectionDbus condbus(con);
        QVariantMapMap secrets = condbus.toDbusSecretsMap();

        kDebug()<< "Secrets are being saved for connection " << con->uuid();
        d->secretsProvider->saveSecrets(con);
    } else {
        kDebug()<< "Secrets for" << con->uuid() << "not save because there is no secretsProvider registered.";
    }
}

void NMDBusSecretAgent::DeleteSecrets(const QVariantMapMap &connection, const QDBusObjectPath &connection_path)
{
    Q_UNUSED(connection_path)
    Q_D(NMDBusSecretAgent);
    Knm::Connection * con = new Knm::Connection(QUuid(), Knm::Connection::Wired);
    ConnectionDbus condbus(con);
    condbus.fromDbusMap(connection);
    if (d->secretsProvider) {
        kDebug() << "Deleting secrets for connection " << con->uuid();
        d->secretsProvider->deleteSecrets(con);
    } else {
        kDebug()<< "Secrets for" << con->uuid() << "not deleted because there is no d->secretsProvider registered.";
    }
}

void NMDBusSecretAgent::deleteSavedConnection(Knm::Connection *con)
{
    delete con;
}


void NMDBusSecretAgent::secretsReady(Knm::Connection *con, const QString &name, bool failed, bool needsSaving)
{
    Q_D(NMDBusSecretAgent);
    kDebug();
    QPair<QString, QDBusMessage> pair = d->connectionsToRead.take(con->uuid() + name);
    if (d->objectPaths.removeOne(pair.first + name)) {
        QDBusMessage reply;
        if (failed) {
            reply = pair.second.createErrorReply(QDBusError::Failed, QString());
        } else {
            ConnectionDbus condbus(con);
            QVariantMapMap secrets = condbus.toDbusSecretsMap(name);
            reply = pair.second.createReply();
            QVariant arg = QVariant::fromValue(secrets);
            reply << arg;
        }
        QDBusConnection::systemBus().send(reply);
        if (needsSaving) {
            d->secretsProvider->saveSecrets(con);
        } else {
            delete con;
        }
    } else {
        kWarning() << "Unexpected secrets arrived for" << pair.first << name;
    }
}

void NMDBusSecretAgent::CancelGetSecrets(const QDBusObjectPath &connection_path, const QString &setting_name)
{
    Q_D(NMDBusSecretAgent);
    d->objectPaths.removeOne(connection_path.path() + setting_name);
}

void NMDBusSecretAgent::registerSecretsProvider(SecretsProvider * provider)
{
    Q_D(NMDBusSecretAgent);
    d->secretsProvider = provider;
    connect(d->secretsProvider,SIGNAL(connectionRead(Knm::Connection*,QString,bool,bool)),SLOT(secretsReady(Knm::Connection*,QString,bool,bool)));
    connect(d->secretsProvider,SIGNAL(connectionSaved(Knm::Connection*)),SLOT(deleteSavedConnection(Knm::Connection*)));
}
