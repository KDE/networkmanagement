/*
Copyright 2009 Paul Marchouk <pmarchouk@gmail.com>
Copyright 2010-2011 Lamarque Souza <lamarque@gmail.com>

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

#ifndef KNM_EXTERNALS_GSMINTERFACECONNECTIONHELPERS_P_H
#define KNM_EXTERNALS_GSMINTERFACECONNECTIONHELPERS_P_H

#include <QString>

#include "interfaceconnectionhelpers_p.h"

class QObject;
namespace Solid {
    namespace Control {
        class GsmNetworkInterfaceNm09;
    }
}

namespace Knm {

class Connection;
class GsmInterfaceConnection;

class GsmInterfaceConnectionBuilder : public InterfaceConnectionBuilder
{
public:
    GsmInterfaceConnectionBuilder(Solid::Control::ModemNetworkInterfaceNm09 * interface,
                                       Knm::Connection *connection,
                                       const QString & deviceUni,
                                       QObject * parent);
    virtual ~GsmInterfaceConnectionBuilder();

    Knm::GsmInterfaceConnection* build();

protected:
    void init(GsmInterfaceConnection *ic);

private:
    Q_DISABLE_COPY(GsmInterfaceConnectionBuilder)

protected:
    Solid::Control::ModemNetworkInterfaceNm09 *m_interface;
};


class GsmInterfaceConnectionSync : public InterfaceConnectionSync
{
public:
    GsmInterfaceConnectionSync();
    virtual ~GsmInterfaceConnectionSync();

    void sync(Knm::GsmInterfaceConnection * interfaceConnection,
              Knm::Connection *connection);

private:
    Q_DISABLE_COPY(GsmInterfaceConnectionSync)
};

} //Knm

#endif // KNM_EXTERNALS_GSMINTERFACECONNECTIONHELPERS_P_H
