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
