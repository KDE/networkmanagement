/*
Copyright 2008-2010 Sebastian Kügler <sebas@kde.org>
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

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

#ifndef APPLET_WIRELESSNETWORKITEM_H
#define APPLET_WIRELESSNETWORKITEM_H

#include "activatableitem.h"

class QGraphicsGridLayout;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

namespace Plasma
{
    class IconWidget;
    class Label;
    class Meter;
}


class RemoteWirelessNetwork;
class WirelessStatus;
/**
    Represents a wireless network or a known connection
 */
class WirelessNetworkItem : public ActivatableItem
{
    Q_OBJECT
    public:
        WirelessNetworkItem(RemoteWirelessNetwork * remote, QGraphicsItem * parent = 0);
        virtual ~WirelessNetworkItem();
        void setupItem();
        QString ssid();
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    private Q_SLOTS:
        void setStrength(int strength);
        void update();
        void activationStateChanged(Knm::InterfaceConnection::ActivationState oldState, Knm::InterfaceConnection::ActivationState newState);

    private:
        //bool readSettings();
        QGraphicsGridLayout* m_layout;
        Plasma::Label* m_ssidLabel;
        Plasma::Meter* m_strengthMeter;
        Plasma::IconWidget* m_connectButton;
        QString m_security;
        Plasma::Label* m_securityIcon;
        RemoteWirelessNetwork* m_remote;
        WirelessStatus* m_wirelessStatus;
        bool m_layoutIsDirty;
};

#endif //#define APPLET_WIRELESSCONNECTIONITEM_H

