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

#include "activatable.h"
#include "interfaceconnection.h"
#include "wirelessinterfaceconnection.h"
#include "wirelessnetworkitem.h"

ActivatableDebug::ActivatableDebug(QObject * parent)
: ActivatableObserver(parent)
{
}

ActivatableDebug::~ActivatableDebug()
{
}

QString ActivatableDebug::activatableToString(Knm::Activatable* activatable)
{
    Knm::InterfaceConnection * ic;
    Knm::WirelessInterfaceConnection * wic;
    Knm::WirelessNetworkItem * wni;

    QString string;
    switch (activatable->activatableType()) {
        case Knm::Activatable::Connection:
            ic = qobject_cast<Knm::InterfaceConnection*>(activatable);
            string = QString::fromLatin1("InterfaceConnection %1 (%2) on %3 ").arg(ic->connectionName(), ic->connectionUuid(),  activatable->deviceUni());
            break;
        case Knm::Activatable::WirelessConnection:
            wic = qobject_cast<Knm::WirelessInterfaceConnection*>(activatable);
            string = QString::fromLatin1("WirelessConnection %1 (%2) for network %3 on %4 with strength %5").arg(wic->connectionName(), wic->connectionUuid(), wic->ssid(), activatable->deviceUni(), QString::number(wic->strength()));
            break;
        case Knm::Activatable::WirelessNetworkItem:
            wni = qobject_cast<Knm::WirelessNetworkItem*>(activatable);
            string = QString::fromLatin1("WirelessNetworkItem for network %1 on %2 with strength %3").arg(wni->ssid(), activatable->deviceUni(), QString::number(wni->strength()));
            break;
    }
    return string;
}

void ActivatableDebug::handleAdd(Knm::Activatable * activatable) {
    QString debugString = activatableToString(activatable);
    debugString += " was added";
    kDebug() << debugString;
}

void ActivatableDebug::handleUpdate(Knm::Activatable * activatable)
{
    QString debugString = activatableToString(activatable);
    debugString += " changed";
    kDebug() << debugString;
}

void ActivatableDebug::handleRemove(Knm::Activatable * activatable)
{
    QString debugString = activatableToString(activatable);
    debugString += " was removed";
    kDebug() << debugString;
}

// vim: sw=4 sts=4 et tw=100
