/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2009 Paul Marchouk <paul.marchouk@gmail.com>

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

#include "tooltipbuilder.h"

#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>
#include <solid/control/networkipv4config.h>

#include <interfaceconnection.h>

 #include <KLocale>

// probably this function should be available for other code
// intentionally didn't include this function in a class
QString interfaceTypeToString(Solid::Control::NetworkInterface::Type type)
{
    QString str;
    switch (type) {
        case Solid::Control::NetworkInterface::UnknownType:
            str = i18n("Unknown type");
            break;
        case Solid::Control::NetworkInterface::Ieee8023:
            str = i18nc("The network interface type","Ethernet");
            break;
        case Solid::Control::NetworkInterface::Ieee80211:
            str = i18nc("The network interface type","Wi-Fi");
            break;
        case Solid::Control::NetworkInterface::Serial:
            str = i18nc("The network interface type","Serial");
            break;
        case Solid::Control::NetworkInterface::Gsm:
            str = i18nc("The network interface type","GSM");
            break;
        case Solid::Control::NetworkInterface::Cdma:
            str = i18nc("The network interface type","CDMA");
            break;
        default:
            // oops, we need update this function, or something went wrong
            break;
    }

    return str;
}

QString ToolTipBuilder::toolTipForInterfaceConnection(Knm::InterfaceConnection * interfaceConnection)
{
    QString tip;
    if (interfaceConnection) {
        QStringList tipElements, allTipElements;
        /* All possible names for tooltip elements
         * Eventually there will be a UI where the user can select which elements to show
         */

        allTipElements << "interface:type" << "interface:name" << "interface:driver" << "interface:status" << "interface:designspeed"
            << "interface:hardwareaddress" << "interface:bitrate"
            << "ipv4:address" << "ipv4:nameservers" << "ipv4:domains" << "ipv4:routes"
            << "wired:carrier"
            /* These come from Solid::Control::WirelessNetworkInterface _and_ its active
               Solid::Control::AccessPoint, if any */
            << "wireless:strength" << "wireless:ssid" << "wireless:bitrate" << "wireless:mode" << "wireless:accesspoint"
            << /* High level description of security in use */ "wireless:security"
            << /* low level flags so ciphers can be seen*/ "wireless:wpaflags" << "wireless:rsnflags"
            << "wireless:frequency"
            /* These are not yet available in the API!*/
            << "cellular:strength" << "cellular:network";

        /* default set to use for initial development */
        tipElements << "interface:type" << "interface:name" << "interface:driver" << "interface:status" << "interface:designspeed";

        QString deviceUni = interfaceConnection->deviceUni();
        Solid::Control::NetworkInterface * iface = Solid::Control::NetworkManager::findNetworkInterface(deviceUni);
        if (iface) {
            // generate html table header
            tip = QLatin1String("<qt><table>");

            // iterate each item in tipElements and generate a table row for it
            for (int i = 0; i < tipElements.size(); i++) {
                tip += QLatin1String("<tr><td>");

                if (tipElements.at(i) == QLatin1String("interface:name")) {
                    tip += i18nc("@info:tooltip network device name eg eth0","<b>Interface:</b>&nbsp;%1", iface->interfaceName());
                }
                else if (tipElements.at(i) == QLatin1String("interface:type")) {
                    tip += i18nc("@info:tooltip interface type", "<b>Type:</b>&nbsp;%1", interfaceTypeToString(iface->type()));
                }
                else if (tipElements.at(i) == QLatin1String("interface:driver")) {
                    tip += i18nc("@info:tooltip system driver name", "<b>Driver:</b>&nbsp;%1", iface->driver());
                }
                /* This should be a string derived from NetworkInterface::ConnectionState - the
                 * relevant switch code is already in networkmanager/applet/networkmanager.cpp
                 */
                else if (tipElements.at(i) == QLatin1String("interface:status")) {
                    tip += i18nc("@info:tooltip connection status of an interface", "<b>Status:</b>&nbsp;%1",
                            QString::number(iface->connectionState()));
                }
                else if (tipElements.at(i) == QLatin1String("interface:designspeed")) {
                    tip += i18nc("@info:tooltip The network device's maximum speed", "<b>Max speed:</b>&nbsp;%1",
                           iface->designSpeed());
                }
                else if (tipElements.at(i) == QLatin1String("interface:hardwareaddress")) {
                    tip += i18nc("@info:tooltip this is the hardware address of a network interface", "<b>Hardware address:</b>&nbsp;%1");//TODO
                }
                else if (tipElements.at(i) == QLatin1String("ipv4:address")) {
                    tip += i18nc("@info:tooltip IPv4 address", "<b>IP address:</b>&nbsp;%1");//TODO
                }
                else if (tipElements.at(i) == QLatin1String("interface:bitrate")) {
                    tip += i18nc("@info:tooltip network connection bit raet","<b>Bit rate:</b>&nbsp;%1");//TODO
                }

                tip += QLatin1String("</td></tr>");
            }
            tip += QLatin1String("</table></qt>");
        }
    }
    return tip;
}

// vim: sw=4 sts=4 et tw=100
