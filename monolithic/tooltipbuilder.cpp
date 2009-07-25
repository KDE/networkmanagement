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

#include <kdeversion.h>

#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirednetworkinterface.h>
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

QString operationModeToString(Solid::Control::WirelessNetworkInterface::OperationMode mode)
{
    QString modeString;
    switch (mode) {
        case Solid::Control::WirelessNetworkInterface::Unassociated:
            modeString = i18nc("wireless network operation mode", "Unassociated");
            break;
        case Solid::Control::WirelessNetworkInterface::Adhoc:
            modeString = i18nc("wireless network operation mode", "Adhoc");
            break;
        case Solid::Control::WirelessNetworkInterface::Managed:
            modeString = i18nc("wireless network operation mode", "Managed");
            break;
        case Solid::Control::WirelessNetworkInterface::Master:
            modeString = i18nc("wireless network operation mode", "Master");
            break;
        case Solid::Control::WirelessNetworkInterface::Repeater:
            modeString = i18nc("wireless network operation mode", "Repeater");
            break;
        default:
            modeString = I18N_NOOP("UNKNOWN MODE FIX ME");
    }
    return modeString;
}

// private functions. Not included in the ToolTipBuilder class.
QString interfaceTooltipHtmlPart(Solid::Control::NetworkInterface *, const QString &);
QString ipv4TooltipHtmlPart(Solid::Control::NetworkInterface *, const QString &);
QString wirelessTooltipHtmlPart(Solid::Control::WirelessNetworkInterface *, const QString &);
QString wiredTooltipHtmlPart(Solid::Control::WiredNetworkInterface * , const QString &);
QString securityLevel(Solid::Control::AccessPoint *);
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

        allTipElements << "interface:type" << "interface:name" << "interface:driver" << "interface:status" << "interface:designspeed"
            << "ipv4:address" << "ipv4:nameservers" << "ipv4:domains" << "ipv4:routes"
            << "wired:carrier" << "wired:hardwareaddress" << "wired:bitrate"
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
                    << "ipv4:routes" << "wired:hardwareaddress" << "wireless:hardwareaddress"
                    << "wired:bitrate" << "wireless:bitrate";

        QString deviceUni = interfaceConnection->deviceUni();
        Solid::Control::NetworkInterface * iface = Solid::Control::NetworkManager::findNetworkInterface(deviceUni);
        Solid::Control::WirelessNetworkInterface * wliface = dynamic_cast<Solid::Control::WirelessNetworkInterface*> (iface);
        Solid::Control::WiredNetworkInterface * wdiface = dynamic_cast<Solid::Control::WiredNetworkInterface*> (iface);

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

        /* PLEASE REMOVE THIS CODE
        I dont have wireless connection, so check 'buildFlagsHtmlTable' this way
        Solid::Control::AccessPoint::WpaFlags flags;
        temp = buildFlagsHtmlTable(flags);
        */
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

    if (requestedInfo == QLatin1String("hardwareaddress")) {
        html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                .arg(i18nc("@info:tooltip this is the hardware address of a network interface",
                           "Hardware address"), wiface->hardwareAddress());
    }
    else if (requestedInfo == QLatin1String("bitrate")) {
        html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2 %3</td></tr>")
                .arg(i18nc("@info:tooltip network connection bit rate","Bit rate"))
                .arg(wiface->bitRate())
                .arg(i18nc("@info:tooltip network connection bit rate units","Mbit/s"));
    }
    else if (requestedInfo == QLatin1String("ssid")) {
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
                .arg(operationModeToString(wiface->mode()));
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
            temp = securityLevel(ap);
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

    if (requestedInfo == QLatin1String("hardwareaddress")) {
        html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2</td></tr>")
                .arg(i18nc("@info:tooltip this is the hardware address of a network interface",
                           "Hardware address"), wdiface->hardwareAddress());
    }
    else if (requestedInfo == QLatin1String("bitrate")) {
        html += QString("<tr><td><b>%1:</b></td><td>&nbsp;%2 %3</td></tr>")
                .arg(i18nc("@info:tooltip network connection bit rate","Bit rate"))
                .arg(wdiface->bitRate())
                .arg(i18nc("@info:tooltip network connection bit rate units","Mbit/s"));
    }

    return html;
}

