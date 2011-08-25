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

#ifndef NM09_NETWORKINTERFACE_P_H
#define NM09_NETWORKINTERFACE_P_H

#include "dbus/nm-deviceinterface.h"
#include "nmpropertyhelper.h"

class NMNetworkManagerNm09;

class NMNetworkInterfacePrivate
{
public:
    NMNetworkInterfacePrivate( const QString & path, QObject * owner );
    virtual ~NMNetworkInterfacePrivate();
    OrgFreedesktopNetworkManagerDeviceInterface deviceIface;
    Solid::Control::NetworkInterfaceNm09::Capabilities capabilities;
    //bool isLinkUp;
    QString uni;
    QString udi;
    int designSpeed;
    Solid::Control::NetworkInterfaceNm09::ConnectionState connectionState;
    bool managed;
    Solid::Control::IPv4ConfigNm09 ipV4Config;
    QString driver;
    uint ipV4Address;
    QString interfaceName;
    bool firmwareMissing;
    NMNetworkManagerNm09 * manager;
    //NMPropertyHelper propHelper;
};

#endif

