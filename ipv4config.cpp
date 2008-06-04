/*  This file is part of the KDE project
    Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>

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

#include "ip4config.h"

IP4Config::IP4Config(const Solid::Control::IPv4Config &config, QObejct *parent=0)
    : QObject(parent),
      ipv4Config(config)
{
}

IP4Config::~IP4Config()
{
}

QList< QList<uint> > IP4Config::Addresses() const
{
    QList<uint> address;
    address << ipv4Config.address() << ipv4Config.netmask() << ipv4Config.gateway();

    return QList< QList<uint> >() << address;
}

QString IP4Config::Hostname() const
{
    ipv4Config.hostname();
}

QList<uint> IP4Config::Nameservers() const
{
    ipv4Config.nameservers();
}

QStringList IP4Config::Domains() const
{
    ipv4Config.domains();
}

QString IP4Config::NisDomain() const
{
    ipv4Config.nisDomain();
}

QList<uint> IP4Config::NisServers() const
{
    ipv4Config.nisServers();
}

#include "ip4config.moc"
