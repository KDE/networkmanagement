/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

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

#include "ifaces/networkmanager.h"
#include "ifaces/networkinterface.h"
#include "ifaces/wirednetworkinterface.h"
#include "ifaces/wirelessnetworkinterface.h"
#include "ifaces/networkmodeminterface.h"
#include "ifaces/networkbtinterface.h"

#include "soliddefs_p.h"
#include "networkmanager_p.h"
#include "networkinterface.h"
#include "wirednetworkinterface.h"
#include "wirelessnetworkinterface.h"

#include "networkmanager.h"

#include <kglobal.h>

#include <kdebug.h>

K_GLOBAL_STATIC(Solid::Control::NetworkManagerNm09Private, globalNetworkManager)

Solid::Control::NetworkManagerNm09Private::NetworkManagerNm09Private() : m_invalidDevice(0)
{
    loadBackend("Network Management",
                "SolidNetworkManager",
                "Solid::Control::Ifaces::NetworkManagerNm09");

    if (managerBackend()!=0) {
        connect(managerBackend(), SIGNAL(networkInterfaceAdded(QString)),
                this, SLOT(_k_networkInterfaceAdded(QString)));
        connect(managerBackend(), SIGNAL(networkInterfaceRemoved(QString)),
                this, SLOT(_k_networkInterfaceRemoved(QString)));
        connect(managerBackend(), SIGNAL(statusChanged(Solid::Networking::Status)),
                this, SIGNAL(statusChanged(Solid::Networking::Status)));
        connect(managerBackend(), SIGNAL(wirelessEnabledChanged(bool)),
                this, SIGNAL(wirelessEnabledChanged(bool)));
        connect(managerBackend(), SIGNAL(wirelessHardwareEnabledChanged(bool)),
                this, SIGNAL(wirelessHardwareEnabledChanged(bool)));
        connect(managerBackend(), SIGNAL(wwanEnabledChanged(bool)),
                this, SIGNAL(wwanEnabledChanged(bool)));
        connect(managerBackend(), SIGNAL(wwanHardwareEnabledChanged(bool)),
                this, SIGNAL(wwanHardwareEnabledChanged(bool)));
        connect(managerBackend(), SIGNAL(networkingEnabledChanged(bool)),
                this, SIGNAL(networkingEnabledChanged(bool)));
        connect(managerBackend(), SIGNAL(activeConnectionsChanged()),
                this, SIGNAL(activeConnectionsChanged()));
    }
}

Solid::Control::NetworkManagerNm09Private::~NetworkManagerNm09Private()
{
    // Delete all the devices, they are now outdated
    typedef QPair<NetworkInterfaceNm09 *, QObject *> NetworkInterfaceIfacePair;

    foreach (const NetworkInterfaceIfacePair &pair, m_networkInterfaceMap) {
        delete pair.first;
        delete pair.second;
    }

    m_networkInterfaceMap.clear();
}

Solid::Control::NetworkInterfaceNm09List Solid::Control::NetworkManagerNm09Private::buildDeviceList(const QStringList &uniList)
{
    NetworkInterfaceNm09List list;
    Ifaces::NetworkManagerNm09 *backend = qobject_cast<Ifaces::NetworkManagerNm09 *>(managerBackend());

    if (backend == 0) return list;

    foreach (const QString &uni, uniList)
    {
        QPair<NetworkInterfaceNm09 *, QObject *> pair = findRegisteredNetworkInterface(uni);

        if (pair.first!= 0)
        {
            list.append(pair.first);
        }
    }

    return list;
}

Solid::Control::NetworkInterfaceNm09List Solid::Control::NetworkManagerNm09Private::networkInterfaces()
{
    Ifaces::NetworkManagerNm09 *backend = qobject_cast<Ifaces::NetworkManagerNm09 *>(managerBackend());

    if (backend!= 0)
    {
        return buildDeviceList(backend->networkInterfaces());
    }
    else
    {
        kWarning() << "NetworkManager backend not found";
        return NetworkInterfaceNm09List();
    }
}

Solid::Control::NetworkInterfaceNm09List Solid::Control::NetworkManagerNm09::networkInterfaces()
{
    return globalNetworkManager->networkInterfaces();
}

bool Solid::Control::NetworkManagerNm09::isNetworkingEnabled()
{
    return_SOLID_CALL(Ifaces::NetworkManagerNm09 *, globalNetworkManager->managerBackend(), false, isNetworkingEnabled());
}

