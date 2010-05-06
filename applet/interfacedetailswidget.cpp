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
//#include <QGraphicsLinearLayout>
#include <QGridLayout>
//#include <QGraphicsLayout>
#include <QLabel>

// KDE
#include <kdebug.h>

// Plasma
#include <Plasma/IconWidget>
#include <Plasma/Label>

//Solid
#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirednetworkinterface.h>
#include <solid/control/networkipv4config.h>
#include <solid/control/networkinterface.h>
#include <solid/control/networkmanager.h>
#include <Solid/Device>

#include <uiutils.h>

#include "interfaceitem.h"

class InterfaceItem;

InterfaceDetailsWidget::InterfaceDetailsWidget(QGraphicsItem * parent) : QGraphicsWidget(parent, 0),
    m_iface(0)
{
    m_gridLayout = new QGraphicsGridLayout(this);

    int row = 0;

    //Interface
    m_interfaceNameLabel = new Plasma::Label(this);
    m_interfaceNameLabel->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    //m_interfaceNameLabel->setText("Interface:");
    m_gridLayout->addItem(m_interfaceNameLabel, row, 0, 1, 2);

    //Type
    row++;
    m_typeLabel = new Plasma::Label(this);
    m_typeLabel->setText(i18nc("interface details", "Interface Type:"));
    m_typeLabel->setAlignment(Qt::AlignRight);
    m_gridLayout->addItem(m_typeLabel, row, 0);
    
    m_type = new Plasma::Label(this);
    m_type->setText("Type details!");
    m_type->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_gridLayout->addItem(m_type, row, 1);
    
    
    //State
    row++;
    m_stateLabel = new Plasma::Label(this);
    m_stateLabel->setText(i18nc("interface details", "Connection State:"));
    m_stateLabel->setAlignment(Qt::AlignRight);
    m_gridLayout->addItem(m_stateLabel, row, 0);
    
    m_state = new Plasma::Label(this);
    m_state->setText("State details!");
    m_state->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_gridLayout->addItem(m_state, row, 1);
    
    
    //IP
    row++;
    m_ipLabel = new Plasma::Label(this);
    m_ipLabel->setText(i18nc("interface details", "Network Address (IP):"));
    m_ipLabel->setAlignment(Qt::AlignRight);
    m_gridLayout->addItem(m_ipLabel, row, 0);
    
    m_ip = new Plasma::Label(this);
    m_ip->setText("IP details!");
    m_ip->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_gridLayout->addItem(m_ip, row, 1);
    
    //Bit
    row++;
    m_bitLabel = new Plasma::Label(this);
    m_bitLabel->setText(i18nc("interface details", "Connection Speed:"));
    m_bitLabel->setAlignment(Qt::AlignRight);
    m_gridLayout->addItem(m_bitLabel, row, 0);
    
    m_bit = new Plasma::Label(this);
    m_bit->setText("Bit details!");
    m_bit->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_gridLayout->addItem(m_bit, row, 1);

    //Interface
    row++;
    m_interfaceLabel = new Plasma::Label(this);
    m_interfaceLabel->setText(i18nc("interface details", "System Name:"));
    m_interfaceLabel->setAlignment(Qt::AlignRight);
    m_gridLayout->addItem(m_interfaceLabel, row, 0);
    
    m_interface = new Plasma::Label(this);
    m_interface->setText("Interface details!");
    m_interface->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_gridLayout->addItem(m_interface, row, 1);
    
    //MAC
    row++;
    m_macLabel = new Plasma::Label(this);
    m_macLabel->setText(i18nc("interface details", "Hardware Address (MAC):"));
    m_macLabel->setAlignment(Qt::AlignRight);
    m_gridLayout->addItem(m_macLabel, row, 0);

    m_mac = new Plasma::Label(this);
    m_mac->setText("MAC details!");
    m_mac->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_gridLayout->addItem(m_mac, row, 1);

    //Driver
    row++;
    m_driverLabel = new Plasma::Label(this);
    m_driverLabel->setText(i18nc("interface details", "Driver:"));
    m_driverLabel->setAlignment(Qt::AlignRight);
    m_gridLayout->addItem(m_driverLabel, row, 0);

    m_driver = new Plasma::Label(this);
    m_driver->setText("Driver details!");
    m_driver->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_gridLayout->addItem(m_driver, row, 1);

    /*
    //Speed
    row++;
    m_speedLabel = new Plasma::Label(this);
    m_speedLabel->setText("Speed: ");
    m_speedLabel->setAlignment(Qt::AlignRight);
    m_gridLayout->addItem(m_speedLabel, row, 0);

    m_speed = new Plasma::Label(this);
    m_speed->setText("Speed details!");
    m_gridLayout->addItem(m_speed, row, 1);
    */

    Plasma::IconWidget* back = new Plasma::IconWidget(this);
    back->setIcon("go-previous");
    m_gridLayout->addItem(back, 0, 2, 2, 1);
    setLayout(m_gridLayout);
    connect(back, SIGNAL(clicked()), this, SIGNAL(back()));

    // Add spacer to push content to the top
    row++;
    QGraphicsWidget *spacer = new QGraphicsWidget(this);
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_gridLayout->addItem(spacer, row, 0);
}

