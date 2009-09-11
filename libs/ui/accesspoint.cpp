/*
Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>

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

AccessPoint::AccessPoint(QString essid, int signalStrength, QString macAddress, bool isEncrypted)
    : m_essid(essid),
      m_macAddress(macAddress),
      m_signalStrength(signalStrength),
      m_isEncrypted(isEncrypted),
      m_isValid(true)
{
}

AccessPoint::~AccessPoint()
{
}

void AccessPoint::setData(QString essid, int signalStrength, QString macAddress, bool isEncrypted)
{
    m_essid = essid;
    m_signalStrength = signalStrength;
    m_macAddress = macAddress;
    m_isEncrypted = isEncrypted;
    m_isValid = true;
}

QString AccessPoint::essid() const
{
        return m_essid;
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

