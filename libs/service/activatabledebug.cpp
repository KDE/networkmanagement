/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#include "activatabledebug.h"

#include <KDebug>
#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>

#include <activatable.h>
#include <interfaceconnection.h>
#include <vpninterfaceconnection.h>
#include <hiddenwirelessinterfaceconnection.h>
#include <wirelessinterfaceconnection.h>
#include <wirelessnetwork.h>
#include <gsminterfaceconnection.h>

ActivatableDebug::ActivatableDebug()
{
}

ActivatableDebug::~ActivatableDebug()
{
}

QString ActivatableDebug::activatableToString(Knm::Activatable* activatable)
{
    Knm::InterfaceConnection * ic;
    Knm::WirelessInterfaceConnection * wic;
    Knm::WirelessNetwork * wni;
    Knm::VpnInterfaceConnection * vpn;
    Knm::GsmInterfaceConnection * gsm;

    QString string;

    QString identifier;
    Solid::Control::NetworkInterfaceNm09 * iface = Solid::Control::NetworkManagerNm09::findNetworkInterface(activatable->deviceUni());
    if (iface) {
        identifier = iface->interfaceName();
    } else {
        identifier = activatable->deviceUni();
    }

    switch (activatable->activatableType()) {
        case Knm::Activatable::InterfaceConnection:
            ic = qobject_cast<Knm::InterfaceConnection*>(activatable);
            string = QString::fromLatin1("InterfaceConnection %1 (%2) on %3 with state %4").arg(ic->connectionName(), ic->connectionUuid(), identifier, QString::number((uint)ic->activationState()));
            break;
        case Knm::Activatable::WirelessInterfaceConnection:
            wic = qobject_cast<Knm::WirelessInterfaceConnection*>(activatable);
            string = QString::fromLatin1("WirelessConnection %1 (%2) on %3 with state %4 for network %5 with strength %6").arg(wic->connectionName(), wic->connectionUuid(), identifier, QString::number(wic->activationState()), wic->ssid(), QString::number(wic->strength()));
            break;
        case Knm::Activatable::HiddenWirelessInterfaceConnection:
            wic = qobject_cast<Knm::HiddenWirelessInterfaceConnection*>(activatable);
            string = QString::fromLatin1("HiddenWirelessConnection %1 (%2) on %3 with state %4 for network %5 with strength %6").arg(wic->connectionName(), wic->connectionUuid(), identifier, QString::number(wic->activationState()), wic->ssid(), QString::number(wic->strength()));
            break;
        case Knm::Activatable::WirelessNetwork:
            wni = qobject_cast<Knm::WirelessNetwork*>(activatable);
            string = QString::fromLatin1("WirelessNetwork for network %1 on %2 with strength %3").arg(wni->ssid(), identifier, QString::number(wni->strength()));
            break;
        case Knm::Activatable::UnconfiguredInterface:
            string = QString::fromLatin1("UnconfiguredDevice %1").arg(identifier);
            break;
        case Knm::Activatable::VpnInterfaceConnection:
            vpn = qobject_cast<Knm::VpnInterfaceConnection*>(activatable);
            string = QString::fromLatin1("VpnInterfaceConnection %1 (%2) on %3").arg(vpn->connectionName(), vpn->connectionUuid(), identifier);
            break;
        case Knm::Activatable::GsmInterfaceConnection:
            gsm = qobject_cast<Knm::GsmInterfaceConnection*>(activatable);
            string = QString::fromLatin1("GsmInterfaceConnection %1 (%2) on %3 with state %4 with signal quality %5").arg(gsm->connectionName(), gsm->connectionUuid(), identifier, QString::number(gsm->activationState()), QString::number(gsm->getSignalQuality()));
            break;
    }

    return string;
}

void ActivatableDebug::handleAdd(Knm::Activatable * activatable) {
    QString debugString = activatableToString(activatable);
    debugString += " was added";
    kDebug() << "(" << objectName() << ")" << debugString;
}

void ActivatableDebug::handleUpdate(Knm::Activatable * activatable)
{
    QString debugString = activatableToString(activatable);
    debugString += " changed";
    kDebug() << "(" << objectName() << ")" << debugString;
}

void ActivatableDebug::handleRemove(Knm::Activatable * activatable)
{
    QString debugString = activatableToString(activatable);
    debugString += " was removed";
    kDebug() << "(" << objectName() << ")" << debugString;
}

// vim: sw=4 sts=4 et tw=100
