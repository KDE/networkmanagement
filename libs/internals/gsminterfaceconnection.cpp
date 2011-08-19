/*
Copyright 2008 Frederik Gladhorn <gladhorn@kde.org>
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2010 Lamarque Souza <lamarque@gmail.com>

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

#ifdef COMPILE_MODEM_MANAGER_SUPPORT

#include <KDebug>

#include <solid/control/networkmanager.h>
#include <solid/control/modemmanager.h>
#include <solid/control/modemgsmnetworkinterface.h>

#include "gsminterfaceconnection.h"

using namespace Knm;
using namespace Solid::Control;

GsmInterfaceConnection::GsmInterfaceConnection(ActivatableType type, const QString & deviceUni, QObject * parent)
: InterfaceConnection(type, deviceUni, parent), m_signalQuality(0), m_enabled(false)
{
    m_accessTechnology = Solid::Control::ModemInterface::UnknownTechnology;
    connectMMSignals();

    // For bluetooth devices.
    connect(Solid::Control::ModemManager::notifier(), SIGNAL(modemInterfaceAdded(QString)), this, SLOT(connectMMSignals()));
}

GsmInterfaceConnection::~GsmInterfaceConnection()
{
}

void GsmInterfaceConnection::connectMMSignals()
{
    kDebug();
    GsmNetworkInterface *interface = qobject_cast<GsmNetworkInterface *>(NetworkManager::findNetworkInterface(deviceUni()));
    if (!interface) {
        return;
    }

    // To force loading ModemManager backend
    if (ModemManager::notifier()) {
        kDebug() << "Loading ModemManager backend";
    }

    ModemGsmNetworkInterface * modemNetworkIface = interface->getModemNetworkIface();

    if (modemNetworkIface) {
        kDebug() << "Connecting signals of " << modemNetworkIface->udi() << " to " << deviceUni();
        QObject::connect(modemNetworkIface, SIGNAL(signalQualityChanged(uint)), this, SLOT(setSignalQuality(uint)));
        QObject::connect(modemNetworkIface, SIGNAL(accessTechnologyChanged(Solid::Control::ModemInterface::AccessTechnology)), this, SLOT(setAccessTechnology(Solid::Control::ModemInterface::AccessTechnology)));
        QObject::connect(modemNetworkIface, SIGNAL(enabledChanged(bool)), this, SLOT(setEnabled(bool)));

        m_signalQuality = modemNetworkIface->getSignalQuality();
        m_accessTechnology = modemNetworkIface->getAccessTechnology();
        m_enabled = modemNetworkIface->enabled();
    }
}

void GsmInterfaceConnection::setSignalQuality(uint signalQuality)
{
    if ((int)signalQuality != m_signalQuality) {
        m_signalQuality = (int)signalQuality;
        emit signalQualityChanged(m_signalQuality);
    }
}

void GsmInterfaceConnection::setAccessTechnology(const Solid::Control::ModemInterface::AccessTechnology accessTechnology)
{
    int temp =accessTechnology;

    if (temp != m_accessTechnology) {
        setSignalQuality(0);
        m_accessTechnology = temp;
        emit accessTechnologyChanged(m_accessTechnology);
    }
}

int GsmInterfaceConnection::getSignalQuality() const
{
    return m_signalQuality;
}

int GsmInterfaceConnection::getAccessTechnology() const
{
    return m_accessTechnology;
}

void GsmInterfaceConnection::setEnabled(const bool enabled)
{
    m_enabled = enabled;
    emit enabledChanged(m_enabled);

    if (!enabled) {
        setSignalQuality(0);
    }
}

#endif
