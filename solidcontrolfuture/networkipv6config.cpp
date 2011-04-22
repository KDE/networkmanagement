/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.net>, based on work by Will Stephenson <wstephenson@kde.org>

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

#include "networkipv6config.h"

namespace Solid
{
namespace Control
{
class IPv6Config::Private
{
public:
    Private(const QList<IPv6Address> &theAddresses,
        const QList<Q_IPV6ADDR> &theNameservers,
        const QStringList &theDomains, const QList<IPv6Route> &theRoutes)
        : addresses(theAddresses), nameservers(theNameservers),
        domains(theDomains), routes(theRoutes)
    {}
    Private()
    {}
    QList<IPv6Address> addresses;
    QList<Q_IPV6ADDR> nameservers;
    QStringList domains;
    QList<IPv6Route> routes;
};


class IPv6Address::Private
{
public:
    Private(Q_IPV6ADDR theAddress, quint32 theNetMask, Q_IPV6ADDR theGateway)
        : address(theAddress), netMask(theNetMask), gateway(theGateway)
    {}
    Private()
        : address(), netMask(0), gateway()
    {}
    Q_IPV6ADDR address;
    quint32 netMask;
    Q_IPV6ADDR gateway;
};

class IPv6Route::Private
{
public:
    Private(Q_IPV6ADDR theRoute, quint32 thePrefix, Q_IPV6ADDR theNextHop, quint32 theMetric)
        : route(theRoute), prefix(thePrefix), nextHop(theNextHop), metric(theMetric)
    {}
    Private()
        : route(), prefix(0), nextHop(), metric(0)
    {}
    Q_IPV6ADDR route;
    quint32 prefix;
    Q_IPV6ADDR nextHop;
    quint32 metric;
};
}
}

Solid::Control::IPv6Address::IPv6Address(Q_IPV6ADDR address, quint32 netMask, Q_IPV6ADDR gateway)
: d(new Private(address, netMask, gateway))
{
}

Solid::Control::IPv6Address::IPv6Address()
: d(new Private())
{
}

Solid::Control::IPv6Address::~IPv6Address()
{
    delete d;
}

Solid::Control::IPv6Address::IPv6Address(const IPv6Address &other)
: d(new Private(*other.d))
{
}

Q_IPV6ADDR Solid::Control::IPv6Address::address() const
{
    return d->address;
}

quint32 Solid::Control::IPv6Address::netMask() const
{
    return d->netMask;
}

Q_IPV6ADDR Solid::Control::IPv6Address::gateway() const
{
    return d->gateway;
}

Solid::Control::IPv6Address &Solid::Control::IPv6Address::operator=(const Solid::Control::IPv6Address &other)
{
    *d = *other.d;
    return *this;
}

bool Solid::Control::IPv6Address::isValid() const
{
    return !QHostAddress(d->address).isNull();
}

Solid::Control::IPv6Route::IPv6Route(Q_IPV6ADDR route, quint32 prefix, Q_IPV6ADDR nextHop, quint32 metric)
: d(new Private(route, prefix, nextHop, metric))
{
}

Solid::Control::IPv6Route::IPv6Route()
: d(new Private())
{
}

Solid::Control::IPv6Route::~IPv6Route()
{
    delete d;
}

Solid::Control::IPv6Route::IPv6Route(const IPv6Route &other)
: d(new Private(*other.d))
{
}

Q_IPV6ADDR Solid::Control::IPv6Route::route() const
{
    return d->route;
}

quint32 Solid::Control::IPv6Route::prefix() const
{
    return d->prefix;
}

Q_IPV6ADDR Solid::Control::IPv6Route::nextHop() const
{
    return d->nextHop;
}

quint32 Solid::Control::IPv6Route::metric() const
{
    return d->metric;
}

Solid::Control::IPv6Route &Solid::Control::IPv6Route::operator=(const Solid::Control::IPv6Route &other)
{
    *d = *other.d;
    return *this;
}

bool Solid::Control::IPv6Route::isValid() const
{
    return !QHostAddress(d->route).isNull();
}


Solid::Control::IPv6Config::IPv6Config(const QList<IPv6Address> &addresses,
        const QList<Q_IPV6ADDR> &nameservers,
        const QStringList &domains,
        const QList<IPv6Route> &routes)
: d(new Private(addresses, nameservers, domains, routes))
{
}

Solid::Control::IPv6Config::IPv6Config()
: d(new Private())
{
}

Solid::Control::IPv6Config::IPv6Config(const Solid::Control::IPv6Config& other)
{
    d = new Private(*other.d);
}

Solid::Control::IPv6Config::~IPv6Config()
{
    delete d;
}

QList<Solid::Control::IPv6Address> Solid::Control::IPv6Config::addresses() const
{
    return d->addresses;
}

QList<Q_IPV6ADDR> Solid::Control::IPv6Config::nameservers() const
{
    return d->nameservers;
}

QStringList Solid::Control::IPv6Config::domains() const
{
    return d->domains;
}

QList<Solid::Control::IPv6Route> Solid::Control::IPv6Config::routes() const
{
    return d->routes;
}

Solid::Control::IPv6Config &Solid::Control::IPv6Config::operator=(const Solid::Control::IPv6Config& other)
{
    *d = *other.d;
    return *this;
}

bool Solid::Control::IPv6Config::isValid() const
{
    return !d->addresses.isEmpty();
}

