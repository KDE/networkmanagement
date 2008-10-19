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

#ifndef CONNECTIONINSPECTOR_H
#define CONNECTIONINSPECTOR_H

#include <QMap>

#include "../libs/types.h"

namespace Solid
{
namespace Control
{
    class NetworkInterface;
    class WiredNetworkInterface;
    class WirelessNetworkInterface;
} // namespace Control
} // namespace Solid
class RemoteConnection;

class ConnectionInspector
{
public:
    virtual ~ConnectionInspector();
    virtual bool accept(RemoteConnection * connection);
    QString connectionType(RemoteConnection * connection);
};

class WiredConnectionInspector : public ConnectionInspector
{
public:
    WiredConnectionInspector(Solid::Control::WiredNetworkInterface* iface);
    bool accept(RemoteConnection * connection);
private:
    Solid::Control::WiredNetworkInterface *m_iface;
};

class WirelessConnectionInspector : public ConnectionInspector
{
public:
    WirelessConnectionInspector(Solid::Control::WirelessNetworkInterface* iface);
    bool accept(RemoteConnection * connection);
private:
    Solid::Control::WirelessNetworkInterface *m_iface;
};

class GsmConnectionInspector : public ConnectionInspector
{
public:
    bool accept(RemoteConnection * connection);
};

class CdmaConnectionInspector : public ConnectionInspector
{
public:
    bool accept(RemoteConnection * connection);
};

class PppoeConnectionInspector : public ConnectionInspector
{
public:
    bool accept(RemoteConnection * connection);
};

class ConnectionInspectorFactory
{
public:
    ConnectionInspector *connectionInspector(Solid::Control::NetworkInterface* iface);
private:
    QMap<Solid::Control::NetworkInterface*, ConnectionInspector*> m_inspectors;
};
#endif // CONNECTIONINSPECTOR_H
