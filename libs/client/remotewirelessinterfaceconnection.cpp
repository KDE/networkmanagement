/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2012 Lamarque V. Souza <lamarque@kde.org>

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

#include "remotewirelessinterfaceconnection.h"
#include "remotewirelessinterfaceconnection_p.h"

#include "solid/control/wirelessnetworkinterface.h"

#include <KDebug>

RemoteWirelessInterfaceConnection::RemoteWirelessInterfaceConnection(const QVariantMap &properties, QObject * parent)
: RemoteInterfaceConnection(*new RemoteWirelessInterfaceConnectionPrivate, properties, parent)
{
    Q_D(RemoteWirelessInterfaceConnection);
    d->wirelessInterfaceConnectionIface = new WirelessInterfaceConnectionInterface("org.kde.networkmanagement", properties["path"].toString(), QDBusConnection::sessionBus(), this);
    connect(d->wirelessInterfaceConnectionIface, SIGNAL(wicPropertiesChanged(QVariantMap)), SLOT(wicPropertiesChanged(QVariantMap)));
    wicPropertiesChanged(properties);
}

RemoteWirelessInterfaceConnection::~RemoteWirelessInterfaceConnection()
{

}

void RemoteWirelessInterfaceConnection::wicPropertiesChanged(const QVariantMap &changedProperties)
{
    Q_D(RemoteWirelessInterfaceConnection);
    QStringList propKeys = changedProperties.keys();
    QLatin1String ssidKey("ssid"),
                  signalStrengthKey("signalStrength"),
                  interfaceCapabilitiesKey("interfaceCapabilities"),
                  apCapabilitiesKey("apCapabilities"),
                  wpaFlagsKey("wpaFlags"),
                  rsnFlagsKey("rsnFlags"),
                  operationModeKey("operationMode");
    QVariantMap::const_iterator it = changedProperties.find(ssidKey);
    if (it != changedProperties.end()) {
        d->ssid = it->toString();
        propKeys.removeOne(ssidKey);
    }
    it = changedProperties.find(signalStrengthKey);
    if (it != changedProperties.end()) {
        d->signalStrength = it->toInt();
        emit strengthChanged(d->signalStrength);
        propKeys.removeOne(signalStrengthKey);
    }
    it = changedProperties.find(interfaceCapabilitiesKey);
    if (it != changedProperties.end()) {
        d->interfaceCapabilities = static_cast<Solid::Control::WirelessNetworkInterfaceNm09::Capabilities>(it->toUInt());
        propKeys.removeOne(interfaceCapabilitiesKey);
    }
    it = changedProperties.find(apCapabilitiesKey);
    if (it != changedProperties.end()) {
        d->apCapabilities = static_cast<Solid::Control::AccessPointNm09::Capabilities>(it->toUInt());
        propKeys.removeOne(apCapabilitiesKey);
    }
    it = changedProperties.find(wpaFlagsKey);
    if (it != changedProperties.end()) {
        d->wpaFlags = static_cast<Solid::Control::AccessPointNm09::WpaFlags>(it->toUInt());
        propKeys.removeOne(wpaFlagsKey);
    }
    it = changedProperties.find(rsnFlagsKey);
    if (it != changedProperties.end()) {
        d->rsnFlags = static_cast<Solid::Control::AccessPointNm09::WpaFlags>(it->toUInt());
        propKeys.removeOne(rsnFlagsKey);
    }
    it = changedProperties.find(operationModeKey);
    if (it != changedProperties.end()) {
        d->operationMode = static_cast<Solid::Control::WirelessNetworkInterfaceNm09::OperationMode>(it->toUInt());
        propKeys.removeOne(operationModeKey);
    }
    /*if (propKeys.count()) {
        kDebug() << "Unhandled properties: " << propKeys;
    }*/
    emit changed();
}

QString RemoteWirelessInterfaceConnection::ssid() const
{
    Q_D(const RemoteWirelessInterfaceConnection);
    return d->ssid;
}

int RemoteWirelessInterfaceConnection::strength() const
{
    Q_D(const RemoteWirelessInterfaceConnection);
    return d->signalStrength;
}

Solid::Control::WirelessNetworkInterfaceNm09::Capabilities RemoteWirelessInterfaceConnection::interfaceCapabilities() const
{
    Q_D(const RemoteWirelessInterfaceConnection);
    return d->interfaceCapabilities;
}

Solid::Control::AccessPointNm09::Capabilities RemoteWirelessInterfaceConnection::apCapabilities() const
{
    Q_D(const RemoteWirelessInterfaceConnection);
    return d->apCapabilities;
}

Solid::Control::AccessPointNm09::WpaFlags RemoteWirelessInterfaceConnection::wpaFlags() const
{
    Q_D(const RemoteWirelessInterfaceConnection);
    return d->wpaFlags;
}

Solid::Control::AccessPointNm09::WpaFlags RemoteWirelessInterfaceConnection::rsnFlags() const
{
    Q_D(const RemoteWirelessInterfaceConnection);
    return d->rsnFlags;
}

Solid::Control::WirelessNetworkInterfaceNm09::OperationMode RemoteWirelessInterfaceConnection::operationMode() const
{
    Q_D(const RemoteWirelessInterfaceConnection);
    return d->operationMode;
}

// vim: sw=4 sts=4 et tw=100
