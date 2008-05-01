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

#include "accesspoint.h"

AccessPoint::AccessPoint()
    : m_essid(QString()),
      m_macAddress(QString()),
      m_type(UnknownType),
      m_signalStrength(0),
      m_isEncrypted(false),
      m_isValid(false)
{
    
}

AccessPoint::AccessPoint(QString essid, ApType type, int signalStrength, QString macAddress, bool isEncrypted)
    : m_essid(essid),
      m_macAddress(macAddress),
      m_type(type),
      m_signalStrength(signalStrength),
      m_isEncrypted(isEncrypted),
      m_isValid(true)
{
}

AccessPoint::~AccessPoint()
{
}

void AccessPoint::setData(QString essid, ApType type, int signalStrength, QString macAddress, bool isEncrypted)
{
    m_essid = essid;
    m_type = type;
    m_signalStrength = signalStrength;
    m_macAddress = macAddress;
    m_isEncrypted = isEncrypted;
    m_isValid = true;
}

QString AccessPoint::essid() const
{
        return m_essid;
}

AccessPoint::ApType AccessPoint::type() const
{
    return m_type;
}

QString AccessPoint::macAddress() const
{
    return m_macAddress;
}

int AccessPoint::signalStrength() const
{
    return m_signalStrength;
}

bool AccessPoint::encrypted() const
{
    return m_isEncrypted;
}

bool AccessPoint::isValid()
{
    return m_isValid;
}

bool AccessPoint::operator<(const AccessPoint &other) const
{
    return this->m_signalStrength < other.signalStrength();
}

bool AccessPoint::operator>(const AccessPoint &other) const
{
    return this->m_signalStrength > other.signalStrength();
}

