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

#ifndef ACCESSPOINT_H
#define ACCESSPOINT_H

#include <QObject>

class AccessPoint
{
    public:
        enum ApType {UnknownType, Wireless, Home, Work, Cafe, Airport};
        
        AccessPoint();
        AccessPoint(QString essid, int signalStrength, QString macAddress, bool isEncrypted);
        ~AccessPoint();

        void setData(QString essid, int signalStrength, QString macAddress, bool isEncrypted);
        QString essid() const;
        QString macAddress() const;
        int signalStrength() const;
        bool encrypted() const;
        bool isValid();
        bool operator<(const AccessPoint &other) const;
        bool operator>(const AccessPoint &other) const;

    private:
        QString m_essid, m_macAddress;
        ApType  m_type;
        int m_signalStrength;
        bool m_isEncrypted, m_isValid;
};

#endif
