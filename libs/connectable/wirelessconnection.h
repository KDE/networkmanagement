/*
Copyright 2008 Frederik Gladhorn <gladhorn@kde.org>

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

#ifndef WIRELESSCONNECTION_H
#define WIRELESSCONNECTION_H

#include "connection.h"

#include <QtCore/QString>
#include <QObject>

class WirelessConnection :public Connection
{
    Q_OBJECT

public:
    WirelessConnection();
    
    virtual ~WirelessConnection() {}

    void setNetwork(const QString& network);
    QString network();
    
private:
    QString m_network;

};

#endif