bool Solid::Control::NetworkManagerNm09::isWirelessEnabled()
{
    return_SOLID_CALL(Ifaces::NetworkManagerNm09 *, globalNetworkManager->managerBackend(), false, isWirelessEnabled());
}

bool Solid::Control::NetworkManagerNm09::isWirelessHardwareEnabled()
{
    return_SOLID_CALL(Ifaces::NetworkManagerNm09 *, globalNetworkManager->managerBackend(), false, isWirelessHardwareEnabled());
}

bool Solid::Control::NetworkManagerNm09::isWwanEnabled()
{
    return_SOLID_CALL(Ifaces::NetworkManagerNm09 *, globalNetworkManager->managerBackend(), false, isWwanEnabled());
}

bool Solid::Control::NetworkManagerNm09::isWwanHardwareEnabled()
{
    return_SOLID_CALL(Ifaces::NetworkManagerNm09 *, globalNetworkManager->managerBackend(), false, isWwanHardwareEnabled());
}

void Solid::Control::NetworkManagerNm09::setNetworkingEnabled(bool enabled)
{
    SOLID_CALL(Ifaces::NetworkManagerNm09 *, globalNetworkManager->managerBackend(), setNetworkingEnabled(enabled));
}

void Solid::Control::NetworkManagerNm09::setWirelessEnabled(bool enabled)
{
    SOLID_CALL(Ifaces::NetworkManagerNm09 *, globalNetworkManager->managerBackend(), setWirelessEnabled(enabled));
}

void Solid::Control::NetworkManagerNm09::setWwanEnabled(bool enabled)
{
    SOLID_CALL(Ifaces::NetworkManagerNm09 *, globalNetworkManager->managerBackend(), setWwanEnabled(enabled));
}

QString Solid::Control::NetworkManagerNm09::version()
{
    return_SOLID_CALL(Ifaces::NetworkManagerNm09 *, globalNetworkManager->managerBackend(), QString(), version());
}

int Solid::Control::NetworkManagerNm09::compareVersion(const QString & version)
{
    return_SOLID_CALL(Ifaces::NetworkManagerNm09 *, globalNetworkManager->managerBackend(), -1, compareVersion(version));
}

int Solid::Control::NetworkManagerNm09::compareVersion(const int x, const int y, const int z)
{
    return_SOLID_CALL(Ifaces::NetworkManagerNm09 *, globalNetworkManager->managerBackend(), -1, compareVersion(x, y, z));
}

Solid::Networking::Status Solid::Control::NetworkManagerNm09::status()
{
    return_SOLID_CALL(Ifaces::NetworkManagerNm09 *, globalNetworkManager->managerBackend(), Solid::Networking::Unknown, status());
}

Solid::Control::NetworkInterfaceNm09::Types Solid::Control::NetworkManagerNm09::supportedInterfaceTypes()
{
    return_SOLID_CALL(Ifaces::NetworkManagerNm09 *, globalNetworkManager->managerBackend(),
                      Solid::Control::NetworkInterfaceNm09::Types(), supportedInterfaceTypes());
}

Solid::Control::NetworkInterfaceNm09 * Solid::Control::NetworkManagerNm09Private::findNetworkInterface(const QString &uni)
{
    Ifaces::NetworkManagerNm09 *backend = qobject_cast<Ifaces::NetworkManagerNm09 *>(managerBackend());

    if (backend == 0) return 0;

    if (!backend->networkInterfaces().contains(uni)) {
        return 0;
    }

    QPair<NetworkInterfaceNm09 *, QObject *> pair = findRegisteredNetworkInterface(uni);

    if (pair.first != 0)
    {
        return pair.first;
    }
    else
    {
        return 0;
    }
}

Solid::Control::NetworkInterfaceNm09 * Solid::Control::NetworkManagerNm09::findNetworkInterface(const QString &uni)
{
    return globalNetworkManager->findNetworkInterface(uni);
}

Solid::Control::NetworkManagerNm09::Notifier * Solid::Control::NetworkManagerNm09::notifier()
{
    return globalNetworkManager;
}

