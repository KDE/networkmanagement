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

#include "networkprofile.h"

#include <klocalizedstring.h>

NetworkProfile::NetworkProfile()
{
}
NetworkProfile::NetworkProfile(const QString &name, Type profileType)
{
    m_name = name;
    m_type = profileType;
}

QString NetworkProfile::name() const
{
    return m_name;
}

void NetworkProfile::setName(const QString &name)
{
    m_name = name;
}

NetworkProfile::Type NetworkProfile::type() const
{
    return m_type;
}

void NetworkProfile::setType(Type profileType)
{
    m_type = profileType;
}

QString NetworkProfile::icon() const
{
    switch (m_type) {
        case Home:
            return QString("go-home");
        case Work:
            return QString("work");
        case Airport:
            return QString("airport");
        case Cafe:
            return QString("cafe");
        case Hotel:
            return QString("hotel");
        default:
            return QString();
    }
}

QString NetworkProfile::typeAsString(Type profileType)
{
    switch (profileType) {
        case Home:
            return QString(i18n("Home"));
        case Work:
            return QString(i18n("Work"));
        case Airport:
            return QString(i18n("Airport"));
        case Cafe:
            return QString(i18n("Cafe"));
        case Hotel:
            return QString(i18n("Hotel"));
        default:
            return QString(i18n("Unknown"));
    }
}

NetworkProfile::Type NetworkProfile::stringAsType(const QString &profileString)
{
    if (profileString == "Home") {
        return Home;
    } else if (profileString == "Work") {
        return Work;
    } else if (profileString == "Airport") {
        return Airport;
    } else if (profileString == "Cafe") {
        return Cafe;
    } else if (profileString == "Hotel") {
        return Hotel;
    } else {
        return UnknownType;
    }
}