InterfaceDetailsWidget::~InterfaceDetailsWidget()
{
}

void InterfaceDetailsWidget::setInterface(Solid::Control::NetworkInterface* iface)
{
    if (iface) {
        m_iface = iface;
        m_interfaceNameLabel->setText(QString("<b>%1</b>").arg(UiUtils::interfaceNameLabel(iface->uni())));
        m_interface->setText(m_iface->interfaceName());
        m_driver->setText(iface->driver());
        //m_speed->setText(QString::number(iface->designSpeed()));
        m_type->setText(UiUtils::interfaceTypeLabel(iface->type()));
        m_state->setText(UiUtils::connectionStateToString(iface->connectionState()));
    }
    Solid::Device *dev = new Solid::Device(iface->uni());
    kDebug() << "IFACE:" << dev->vendor();
    kDebug() << "product:" << dev->product();
    kDebug() << "udi:" << dev->udi();
    kDebug() << "desc:" << dev->description();
    kDebug() << "icon:" << dev->icon();
}

void InterfaceDetailsWidget::setMAC(Solid::Control::NetworkInterface* iface)
{
    QString temp;
    int bitRate = 0;
    QString bit = i18nc("bitrate", "Unknown");

    //wifi?
    Solid::Control::WirelessNetworkInterface * wliface =
                    dynamic_cast<Solid::Control::WirelessNetworkInterface*>(iface);
    if (wliface) {
        m_mac->setText(wliface->hardwareAddress());
        bitRate = wliface->bitRate(); //Bit
        /*
         * for later use ...
        Solid::Control::AccessPoint * ap = wliface->findAccessPoint(wliface->activeAccessPoint());
        if(ap) {
            temp = ap->ssid();
            kDebug() << "temp = " << temp;
        }
        */
    } else {     // wired
        Solid::Control::WiredNetworkInterface * wdiface =
                                    dynamic_cast<Solid::Control::WiredNetworkInterface*> (iface);
        if (wdiface) {
            temp = wdiface->hardwareAddress();
            m_mac->setText(temp);
            bitRate = wdiface->bitRate();
            //m_bit->setText(QString::number(bitRate));
        }
    }
    //*/
    //m_mac->setText(iface->hardwareAddress());
    if (bitRate) {
        m_bit->setText(UiUtils::connectionSpeed(bitRate));
    } else {
        m_bit->setText(i18nc("bitrate", "Unknown"));
    }
}

void InterfaceDetailsWidget::setIP(QString ip)
{
    m_ip->setText(ip);
}

// vim: sw=4 sts=4 et tw=100
