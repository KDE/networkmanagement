/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#include <solid/control/networkinterface.h>
#include <solid/control/networkipv4config.h>

#include <interfaceconnection.h>

QString ToolTipBuilder::toolTipForInterfaceConnection(Knm::InterfaceConnection *)
{
    QString tip = QLatin1String("<table><tr><td>Hello, world</td></tr></table>");
    QStringList tipElements;
    tipElements << "interface:type" << "interface:name" << "interface:driver" << "interface:status" << "interface:designspeed"
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
    return tip;
}

// vim: sw=4 sts=4 et tw=100
