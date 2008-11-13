/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "connectionlist.h"

#include <NetworkManager.h>
#include <KDebug>

#include <QVBoxLayout>
#include "connectionitem.h"
#include "networkmanagersettings.h"
#include "remoteconnection.h"

ConnectionList::ConnectionList(NetworkManagerSettings * userSettings, NetworkManagerSettings * systemSettings, QWidget * parent)
: QWidget(parent), m_userSettings(userSettings), m_systemSettings(systemSettings)
{
    m_layout = new QVBoxLayout(this);
    setLayout(m_layout);
}

ConnectionList::~ConnectionList()
{
}

void ConnectionList::init()
{
    // adds items from subclasses above our layout
    setupHeader();
    m_connectionLayout = new QVBoxLayout(0);
    m_layout->addLayout(m_connectionLayout);
    // create a connectionItem for each appropriate connection
    addSettingsService(m_userSettings);
    addSettingsService(m_systemSettings);
    // adds items from subclasses below our layout
    setupFooter();
    m_layout->addStretch(5);
}

void ConnectionList::addSettingsService(NetworkManagerSettings * service)
{
    connect(service, SIGNAL(connectionAdded(NetworkManagerSettings *, const QString&)), SLOT(connectionAddedToService(NetworkManagerSettings *, const QString&)));
    connect(service, SIGNAL(connectionRemoved(NetworkManagerSettings *, const QString&)), SLOT(connectionRemovedFromService(NetworkManagerSettings *, const QString&)));
    //connect(service, SIGNAL(connectionUpdated(NetworkManagerSettings *, const QString&);
    connect(service, SIGNAL(appeared(NetworkManagerSettings*)), SLOT(assessConnections(NetworkManagerSettings*)));
    connect(service, SIGNAL(disappeared(NetworkManagerSettings*)), SLOT(serviceDisappeared(NetworkManagerSettings*)));

    assessConnections(service);
}


void ConnectionList::assessConnections(NetworkManagerSettings * service)
{
    if (service->isValid()) {
        kDebug() << service->objectName() << "has connections" << service->connections();
        foreach (QString connectionPath, service->connections() ) {
            processConnection(service, connectionPath);
        }
    }
}

void ConnectionList::serviceDisappeared(NetworkManagerSettings* settings)
{
    //remove all connections from this service
    ServiceConnectionHash::iterator i = m_connections.begin();
    while (i != m_connections.end()) {
        if (i.key().first == settings->service()) {
            ConnectionItem * item = i.value();
            m_connectionLayout->removeWidget(item);
            i = m_connections.erase(i);
            delete item;
        } else {
            ++i;
        }
    }
}

void ConnectionList::processConnection(NetworkManagerSettings * service, const QString& connectionPath)
{
    QPair<QString,QString> key(service->service(), connectionPath);
    if (m_connections.contains(key)) {
        ConnectionItem * connection = m_connections.value(key);

        if (!accept(connection->connection())) {
            m_connectionLayout->removeWidget(connection);
            m_connections.remove(key);
            delete connection;
            m_connectionLayout->invalidate();
            m_layout->invalidate();
        }
    } else {
        RemoteConnection * remoteConnection = service->findConnection(connectionPath);
        if (accept(remoteConnection)) {
            kDebug() << "adding connection" << connectionPath << "from" << service->objectName();
            ConnectionItem * ci = createItem(remoteConnection);
            connect(ci, SIGNAL(clicked(AbstractConnectableItem*)), SLOT(activateConnection(AbstractConnectableItem*)));
            m_connections.insert(key, ci);
            m_connectionLayout->addWidget(ci);
            m_connectionLayout->invalidate();
            m_layout->invalidate();
        }
    }
}

void ConnectionList::connectionAddedToService(NetworkManagerSettings * service, const QString& connectionPath)
{
    processConnection(service, connectionPath);
}

void ConnectionList::connectionRemovedFromService(NetworkManagerSettings * service, const QString& connectionPath)
{
    // look up the ConnectionItem and remove it
    QPair<QString,QString> key(service->service(), connectionPath);
    if (m_connections.contains(key)) {
        ConnectionItem * item = m_connections.value(key);
        m_connectionLayout->removeWidget(item);
        m_connections.remove(key);
        delete item;
    }
}

ConnectionItem * ConnectionList::createItem(RemoteConnection* connection)
{
    ConnectionItem * ci;
    ci = new ConnectionItem(connection, this);
    ci->setupItem();
    return ci;
}

void ConnectionList::reassess()
{
    // this will try and add all connections on both services
    // duplicates are rejected
    assessConnections(m_userSettings);
    assessConnections(m_systemSettings);
}

// vim: sw=4 sts=4 et tw=100

