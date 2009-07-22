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
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/networkipv4config.h>

#include <interfaceconnection.h>

#include <KLocale>

#include <arpa/inet.h>

// probably these functions should be in a separate file to be
// easily included from different places.
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

QString connectionStateToString(Solid::Control::NetworkInterface::ConnectionState state)
{
    QString stateString;
    switch (state) {
        case Solid::Control::NetworkInterface::UnknownState:
            stateString = i18nc("description of unknown network interface state", "Unknown");
            break;
        case Solid::Control::NetworkInterface::Unmanaged:
            stateString = i18nc("description of unmanaged network interface state", "Unmanaged");
            break;
        case Solid::Control::NetworkInterface::Unavailable:
            stateString = i18nc("description of unavailable network interface state", "Unavailable");
            break;
        case Solid::Control::NetworkInterface::Disconnected:
            stateString = i18nc("description of unconnected network interface state", "Not connected");
            break;
        case Solid::Control::NetworkInterface::Preparing:
            stateString = i18nc("description of preparing to connect network interface state", "Preparing to connect");
            break;
        case Solid::Control::NetworkInterface::Configuring:
            stateString = i18nc("description of configuring hardware network interface state", "Configuring interface");
            break;
        case Solid::Control::NetworkInterface::NeedAuth:
            stateString = i18nc("description of waiting for authentication network interface state", "Waiting for authorization");
            break;
        case Solid::Control::NetworkInterface::IPConfig:
            stateString = i18nc("network interface doing dhcp request in most cases", "Setting network address");
            break;
        case Solid::Control::NetworkInterface::Activated:
            stateString = i18nc("network interface connected state label", "Connected");
            break;
        case Solid::Control::NetworkInterface::Failed:
            stateString = i18nc("network interface connection failed state label", "Connection Failed");
            break;
        default:
            stateString = I18N_NOOP("UNKNOWN STATE FIX ME");
    }
    return stateString;
}

// private functions. Not included in the ToolTipBuilder class.
QString interfaceTooltipHtmlPart(Solid::Control::NetworkInterface *, const QString &);
QString ipv4TooltipHtmlPart(Solid::Control::NetworkInterface *, const QString &);
QString wirelessTooltipHtmlPart(Solid::Control::WirelessNetworkInterface *, const QString &);
QString buildDomainsHtmlTable(const QStringList &);
QString buildNameserversHtmlTable(const QList<quint32> &);

QString ToolTipBuilder::toolTipForInterfaceConnection(Knm::InterfaceConnection * interfaceConnection)
{
    QString tip;

    if (interfaceConnection) {
        QStringList tipElements, allTipElements;
        /* All possible names for tooltip elements
         * Eventually there will be a UI where the user can select which elements to show
         */

        allTipElements << "interface:type" << "interface:name" << "interface:driver" << "interface:status" << "interface:designspeed"
            << "interface:bitrate"
            << "ipv4:address" << "ipv4:nameservers" << "ipv4:domains" << "ipv4:routes"
            << "wired:carrier"
            /* These come from Solid::Control::WirelessNetworkInterface _and_ its active
               Solid::Control::AccessPoint, if any */
            << "wireless:strength" << "wireless:ssid" << "wireless:bitrate" << "wireless:mode" << "wireless:accesspoint" << "wireless:hardwareaddress"
            << /* High level description of security in use */ "wireless:security"
            << /* low level flags so ciphers can be seen*/ "wireless:wpaflags" << "wireless:rsnflags"
            << "wireless:frequency"
            /* These are not yet available in the API!*/
            << "cellular:strength" << "cellular:network";

        /* default set to use for initial development */
        tipElements << "interface:type" << "interface:name" << "interface:driver" << "interface:status"
                    << "interface:designspeed" << "ipv4:address" << "ipv4:nameservers" << "ipv4:domains"
                    << "ipv4:routes" << "wireless:hardwareaddress";

        QString deviceUni = interfaceConnection->deviceUni();
        Solid::Control::NetworkInterface * iface = Solid::Control::NetworkManager::findNetworkInterface(deviceUni);
        Solid::Control::WirelessNetworkInterface * wiface = dynamic_cast<Solid::Control::WirelessNetworkInterface*> (iface);
        if (iface) {
            // generate html table header
            tip = QLatin1String("<qt><table>");

            // iterate each item in tipElements and generate a table row for it
            foreach (QString element, tipElements) {
                const QString prefix = element.section(QLatin1String(":"), 0, 0);
                const QString requestedInfo = element.section(QLatin1String(":"), 1);

                if (prefix == QLatin1String("interface")) {
                    tip += interfaceTooltipHtmlPart(iface, requestedInfo);
                }
                else if (prefix == QLatin1String("ipv4")) {
                    tip += ipv4TooltipHtmlPart(iface, requestedInfo);
                }
                else if (prefix == QLatin1String("wireless")) {
                    if (wiface) {
                        tip += wirelessTooltipHtmlPart(wiface, requestedInfo);
                    }
                }
                else if (prefix == QLatin1String("wired")) {
                    // TODO
                }
            }
            tip += QLatin1String("</table></qt>");
        }
    }
    return tip;
}

