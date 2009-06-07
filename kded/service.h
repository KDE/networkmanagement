/*
Copyright 2009 Dario Freddi <drf54321@gmail.com>

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

#ifndef KNM07_KDED_SERVICE_H
#define KNM07_KDED_SERVICE_H

#include <KDEDModule>

#include <QHash>

#include <QtDBus/QDBusObjectPath>

#include <KSharedConfig>


namespace KnmInternals {
    class Connection;
}

namespace Knm {
namespace Externals {
    class Connectable;
    class WirelessConnection;
}
}

namespace Solid {
namespace Control {
    class WirelessNetwork;
    class WirelessNetworkInterfaceEnvironment;
    class NetworkInterface;
}
}

class NetworkManagementService : public KDEDModule {

Q_OBJECT

public:
    NetworkManagementService(QObject * parent, const QVariantList&);
    virtual ~NetworkManagementService();

    QList<QDBusObjectPath> ListConnectables();

    void reparseConfiguration(const QStringList& changedConnections);

private Q_SLOTS:
    void networkInterfaceAdded(const QString  & uni);

    void networkInterfaceRemoved(const QString  & uni);

    void wirelessNetworkAppeared(const QString &);

    void wirelessNetworkDisappeared(const QString &);

Q_SIGNALS:
    void ConnectableAdded(const QDBusObjectPath &path);
    void ConnectableRemoved(const QDBusObjectPath &path);

private:
    void networkInterfaceAdded(Solid::Control::NetworkInterface *iface);
    void networkInterfaceRemoved(Solid::Control::NetworkInterface *iface);
    Knm::Externals::WirelessConnection *processNewWirelessNetwork(const QString &ssid);
    KnmInternals::Connection *restoreConnection(const QString & connectionId);
    KSharedConfig::Ptr connectionFileForUuid(const QString & uuid);

private:
    QHash<Knm::Externals::Connectable*, QDBusObjectPath> m_connectables;
    QHash<QString, KnmInternals::Connection*> m_connections;
    QHash<QString, Solid::Control::WirelessNetworkInterfaceEnvironment*> m_environments;
    int m_counter;
    KSharedConfigPtr m_config;
};

#endif
