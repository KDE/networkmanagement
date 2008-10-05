/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

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

#include "networkmanager.h"

#include <QIcon>

#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>

#include "networkmanagerpopup.h"

K_EXPORT_PLASMA_APPLET(networkmanager_alpha, NetworkManagerApplet)

/* for qSort()ing */
bool networkInterfaceLessThan(Solid::Control::NetworkInterface * if1, Solid::Control::NetworkInterface * if2);
bool networkInterfaceSameConnectionStateLessThan(Solid::Control::NetworkInterface * if1, Solid::Control::NetworkInterface * if2);

NetworkManagerApplet::NetworkManagerApplet(QObject * parent, const QVariantList & args)
: Plasma::PopupApplet(parent, args), m_iconPerDevice(false), m_svg(this)
{
    setHasConfigurationInterface(true);
    setPopupIcon(QIcon());
    setAspectRatioMode(Plasma::ConstrainedSquare);// copied from Battery - the comment for this value is meaningless
    m_svg.setImagePath("networkmanager/networkmanager_alpha");
    m_popup = new NetworkManagerPopup(this);
}

NetworkManagerApplet::~NetworkManagerApplet()
{

}

void NetworkManagerApplet::init()
{
    KConfigGroup cg = config();
    m_iconPerDevice = cg.readEntry("IconPerDevice", false);
    m_svg.resize(contentsRect().size());
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceAdded(const QString&)),
            this, SLOT(networkInterfaceAdded(const QString&)));
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceRemoved(const QString&)),
            this, SLOT(networkInterfaceRemoved(const QString&)));
    foreach (Solid::Control::NetworkInterface * interface,
            Solid::Control::NetworkManager::networkInterfaces()) {
        QObject::connect(interface, SIGNAL(connectionStateChanged(int)), this, SLOT(interfaceConnectionStateChanged(int)));
    }
}

void NetworkManagerApplet::paintInterface(QPainter * p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    // i can't figure out how to do layouting of multiple items in constraintsEvent properly,
    // so only have 1 rather than hack something ugly that will be thrown out later
    Solid::Control::NetworkInterfaceList interfaces
        = Solid::Control::NetworkManager::networkInterfaces();
    qSort(interfaces.begin(), interfaces.end(), networkInterfaceLessThan);
    Solid::Control::NetworkInterface *iface = interfaces.first();
    kDebug() << "most interesting interface to paint: " << iface->uni();
    paintInterfaceStatus(iface, p, option, contentsRect);
}
void NetworkManagerApplet::paintInterfaceStatus(Solid::Control::NetworkInterface* interface, QPainter * p, const QStyleOptionGraphicsItem * option, const QRect &contentsRect)
{
    QString elementNameToPaint;
    switch (interface->type() ) {
        case Solid::Control::NetworkInterface::Ieee8023:
            elementNameToPaint = "wired";
            break;
        case Solid::Control::NetworkInterface::Ieee80211:
            elementNameToPaint = "wireless";
            break;
        case Solid::Control::NetworkInterface::Serial:
            elementNameToPaint = "ppp";
            break;
        case Solid::Control::NetworkInterface::Gsm:
        case Solid::Control::NetworkInterface::Cdma:
            elementNameToPaint = "cellular";
            break;
        default:
            elementNameToPaint = "wired";
            break;
    }
    if (interface->connectionState() == Solid::Control::NetworkInterface::Activated) {
        elementNameToPaint += "_connected";
    } else {
        elementNameToPaint += "_disconnected";
    }
    m_svg.paint(p, contentsRect, elementNameToPaint);
}

QGraphicsWidget * NetworkManagerApplet::graphicsWidget()
{
    return m_popup;
}

void NetworkManagerApplet::popupEvent(bool show)
{
    kDebug() << show;
}

/* Slots to react to changes from the daemon */
void NetworkManagerApplet::networkInterfaceAdded(const QString & uni)
{
    // update the tray icon
    update();
    // update popup contents
    // do notifications
}

void NetworkManagerApplet::networkInterfaceRemoved(const QString & uni)
{
    // update the tray icon
    update();
    // update popup contents
    // do notifications
    // kill any animations involving this interface
}

void NetworkManagerApplet::interfaceConnectionStateChanged(int)
{
    kDebug();
    Solid::Control::NetworkInterface * interface = static_cast<Solid::Control::NetworkInterface *>(sender());
    // notifications
    // update appearance
    update();
}