QString interfaceTooltipHtmlPart(Solid::Control::NetworkInterface * iface, const QString& requestedInfo)
{
    QString html;

    if (requestedInfo == QLatin1String("name")) {
        html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                       .arg(i18nc("@info:tooltip network device name eg eth0", "Interface"))
                       .arg(iface->interfaceName());
    }
    else if (requestedInfo == QLatin1String("type")) {
        html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                        .arg(i18nc("@info:tooltip interface type", "Type"))
                        .arg(interfaceTypeToString(iface->type()));
    }
    else if (requestedInfo == QLatin1String("driver")) {
        html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                        .arg(i18nc("@info:tooltip system driver name", "Driver"))
                        .arg(iface->driver());
    }
    else if (requestedInfo == QLatin1String("status")) {
        html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                .arg(i18nc("@info:tooltip network interface status", "Status"))
                .arg(connectionStateToString(iface->connectionState()));
    }
    else if (requestedInfo == QLatin1String("designspeed")) {
        html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                .arg(i18nc("@info:tooltip The network device's maximum speed", "Max speed"))
                .arg(iface->designSpeed());
    }
    else if (requestedInfo == QLatin1String("bitrate")) {
        // TODO
        html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                .arg(i18nc("@info:tooltip network connection bit rate","Bit rate"));
    }

    return html;
}

QString ipv4TooltipHtmlPart(Solid::Control::NetworkInterface * iface, const QString& requestedInfo)
{
    QString html;
    QString temp;

    Solid::Control::IPv4Config cfg = iface->ipV4Config();

    if (requestedInfo == QLatin1String("address")) {
        if (!cfg.addresses().isEmpty()) {
            QHostAddress addr(cfg.addresses().first().address());
            temp = addr.toString();
        }
        else temp = QLatin1String("");

        html = QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
               .arg(i18nc("@info:tooltip IPv4 address", "IP address"))
               .arg(temp);
    }
    else if (requestedInfo == QLatin1String("nameservers")) {
        if (!cfg.nameservers().isEmpty()) {
            temp = buildNameserversHtmlTable(cfg.nameservers());
        }
        else temp = QLatin1String("");

        html = QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                .arg(i18nc("@info:tooltip network name servers", "Name servers"))
                .arg(temp);
    }
    else if (requestedInfo == QLatin1String("domains")) {
        if (!cfg.domains().isEmpty()) {
            temp = buildDomainsHtmlTable(cfg.domains());
        }
        else temp = QLatin1String("");

        html = QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                .arg(i18nc("@info:tooltip network domains", "Domains"))
                .arg(temp);
    }
    else if (requestedInfo == QLatin1String("routes")) {
        // TODO
        temp = QLatin1String("");

        html = QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                .arg(i18nc("@info:tooltip network routes", "Routes"))
                .arg(temp);
        //QList<Solid::Control::IPv4Route> routes = cfg.routes();
    }

    return html;
}

QString wirelessTooltipHtmlPart(Solid::Control::WirelessNetworkInterface * wiface,
                                const QString & requestedInfo)
{
    QString html;

    if (requestedInfo == QLatin1String("hardwareaddress")) {
        html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                .arg(i18nc("@info:tooltip this is the hardware address of a network interface",
                           "Hardware address"), wiface->hardwareAddress());
    }

    return html;
}

QString buildDomainsHtmlTable(const QStringList &lst)
{
    QString table = QLatin1String("<table>");

    for (int i = 0, size = lst.size(); i < size; i++) {
        table += QLatin1String("<tr><td>") + lst.at(i)
                 + QLatin1String("</td></tr>");
    }
    table += QLatin1String("</table>");
    return table;
}

QString buildNameserversHtmlTable(const QList<quint32> &lst)
{
    QString table = QLatin1String("<table>");

    for (int i = 0, size = lst.size(); i < size; i++) {
        // nameservers are stored in diffent byte order
        table += QLatin1String("<tr><td>") + QHostAddress(ntohl(lst.at(i))).toString()
                 + QLatin1String("</td></tr>");
    }
    table += QLatin1String("</table>");
    return table;
}

// vim: sw=4 sts=4 et tw=100