void Solid::Control::NetworkManagerNm09Private::_k_networkInterfaceAdded(const QString &uni)
{
    QPair<NetworkInterfaceNm09 *, QObject*> pair = m_networkInterfaceMap.take(uni);

    if (pair.first!= 0)
    {
        // Oops, I'm not sure it should happen...
        // But well in this case we'd better kill the old device we got, it's probably outdated

        delete pair.first;
        delete pair.second;
    }

    emit networkInterfaceAdded(uni);
}

void Solid::Control::NetworkManagerNm09Private::_k_networkInterfaceRemoved(const QString &uni)
{
    emit networkInterfaceRemoved(uni);

    QPair<NetworkInterfaceNm09 *, QObject *> pair = m_networkInterfaceMap.take(uni);

    if (pair.first!= 0)
    {
        delete pair.first;
        delete pair.second;
    }
}

void Solid::Control::NetworkManagerNm09Private::_k_destroyed(QObject *object)
{
    Ifaces::NetworkInterfaceNm09 *device = qobject_cast<Ifaces::NetworkInterfaceNm09 *>(object);

    if (device!=0)
    {
        QString uni = device->uni();
        QPair<NetworkInterfaceNm09 *, QObject *> pair = m_networkInterfaceMap.take(uni);
        delete pair.first;
    }
}

/***************************************************************************/

QPair<Solid::Control::NetworkInterfaceNm09 *, QObject *>
Solid::Control::NetworkManagerNm09Private::findRegisteredNetworkInterface(const QString &uni)
{
    if (m_networkInterfaceMap.contains(uni)) {
        return m_networkInterfaceMap[uni];
    } else {
        Ifaces::NetworkManagerNm09 *backend = qobject_cast<Ifaces::NetworkManagerNm09 *>(managerBackend());

        if (backend!=0)
        {
            QObject * iface = backend->createNetworkInterface(uni);
            NetworkInterfaceNm09 *device = 0;
            if (qobject_cast<Ifaces::WirelessNetworkInterfaceNm09 *>(iface) != 0) {
                device = new WirelessNetworkInterfaceNm09(iface);
            } else if (qobject_cast<Ifaces::WiredNetworkInterfaceNm09 *>(iface) != 0) {
                device = new WiredNetworkInterfaceNm09(iface);
            } else if (qobject_cast<Ifaces::BtNetworkInterfaceNm09 *>(iface) != 0) {
                device = new BtNetworkInterfaceNm09(iface);
            } else if (qobject_cast<Ifaces::ModemNetworkInterfaceNm09 *>(iface) != 0) {
                device = new ModemNetworkInterfaceNm09(iface);
            /* TODO: add WimaxNetworkInterfaceNm09 and OlpcMeshNetworkInterfaceNm09 */
            } else {
                kDebug() << "Unhandled network interface: " << uni;
            }
            if (device != 0) {
                QPair<NetworkInterfaceNm09 *, QObject *> pair(device, iface);
                connect(iface, SIGNAL(destroyed(QObject*)),
                        this, SLOT(_k_destroyed(QObject*)));
                m_networkInterfaceMap[uni] = pair;
                return pair;
            }
            else
            {
                return QPair<NetworkInterfaceNm09 *, QObject *>(0, 0);
            }
        }
        else
        {
            return QPair<NetworkInterfaceNm09 *, QObject *>(0, 0);
        }
    }
}

void Solid::Control::NetworkManagerNm09::activateConnection(const QString & interfaceUni, const QString & connectionUni,
                const QVariantMap & connectionParameters )
{
    SOLID_CALL(Ifaces::NetworkManagerNm09 *, globalNetworkManager->managerBackend(), activateConnection(interfaceUni, connectionUni, connectionParameters));
}

void Solid::Control::NetworkManagerNm09::deactivateConnection(const QString & activeConnectionUni)
{
    SOLID_CALL(Ifaces::NetworkManagerNm09 *, globalNetworkManager->managerBackend(), deactivateConnection(activeConnectionUni));
}

QStringList Solid::Control::NetworkManagerNm09::activeConnections()
{
    return_SOLID_CALL(Ifaces::NetworkManagerNm09 *, globalNetworkManager->managerBackend(), QStringList(), activeConnections());
}

QStringList Solid::Control::NetworkManagerNm09::activeConnectionsUuid()
{
    return_SOLID_CALL(Ifaces::NetworkManagerNm09 *, globalNetworkManager->managerBackend(), QStringList(), activeConnectionsUuid());
}

#include "networkmanager_p.moc"
#include "networkmanager.moc"