bool networkInterfaceLessThan(Solid::Control::NetworkInterface *if1, Solid::Control::NetworkInterface * if2)
{
    /*
     * status merging algorithm
     * In descending order of importance:
     * - Connecting devices
     *   - Cellular devices (because of cost)
     *   - = PPP devices
     *   - Ethernet devices
     *   - Wireless devices
     * - Connected devices
     *   - order as above
     * - Disconnected devices
     *   - order as above
     */
    bool lessThan = false;
    Solid::Control::NetworkInterface::ConnectionState if2State = if2->connectionState();
    switch (if1->connectionState()) {
        case Solid::Control::NetworkInterface::Preparing:
        case Solid::Control::NetworkInterface::Configuring:
        case Solid::Control::NetworkInterface::NeedAuth:
        case Solid::Control::NetworkInterface::IPConfig:
            if ( if2State == Solid::Control::NetworkInterface::Preparing
                    || if2State == Solid::Control::NetworkInterface::Configuring
                    || if2State == Solid::Control::NetworkInterface::NeedAuth
                    || if2State == Solid::Control::NetworkInterface::IPConfig
               ) {
                lessThan = networkInterfaceSameConnectionStateLessThan(if1, if2);
            } else {
                lessThan = true;
            }
            break;
        case Solid::Control::NetworkInterface::Activated:
            switch (if2->connectionState()) {
                case Solid::Control::NetworkInterface::Preparing:
                case Solid::Control::NetworkInterface::Configuring:
                case Solid::Control::NetworkInterface::NeedAuth:
                case Solid::Control::NetworkInterface::IPConfig:
                    lessThan = false;
                    break;
                case Solid::Control::NetworkInterface::Activated:
                    lessThan = networkInterfaceSameConnectionStateLessThan(if1, if2);
                    break;
                default:
                    lessThan = true;
            }
            break;
        default:
            lessThan = networkInterfaceSameConnectionStateLessThan(if1, if2);

    }
    return lessThan;
}

bool networkInterfaceSameConnectionStateLessThan(Solid::Control::NetworkInterface * if1, Solid::Control::NetworkInterface * if2)
{
    bool lessThan = false;
    switch (if1->type() ) {
        case Solid::Control::NetworkInterface::Ieee8023:
            switch (if2->type()) {
                case Solid::Control::NetworkInterface::Ieee8023:
                    lessThan = if1->uni() < if2->uni();
                    break;
                case Solid::Control::NetworkInterface::Ieee80211:
                    lessThan = true;
                    break;
                case Solid::Control::NetworkInterface::Serial:
                case Solid::Control::NetworkInterface::Gsm:
                case Solid::Control::NetworkInterface::Cdma:
                default:
                    lessThan = false;
                    break;
            }
            break;
        case Solid::Control::NetworkInterface::Ieee80211:
            switch (if2->type()) {
                case Solid::Control::NetworkInterface::Ieee8023:
                    lessThan = false;
                    break;
                case Solid::Control::NetworkInterface::Ieee80211:
                    lessThan = if1->uni() < if2->uni();
                    break;
                case Solid::Control::NetworkInterface::Serial:
                case Solid::Control::NetworkInterface::Gsm:
                case Solid::Control::NetworkInterface::Cdma:
                    lessThan = false;
                    break;
                default:
                    lessThan = true;
                    break;
            }
            break;
        case Solid::Control::NetworkInterface::Serial:
            switch (if2->type()) {
                case Solid::Control::NetworkInterface::Ieee8023:
                case Solid::Control::NetworkInterface::Ieee80211:
                    lessThan = true;
                    break;
                case Solid::Control::NetworkInterface::Serial:
                    lessThan = if1->uni() < if2->uni();
                    break;
                case Solid::Control::NetworkInterface::Gsm:
                case Solid::Control::NetworkInterface::Cdma:
                    lessThan = false;
                    break;
                default:
                    lessThan = true;
                    break;
            }
            break;
        case Solid::Control::NetworkInterface::Gsm:
            switch (if2->type()) {
                case Solid::Control::NetworkInterface::Ieee8023:
                case Solid::Control::NetworkInterface::Ieee80211:
                case Solid::Control::NetworkInterface::Serial:
                    lessThan = true;
                    break;
                case Solid::Control::NetworkInterface::Gsm:
                    lessThan = if1->uni() < if2->uni();
                    break;
                case Solid::Control::NetworkInterface::Cdma:
                    lessThan = false;
                    break;
                default:
                    lessThan = true;
                    break;
            }
            break;
        case Solid::Control::NetworkInterface::Cdma:
            switch (if2->type()) {
                case Solid::Control::NetworkInterface::Ieee8023:
                case Solid::Control::NetworkInterface::Ieee80211:
                case Solid::Control::NetworkInterface::Serial:
                case Solid::Control::NetworkInterface::Gsm:
                    lessThan = true;
                    break;
                case Solid::Control::NetworkInterface::Cdma:
                    lessThan = if1->uni() < if2->uni();
                    break;
                default:
                    lessThan = true;
                    break;
            }
            break;
        default:
            lessThan = false;
        }
    return lessThan;
}

#include "networkmanager.moc"
