/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_NETWORK_P_H
#define SOLID_NETWORK_P_H

#include "networkinterface_p.h"

namespace Solid
{
namespace Control
{
    class AccessPointNm09;
namespace Ifaces
{
    class AccessPointNm09;
}

    typedef QPair<AccessPointNm09 *, Ifaces::AccessPointNm09 *> AccessPointNm09Pair;
    typedef QMap<QString, AccessPointNm09Pair > AccessPointNm09Map;


    class WirelessNetworkInterfaceNm09Private : public NetworkInterfaceNm09Private
    {
    public:
        explicit WirelessNetworkInterfaceNm09Private(QObject *parent)
            : NetworkInterfaceNm09Private(parent) { }

        void setBackendObject(QObject *object);

        //Solid::Control::AccessPointNm09 *findRegisteredAccessPoint(const QString &uni) const;
        void readAccessPoints();
        AccessPointNm09Pair createAP(const QString &uni) const;

        //mutable QMap<QString, AccessPointNm09 *> apMap;
        mutable AccessPointNm09Map apMap;
    };
}
}

#endif
