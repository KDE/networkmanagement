/*
Copyright 2008 Sebastian Kügler <sebas@kde.org>

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

#include "connectionitem.h"

class QGraphicsGridLayout;

class RemoteConnection;
class WirelessNetwork;

namespace Plasma
{
    class Icon;
    class Label;
    class Meter;
}

/**
 * Represents an inactive connection
 */
class WirelessConnectionItem : public ConnectionItem
{
Q_OBJECT
public:
    WirelessConnectionItem(RemoteConnection * conn, QGraphicsItem * parent = 0);
    virtual ~WirelessConnectionItem();
    void setupItem();
    void setNetwork(const WirelessNetwork * network);
    QString essid();
protected:
    RemoteConnection * m_connection;
private slots:
    void setStrength(QString essid, int strength);
private:
    void readSettings();
    QGraphicsGridLayout * m_layout;
    Plasma::Icon * m_icon;
    Plasma::Label * m_connectionNameLabel;
    Plasma::Meter * m_strengthMeter;
    Plasma::Icon * m_connectButton;
    QString m_security;
    Plasma::Icon * m_securityIcon;
    QString m_securityIconName;
    WirelessNetwork * m_wirelessNetwork;
    QString m_essid;
    int m_strength;
};

#endif //#define APPLET_WIRELESSCONNECTIONITEM_H

