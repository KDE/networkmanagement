/*
Copyright 2009 Paul Marchouk <pmarchouk@gmail.com>
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

#include <solid/control/modemmanager.h>
#include <solid/control/modemgsmnetworkinterface.h>

#include "gsminterfaceconnectionhelpers.h"
#include "gsminterfaceconnectionhelpers_p.h"

#include <memory>

#include "connection.h"
#include "settings/gsm.h"
#include "gsminterfaceconnection.h"

using namespace Knm;

Knm::GsmInterfaceConnection* GsmInterfaceConnectionHelpers::buildGsmInterfaceConnection(
        Solid::Control::GsmNetworkInterface *interface,
        Knm::Connection *connection, const QString & deviceUni, QObject * parent)
{
    GsmInterfaceConnectionBuilder builder(interface, connection, deviceUni, parent);

    return builder.build();
}

void GsmInterfaceConnectionHelpers::syncGsmInterfaceConnection(GsmInterfaceConnection *ic, Knm::Connection *c)
{
    GsmInterfaceConnectionSync sync;

    sync.sync(ic, c);
}

GsmInterfaceConnectionBuilder::GsmInterfaceConnectionBuilder(
                    Solid::Control::GsmNetworkInterface *interface,
                    Knm::Connection *connection,
                    const QString &deviceUni,
                    QObject *parent)
: InterfaceConnectionBuilder(connection, deviceUni, parent), m_interface(interface)
{
}

GsmInterfaceConnectionBuilder::~GsmInterfaceConnectionBuilder()
{
}

Knm::GsmInterfaceConnection* GsmInterfaceConnectionBuilder::build()
{
    GsmInterfaceConnection *ic =
            new GsmInterfaceConnection(Activatable::GsmInterfaceConnection,
                                            m_deviceUni, m_parent);

    if (ic) {
        init(ic);
    }

    return ic;
}

void GsmInterfaceConnectionBuilder::init(GsmInterfaceConnection *ic)
{
    InterfaceConnectionBuilder::init(ic);

    // To force loading ModemManager backend
    if (Solid::Control::ModemManager::notifier()) {
        kDebug() << "Loading ModemManager backend";
    }

    Solid::Control::ModemGsmNetworkInterface * modemNetworkIface = m_interface->getModemNetworkIface();

    if (modemNetworkIface) {
        ic->m_signalQuality = modemNetworkIface->getSignalQuality();
        ic->m_accessTechnology = Solid::Control::ModemInterface::convertAccessTechnologyToString(modemNetworkIface->getAccessTechnology());
    } else {
        ic->m_signalQuality = 0;
        ic->m_accessTechnology = Solid::Control::ModemInterface::convertAccessTechnologyToString(Solid::Control::ModemInterface::UnknownTechnology);
    }
}

GsmInterfaceConnectionSync::GsmInterfaceConnectionSync()
{
}

GsmInterfaceConnectionSync::~GsmInterfaceConnectionSync()
{
}

void GsmInterfaceConnectionSync::sync(Knm::GsmInterfaceConnection * interfaceConnection,
                                           Knm::Connection *connection)
{
    InterfaceConnectionSync::sync(interfaceConnection, connection);
}

#endif
