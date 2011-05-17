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

#include <arpa/inet.h>

#include <kdeversion.h>

#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirednetworkinterface.h>
#include <solid/control/networkipv4confignm09.h>

#include <interfaceconnection.h>
#include <uiutils.h>

#include <KLocale>

#include <knmserviceprefs.h>
#include <wirelesssecurityidentifier.h>

// private functions. Not included in the ToolTipBuilder class.
QString interfaceTooltipHtmlPart(Solid::Control::NetworkInterface *, const QString &);
QString ipv4TooltipHtmlPart(Solid::Control::NetworkInterface *, const QString &);
QString wirelessTooltipHtmlPart(Solid::Control::WirelessNetworkInterface *, const QString &);
QString wiredTooltipHtmlPart(Solid::Control::WiredNetworkInterface * , const QString &);
QString buildFlagsHtmlTable(Solid::Control::AccessPoint::WpaFlags);
QString buildRoutesHtmlTable(const QList<Solid::Control::IPv4Route> &);
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

        allTipElements << "interface:type" << "interface:name" << "interface:driver" << "interface:status"
            << "interface:designspeed" << "interface:hardwareaddress" << "interface:bitrate"
            << "ipv4:address" << "ipv4:nameservers" << "ipv4:domains" << "ipv4:routes"
            << "wired:carrier"
            /* These come from Solid::Control::WirelessNetworkInterface _and_ its active
               Solid::Control::AccessPoint, if any */
            << "wireless:strength" << "wireless:ssid" << "wireless:mode" << "wireless:accesspoint"
            << /* High level description of security in use */ "wireless:security"
            << /* low level flags so ciphers can be seen*/ "wireless:wpaflags" << "wireless:rsnflags"
            << "wireless:frequency"
            /* These are not yet available in the API!*/
            << "cellular:strength" << "cellular:network";

        tipElements = KNetworkManagerServicePrefs::self()->toolTipKeys();

        QString deviceUni = interfaceConnection->deviceUni();
        Solid::Control::NetworkInterface * iface = Solid::Control::NetworkManager::findNetworkInterface(deviceUni);
        Solid::Control::WirelessNetworkInterface * wliface = dynamic_cast<Solid::Control::WirelessNetworkInterface*> (iface);
        Solid::Control::WiredNetworkInterface * wdiface = dynamic_cast<Solid::Control::WiredNetworkInterface*> (iface);

        if (iface) {
            // generate html table header
            tip = QLatin1String("<qt><table>");

            // iterate each item in tipElements and generate a table row for it
            foreach (const QString &element, tipElements) {
                const QString prefix = element.section(QLatin1String(":"), 0, 0);
                const QString requestedInfo = element.section(QLatin1String(":"), 1);

                if (prefix == QLatin1String("interface")) {
                    tip += interfaceTooltipHtmlPart(iface, requestedInfo);
                }
                else if (prefix == QLatin1String("ipv4")) {
                    tip += ipv4TooltipHtmlPart(iface, requestedInfo);
                }
                else if (prefix == QLatin1String("wireless")) {
                    if (wliface) {
                        tip += wirelessTooltipHtmlPart(wliface, requestedInfo);
                    }
                }
                else if (prefix == QLatin1String("wired")) {
                    if (wdiface) {
                        tip += wiredTooltipHtmlPart(wdiface, requestedInfo);
                    }
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
                        .arg(UiUtils::interfaceTypeLabel(iface->type()));
    }
    else if (requestedInfo == QLatin1String("driver")) {
        html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                        .arg(i18nc("@info:tooltip system driver name", "Driver"))
                        .arg(iface->driver());
    }
    else if (requestedInfo == QLatin1String("status")) {
        html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                .arg(i18nc("@info:tooltip network interface status", "Status"))
                .arg(UiUtils::connectionStateToString(static_cast<Solid::Control::NetworkInterfaceNm09::ConnectionState>(iface->connectionState())));
    }
    else if (requestedInfo == QLatin1String("designspeed")) {
        html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                .arg(i18nc("@info:tooltip The network device's maximum speed", "Max speed"))
                .arg(iface->designSpeed());
    }
    else if (requestedInfo == QLatin1String("hardwareaddress")) {
        QString temp;

        Solid::Control::WirelessNetworkInterface * wliface = dynamic_cast<Solid::Control::WirelessNetworkInterface*> (iface);

        if (wliface) {
            temp = wliface->hardwareAddress();
        }
        else {
            Solid::Control::WiredNetworkInterface * wdiface = dynamic_cast<Solid::Control::WiredNetworkInterface*> (iface);
            if (wdiface) {
                temp = wdiface->hardwareAddress();
            }
        }

        if (!temp.isEmpty()) {
            html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                .arg(i18nc("@info:tooltip this is the hardware address of a network interface",
                            "Hardware address"), temp);
        }
    }
    else if (requestedInfo == QLatin1String("bitrate")) {
        int bitRate = 0;

        Solid::Control::WirelessNetworkInterface * wliface = dynamic_cast<Solid::Control::WirelessNetworkInterface*> (iface);

        if (wliface) {
            bitRate = wliface->bitRate() / 1000;
        }
        else {
            Solid::Control::WiredNetworkInterface * wdiface = dynamic_cast<Solid::Control::WiredNetworkInterface*> (iface);
            if (wdiface) {
                bitRate = wdiface->bitRate() / 1000;
            }
        }

        if (bitRate) {
        html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2 %3</td></tr>")
                .arg(i18nc("@info:tooltip network connection bit rate","Bit rate"))
                .arg(bitRate)
                .arg(i18nc("@info:tooltip network connection bit rate units","Mbit/s"));
        }
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
        else temp = QLatin1String(""); // or something like 'i18nc("@info:tooltip No data available.","No data");'

        html = QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
               .arg(i18nc("@info:tooltip IPv4 address", "IP address"))
               .arg(temp);
    }
    else if (requestedInfo == QLatin1String("nameservers")) {
        if (!cfg.nameservers().isEmpty()) {
            temp = buildNameserversHtmlTable(cfg.nameservers());
        }
        else temp = QLatin1String(""); // or something like 'i18nc("@info:tooltip No data available.","No data");'

        html = QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                .arg(i18nc("@info:tooltip network name servers", "Name servers"))
                .arg(temp);
    }
    else if (requestedInfo == QLatin1String("domains")) {
        if (!cfg.domains().isEmpty()) {
            temp = buildDomainsHtmlTable(cfg.domains());
        }
        else temp = QLatin1String(""); // or something like 'i18nc("@info:tooltip No data available.","No data");'

        html = QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                .arg(i18nc("@info:tooltip network domains", "Domains"))
                .arg(temp);
    }
    else if (requestedInfo == QLatin1String("routes")) {
#if KDE_IS_VERSION(4, 2, 95)
        QList<Solid::Control::IPv4Route> routes = cfg.routes();

        if (!routes.isEmpty()) {
            temp = buildRoutesHtmlTable(routes);
        }
        else temp = i18nc("@info:tooltip No network route data available", "No route data available");
#else
        temp = i18nc("@info:tooltip No network route data available", "No route data available");
#endif

        html = QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                .arg(i18nc("@info:tooltip network routes", "Routes"))
                .arg(temp);
    }

    return html;
}

QString wirelessTooltipHtmlPart(Solid::Control::WirelessNetworkInterface * wiface,
                                const QString & requestedInfo)
{
    QString html;
    QString temp;

    Solid::Control::AccessPoint * ap = wiface->findAccessPoint(wiface->activeAccessPoint());

    if (requestedInfo == QLatin1String("ssid")) {
        if (ap) {
            temp = ap->ssid();
        }
        else temp = ""; // or something like 'i18nc("@info:tooltip No data available.","No data");'

        html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                .arg(i18nc("@info:tooltip SSID is a friendly name that identifies a 802.11 WLAN.","SSID"))
                .arg(temp);
    }
    else if (requestedInfo == QLatin1String("strength")) {
        if (ap) {
            temp.setNum(ap->signalStrength());
        }
        else temp = "";

        html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2 %</td></tr>")
                .arg(i18nc("@info:tooltip The signal strength of the wifi network","Strength"))
                .arg(temp);
    }
    else if (requestedInfo == QLatin1String("mode")) {
        html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                .arg(i18nc("@info:tooltip the operation mode of wi-fi network","Mode"))
                .arg(UiUtils::operationModeToString(wiface->mode()));
    }
    else if (requestedInfo == QLatin1String("accesspoint")) {
        if (ap) {
            html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                .arg(i18nc("@info:tooltip Active access point MAC address","Access point"))
                .arg(ap->hardwareAddress());
        }
    }
    else if (requestedInfo == QLatin1String("security")) {
        if (ap) {
            Knm::WirelessSecurity::Type best = Knm::WirelessSecurity::best(wiface->wirelessCapabilities(), true, (wiface->mode() == Solid::Control::WirelessNetworkInterface::Adhoc), ap->capabilities(), ap->wpaFlags(), ap->rsnFlags());
            temp = Knm::WirelessSecurity::shortToolTip(best);
        }
        else temp = "";

        html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                .arg(i18nc("@info:tooltip network security level, e.g. high, low", "Security"))
                .arg(temp);
    }
    else if (requestedInfo == QLatin1String("wpaflags")) {
        if (ap) {
            temp = buildFlagsHtmlTable(ap->wpaFlags());
        }
        else temp = "";

        html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                .arg(i18nc("@info:tooltip Flags describing the access point's capabilities according to WPA (Wifi Protected Access)",
                           "WPA flags"))
                .arg(temp);
    }
    else if (requestedInfo == QLatin1String("rsnflags")) {
        if (ap) {
            temp = buildFlagsHtmlTable(ap->rsnFlags());
        }
        else temp = "";

        html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                .arg(i18nc("@info:tooltip Flags describing the access point's capabilities according to RSN (Robust Secure Network)",
                           "RSN(WPA2) flags"))
                .arg(temp);
    }
    else if (requestedInfo == QLatin1String("frequency")) {
        if (ap) {
            temp.setNum(ap->frequency());
        }
        else temp = "";

        html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2 %3</td></tr>")
                .arg(i18nc("@info:tooltip the frequency of the radio channel that the access point is operating on","Frequency"))
                .arg(wiface->bitRate())
                .arg(i18nc("@info:tooltip frequency unit","MHz"));
    }

    return html;
}

QString wiredTooltipHtmlPart(Solid::Control::WiredNetworkInterface * wdiface,
                                const QString & requestedInfo)
{
    QString html;

    if (requestedInfo == QLatin1String("carrier")) {
        html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                .arg(i18nc("@info:tooltip carrier",
                           "Carrier"), (wdiface->carrier()
                               ? i18nc("@info:tooltip network interface status - carrier found", "Found")
                               : i18nc("@info:tooltip network interface status - carrier not found", "Not Found"))
                        );
    }

    return html;
}

QString buildHtmlTableHelper(const QStringList& list, int nItemsInRow)
{
    Q_ASSERT(nItemsInRow);

    QString temp;

    if (nItemsInRow) {
        int nRows = list.size() / nItemsInRow;
        if ((list.size() % nItemsInRow) > 0)
            nRows++;

        for(int i = 0; i < nRows; i++) {
            temp += QLatin1String("<tr><td>");
            for (int j = 0; j < nItemsInRow; j++) {
                temp += list[i*nItemsInRow + j];

                // check for the last item, in this case we will not put the comma sign.
                if ((i*nItemsInRow + j) < (list.size() - 1))
                    temp += QLatin1String(", ");
                else
                    break; // the last item
            }
            temp += QLatin1String("</td></tr>");
        }
    }
    return temp;
}

QString buildFlagsHtmlTable(Solid::Control::AccessPoint::WpaFlags flags)
{
    QString table = QLatin1String("<table>");

    table += buildHtmlTableHelper(UiUtils::wpaFlagsToStringList(flags), 2);
    table += QLatin1String("</table>");

    return table;
}

QString buildRoutesHtmlTable(const QList<Solid::Control::IPv4Route> &lst)
{
    QString table = QLatin1String("<table>");

    foreach(const Solid::Control::IPv4Route &route, lst) {
        table += QString("<tr><td>%1/%2 %3 %4</td></tr>")
                 .arg(QHostAddress(route.route()).toString())
                 .arg(route.prefix())
                 .arg(QHostAddress(route.nextHop()).toString())
                 .arg(route.metric());

    }
    table += QLatin1String("</table>");

    //qDebug() << table;

    return table;
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
