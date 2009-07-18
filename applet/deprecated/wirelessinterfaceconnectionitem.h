/*
Copyright 2008 Sebastian Kügler <sebas@kde.org>
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#ifndef APPLET_WIRELESSCONNECTIONITEM_H
#define APPLET_WIRELESSCONNECTIONITEM_H

#include <QGraphicsWidget>

#include <Plasma/IconWidget>

#include "interfaceconnectionitem.h"

namespace Plasma
{
    class Meter;
}

class RemoteWirelessInterfaceConnection;

/**
 * Represents an inactive connection
 */
class WirelessInterfaceConnectionItem : public InterfaceConnectionItem
{
Q_OBJECT
public:
    WirelessInterfaceConnectionItem(RemoteWirelessInterfaceConnection * conn, QGraphicsItem * parent = 0);
    virtual ~WirelessInterfaceConnectionItem();
    void setupItem();
    //QString ssid();
private slots:
    void setStrength(int strength);
    void update();
private:
    void readSettings();
    RemoteWirelessInterfaceConnection * wirelessInterfaceConnection() const;
    Plasma::Meter * m_strengthMeter;
    Plasma::IconWidget * m_securityIcon;
    QString m_security;
    QString m_securityIconName;
    QString m_securityIconToolTip;
    Plasma::IconWidget * m_connectButton;
    QString m_ssid;
    int m_strength;
};

#endif //#define APPLET_WIRELESSCONNECTIONITEM_H

