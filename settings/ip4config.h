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

#ifndef IP4CONFIG_H
#define IP4CONFIG_H

#include <QObject>
#include <QVariant>
#include <QMap>
#include <QString>

#include <solid/control/networkipv4config.h>

class IP4Config : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.NetworkManagerSettings.IP4Config")

    public:
        IPV4Config(const Solid::Control::IPv4Config &config, QObejct *parent=0);
        ~IPV4Config();

        Q_SCRIPTABLE QList< QList<uint> > Addresses() const;
        Q_SCRIPTABLE QString Hostname() const;
        Q_SCRIPTABLE QList<uint> Nameservers() const;
        Q_SCRIPTABLE QStringList Domains() const;
        Q_SCRIPTABLE QString NisDomain() const;
        Q_SCRIPTABLE QList<uint> NisServers() const;

    private:
        Solid::Control::IPv4Config ipv4Config;
};

#endif