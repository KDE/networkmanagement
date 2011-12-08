/*
Copyright 2009 Paul Marchouk <pmarchouk@gmail.com>
Copyright 2010-2011 Lamarque Souza <lamarque@kde.org>

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

#ifndef KNM_EXTERNALS_GSMINTERFACECONNECTIONHELPERS_H
#define KNM_EXTERNALS_GSMINTERFACECONNECTIONHELPERS_H

#include <QtNetworkManager/modemdevice.h>

#include "knminternals_export.h"

class QObject;
class QString;

namespace ModemManager {
    class ModemInterface;
}

namespace Knm {

class Connection;
class GsmInterfaceConnection;

class KNMINTERNALS_EXPORT GsmInterfaceConnectionHelpers {
public:
    /**
     * This helper builds a GsmInterfaceConnection object from
     * the corresponding Connection object.
     */
    static Knm::GsmInterfaceConnection * buildGsmInterfaceConnection(
                       NetworkManager::ModemDevice *interface,
                       Knm::Connection * connection,
                       const QString & deviceUni,
                       QObject * parent);

    /**
     * This helper syncs a GsmInterfaceConnection object with
     * the corresponding Connection object.
     */
    static void syncGsmInterfaceConnection(Knm::GsmInterfaceConnection *ic, Knm::Connection *c);
};
} //Knm

#endif // KNM_EXTERNALS_GSMINTERFACECONNECTIONHELPERS_H