// copied from WirelessStatus::setSecurity()
QString securityLevel(Solid::Control::AccessPoint * ap)
{
    Q_ASSERT(ap);

    QString level;
    Solid::Control::AccessPoint::WpaFlags wpaFlags = ap->wpaFlags();
    Solid::Control::AccessPoint::WpaFlags rsnFlags = ap->rsnFlags();
    Solid::Control::AccessPoint::Capabilities capabilities = ap->capabilities();

    if (wpaFlags.testFlag( Solid::Control::AccessPoint::PairWep40 ) ||
        wpaFlags.testFlag( Solid::Control::AccessPoint::PairWep104 ) ||
        (wpaFlags == 0 && capabilities.testFlag(Solid::Control::AccessPoint::Privacy))) {

        level = i18nc("@info:tooltip security level", "WEP");
    }
    else if (wpaFlags.testFlag( Solid::Control::AccessPoint::KeyMgmtPsk ) ||
             wpaFlags.testFlag( Solid::Control::AccessPoint::PairTkip ) ) {
        level = i18nc("@info:tooltip security level", "WPA-PSK");
    }
    else if (rsnFlags.testFlag( Solid::Control::AccessPoint::KeyMgmtPsk ) ||
             rsnFlags.testFlag( Solid::Control::AccessPoint::PairTkip ) ||
             rsnFlags.testFlag( Solid::Control::AccessPoint::PairCcmp ) ) {
        level = i18nc("@info:tooltip security level", "WPA-PSK");
    }
    else if (wpaFlags.testFlag( Solid::Control::AccessPoint::KeyMgmt8021x ) ||
             wpaFlags.testFlag( Solid::Control::AccessPoint::GroupCcmp ) ) {
        level = i18nc("@info:tooltip security level", "WPA-EAP");
    }
    else {
        level = i18nc("@info:tooltip security level", "Unencrypted");
    }
    return level;
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

    /* for testing purposes
    flags = Solid::Control::AccessPoint::PairWep40
            | Solid::Control::AccessPoint::PairWep104
            | Solid::Control::AccessPoint::PairTkip
            | Solid::Control::AccessPoint::PairCcmp
            | Solid::Control::AccessPoint::GroupWep40
            | Solid::Control::AccessPoint::GroupWep104
            | Solid::Control::AccessPoint::GroupTkip
            | Solid::Control::AccessPoint::GroupCcmp
            | Solid::Control::AccessPoint::KeyMgmtPsk
            | Solid::Control::AccessPoint::KeyMgmt8021x; */

    QStringList flagList;

    if (flags.testFlag(Solid::Control::AccessPoint::PairWep40))
        flagList.append(QLatin1String("Pairwise WEP40"));
    if (flags.testFlag(Solid::Control::AccessPoint::PairWep104))
        flagList.append(QLatin1String("Pairwise WEP104"));
    if (flags.testFlag(Solid::Control::AccessPoint::PairTkip))
        flagList.append(QLatin1String("Pairwise TKIP"));
    if (flags.testFlag(Solid::Control::AccessPoint::PairCcmp))
        flagList.append(QLatin1String("Pairwise CCMP"));
    if (flags.testFlag(Solid::Control::AccessPoint::GroupWep40))
        flagList.append(QLatin1String("Group WEP40"));
    if (flags.testFlag(Solid::Control::AccessPoint::GroupWep104))
        flagList.append(QLatin1String("Group WEP104"));
    if (flags.testFlag(Solid::Control::AccessPoint::GroupTkip))
        flagList.append(QLatin1String("Group TKIP"));
    if (flags.testFlag(Solid::Control::AccessPoint::GroupCcmp))
        flagList.append(QLatin1String("Group CCMP"));
    if (flags.testFlag(Solid::Control::AccessPoint::KeyMgmtPsk))
        flagList.append(QLatin1String("PSK"));
    if (flags.testFlag(Solid::Control::AccessPoint::KeyMgmt8021x))
        flagList.append(QLatin1String("802.1x"));

    table += buildHtmlTableHelper(flagList, 2);
    table += QLatin1String("</table>");

    return table;
}

QString buildRoutesHtmlTable(const QList<Solid::Control::IPv4Route> &lst)
{
    QString table = QLatin1String("<table>");

    foreach(Solid::Control::IPv4Route route, lst) {
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
