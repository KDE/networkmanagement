/*
* Copyright 2008 Will Stephenson <wstephenson@kde.org>

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

#ifndef SOLID_CONTROL_NETWORKIPV4CONFIG_NM09_H
#define SOLID_CONTROL_NETWORKIPV4CONFIG_NM09_H

#include <QtCore/QStringList>

#include "solid_control_export.h"

namespace Solid
{
namespace Control
{

class SOLIDCONTROL_EXPORT IPv4AddressNm09
{
public:
    IPv4AddressNm09(quint32 address, quint32 netMask, quint32 gateway);
    IPv4AddressNm09();
    ~IPv4AddressNm09();
    IPv4AddressNm09(const IPv4AddressNm09&);
    quint32 address() const;
    quint32 netMask() const;
    quint32 gateway() const;
    IPv4AddressNm09 &operator=(const IPv4AddressNm09&);
    bool isValid() const;
private:
    class Private;
    Private * d;
};

class SOLIDCONTROL_EXPORT IPv4RouteNm09
{
public:
    IPv4RouteNm09(quint32 route, quint32 prefix, quint32 nextHop, quint32 metric);
    IPv4RouteNm09();
    ~IPv4RouteNm09();
    IPv4RouteNm09(const IPv4RouteNm09&);
    IPv4RouteNm09 &operator=(const IPv4RouteNm09&);
    bool isValid() const;
    quint32 route() const;
    quint32 prefix() const;
    quint32 nextHop() const;
    quint32 metric() const;
private:
    class Private;
    Private * d;
};

class SOLIDCONTROL_EXPORT IPv4ConfigNm09
{
public:
    IPv4ConfigNm09(const QList<IPv4AddressNm09> &addresses,
        const QList<quint32> &nameservers,
        const QStringList &domains,
        const QList<IPv4RouteNm09> &routes);
    IPv4ConfigNm09();
    ~IPv4ConfigNm09();
    IPv4ConfigNm09(const IPv4ConfigNm09&);
    /** 
     * List of IP addresses related to this configuration. 
     */
    QList<IPv4AddressNm09> addresses() const;
    QList<quint32> nameservers() const;
    QStringList domains() const;
    QList<IPv4RouteNm09> routes() const;
    IPv4ConfigNm09 &operator=(const IPv4ConfigNm09&);
    bool isValid() const;
private:
    class Private;
    Private * d;
};

} // namespace Control
} // namespace Solid

#endif // NETWORKIPV4CONFIG_H
