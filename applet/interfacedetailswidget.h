/*
Copyright 2010 Sebastian Kügler <sebas@kde.org>

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

#ifndef INTERFACEDETAILSWIDGET_H
#define INTERFACEDETAILSWIDGET_H

#include <QGraphicsGridLayout>
#include <QGraphicsWidget>
#include <QGraphicsLinearLayout>

#include <solid/control/networkinterface.h>
#include <Plasma/Label>

class RemoteActivatable;
class RemoteInterfaceConnection;
class RemoteInterfaceList;

class InterfaceDetailsWidget : public QGraphicsWidget
{
Q_OBJECT
    public:
        InterfaceDetailsWidget(QGraphicsItem* parent = 0);
        virtual ~InterfaceDetailsWidget();
        void setInterface(Solid::Control::NetworkInterface* iface);
        void setMAC(Solid::Control::NetworkInterface* iface);
        void setIP(QString ip);

    Q_SIGNALS:
        void back();

    private:
        Solid::Control::NetworkInterface* m_iface;

        Plasma::Label* m_interfaceLabel;
        Plasma::Label* m_interface;
        Plasma::Label* m_macLabel;
        Plasma::Label* m_mac;
        Plasma::Label* m_ipLabel;
        Plasma::Label* m_ip;
        Plasma::Label* m_driverLabel;
        Plasma::Label* m_driver;
        Plasma::Label* m_typeLabel;
        Plasma::Label* m_type;
        //Plasma::Label* m_speedLabel;
        //Plasma::Label* m_speed;
        Plasma::Label* m_stateLabel;
        Plasma::Label* m_state;
        Plasma::Label* m_bitLabel;
        Plasma::Label* m_bit;
        //Plasma::Label* m_otherLabel;
        //Plasma::Label* m_other;
};

#endif // INTERFACEDETAILSWIDGET_H
