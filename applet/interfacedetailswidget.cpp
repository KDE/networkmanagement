/*
Copyright 2010 Sebastian Kügler <sebas@kde.org>
Copyright 2010 Alexander Naumov <posix.ru@gmail.com>

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

// own
#include <interfacedetailswidget.h>

// Qt
#include <QGraphicsLinearLayout>
#include <QGridLayout>
#include <QGraphicsLayout>

// KDE
#include <kdebug.h>

// Plasma
#include <Plasma/Label>
#include <Plasma/IconWidget>

//Solid
#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirednetworkinterface.h>
#include <solid/control/networkipv4config.h>
#include <solid/control/networkinterface.h>
#include <solid/control/networkmanager.h>


#include <uiutils.h>

#include "interfaceitem.h"

class InterfaceItem;

InterfaceDetailsWidget::InterfaceDetailsWidget(QGraphicsItem * parent) : QGraphicsWidget(parent, 0),
    m_iface(0)
{
    QGraphicsGridLayout *m_gridLayout = new QGraphicsGridLayout;

    //Interface
    m_interfaceLabel = new Plasma::Label;
    m_interfaceLabel->setText("<h4>Interface:</h4>");
    m_gridLayout->addItem(m_interfaceLabel, 0, 0);

    m_interface = new Plasma::Label;
    m_interface->setText("Interface details!");
    m_gridLayout->addItem(m_interface, 0, 1);

    //MAC
    m_macLabel = new Plasma::Label;
    m_macLabel->setText("<h4>MAC:</h4>");
    m_gridLayout->addItem(m_macLabel, 1, 0);

    m_mac = new Plasma::Label;
    m_mac->setText("MAC details!");
    m_gridLayout->addItem(m_mac, 1, 1);

    //IP
    m_ipLabel = new Plasma::Label;
    m_ipLabel->setText("<h4>IP:</h4>");
    m_gridLayout->addItem(m_ipLabel, 2, 0);

    m_ip = new Plasma::Label;
    m_ip->setText("IP details!");
    m_gridLayout->addItem(m_ip, 2, 1);


    //Driver
    m_driverLabel = new Plasma::Label;
    m_driverLabel->setText("<h4>Driver:</h4>");
    m_gridLayout->addItem(m_driverLabel, 3, 0);

    m_driver = new Plasma::Label;
    m_driver->setText("Driver details!");
    m_gridLayout->addItem(m_driver, 3, 1);

    //Type
    m_typeLabel = new Plasma::Label;
    m_typeLabel->setText("<h4>Type:</h4>");
    m_gridLayout->addItem(m_typeLabel, 4, 0);

    m_type = new Plasma::Label;
    m_type->setText("Type details!");
    m_gridLayout->addItem(m_type, 4, 1);

    //State
    m_stateLabel = new Plasma::Label;
    m_stateLabel->setText("<h4>State:</h4>");
    m_gridLayout->addItem(m_stateLabel, 5, 0);

    m_state = new Plasma::Label;
    m_state->setText("State details!");
    m_gridLayout->addItem(m_state, 5, 1);

    //Bit
    m_bitLabel = new Plasma::Label;
    m_bitLabel->setText("<h4>MBit/s:</h4>");
    m_gridLayout->addItem(m_bitLabel, 6, 0);

    m_bit = new Plasma::Label;
    m_bit->setText("Bit details!");
    m_gridLayout->addItem(m_bit, 6, 1);

    /*
    //Speed
    m_speedLabel = new Plasma::Label(this);
    m_speedLabel->setText("<h4>Speed:</h4> ");
    m_gridLayout->addItem(m_speedLabel, 7, 0);

    m_speed = new Plasma::Label(this);
    m_speed->setText("Speed details!");
    m_gridLayout->addItem(m_speed, 7, 1);
    */

    Plasma::IconWidget* back = new Plasma::IconWidget(this);
    back->setIcon("go-previous");
    m_gridLayout->addItem(back, 0, 2);
    setLayout(m_gridLayout);
    connect(back, SIGNAL(clicked()), this, SIGNAL(back()));
}

InterfaceDetailsWidget::~InterfaceDetailsWidget()
{
}

void InterfaceDetailsWidget::setInterface(Solid::Control::NetworkInterface* iface)
{
    if (iface) {
        m_iface = iface;
        m_interface->setText(m_iface->interfaceName());
        m_driver->setText(iface->driver());
        //m_speed->setText(QString::number(iface->designSpeed()));
        m_type->setText(UiUtils::interfaceTypeLabel(iface->type()));
        m_state->setText(UiUtils::connectionStateToString(iface->connectionState()));
    }
}

void InterfaceDetailsWidget::setMAC(Solid::Control::NetworkInterface* iface)
{
    QString temp;
    //int bitRate = 0;

    //wifi
    Solid::Control::WirelessNetworkInterface * wliface =
                    dynamic_cast<Solid::Control::WirelessNetworkInterface*> (iface);

    if (wliface) {
        temp = wliface->hardwareAddress(); //MAC
        m_mac->setText(temp);

        int bitRate = wliface->bitRate() / 1000; //Bit
        m_bit->setText(QString::number(bitRate));

        Solid::Control::AccessPoint * ap = wliface->findAccessPoint(wliface->activeAccessPoint());
        if(ap) {
            temp = ap->ssid();
            kDebug() << "temp = " << temp;
        }
    } else {     // wired
        Solid::Control::WiredNetworkInterface * wdiface =
                                    dynamic_cast<Solid::Control::WiredNetworkInterface*> (iface);
        if (wdiface) {
            temp = wdiface->hardwareAddress();
            m_mac->setText(temp);
            int bitRate = wdiface->bitRate() / 1000;
            m_bit->setText(QString::number(bitRate));
        }
    }
}

void InterfaceDetailsWidget::setIP(QString ip)
{
    m_ip->setText(ip);
}

// vim: sw=4 sts=4 et tw=100
