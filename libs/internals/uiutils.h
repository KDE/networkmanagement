/*
Copyright 2008-2010 Sebastian Kügler <sebas@kde.org>

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

#ifndef UIUTILS_H
#define UIUTILS_H

class QSizeF;

#include "knmserviceprefs.h"
#include "knminternals_export.h"
#include "../client/remoteinterfaceconnection.h"
#include "../client/remoteactivatablelist.h"
#include "../libs/types.h"

#include <solid/control/networkinterface.h>
#include <solid/control/networkmodeminterface.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirelessaccesspoint.h>
#include <Solid/Device>

class KNMINTERNALS_EXPORT UiUtils
{
public:

    /**
     * @return a human-readable description for the network interface type for use as label
     * @param type the type of the network interface
     */
    static QString interfaceTypeLabel(const Solid::Control::NetworkInterfaceNm09::Type type, const Solid::Control::NetworkInterfaceNm09 * iface);

    /**
     * @return a human-readable name for a given network interface according to the configured
     * naming style
     * @param uni uni of the network interface
     */
    static QString interfaceNameLabel(const QString & uni);

    /**
     * @return a human-readable name for a given network interface according to the configured
     * naming style
     * @param uni uni of the network interface
     * @param interfaceNamingStyle name style to use when generating the name
     */
    static QString interfaceNameLabel(const QString & uni, const KNetworkManagerServicePrefs::InterfaceNamingChoices interfaceNamingStyle);

    /**
     * @return a Solid::Device from a NetworkManager uni. The returned object must be deleted after use
     * @param type the type of the network interface
     */
    static Solid::Device* findSolidDevice(const QString & uni);

    /**
     * @return a human-readable description of the connection state of a given network interface
     * @param state The connection state
     */
    static QString connectionStateToString(Solid::Control::NetworkInterfaceNm09::ConnectionState state, const QString &connectionName = QString());

    /**
     * @return a human-readable description of the connection state of a given interface connection
     * @param state The connection state
     */
    static QString connectionStateToString(Knm::InterfaceConnection::ActivationState state, const QString &connectionName = QString());

    /**
     * @return an icon name suitable for the interface type
     * @param iface the network interface
     */
    static QString iconName(Solid::Control::NetworkInterfaceNm09 *iface);

    /** This method can be used to retrieve an icon size that fits into a given size.
     * The resulting size can be used to render Pixmaps from KIconLoader without
     * rescaling them (and thereby losing quality)
     *
     * @return a size available in KIconLoader.
     * @param size The size of the rect it should fit in
     */
    static int iconSize(const QSizeF size);

    /** This method can be used to retrieve the progress of a connection attempt
     * as a qreal, for painting progress bars.
     *
     * @return the progress between 0 (disconnected) and 1 (activated).
     * @param interface the network interface
     */
    static qreal interfaceState(const Solid::Control::NetworkInterfaceNm09 *interface);

    /**
     * @return a human-readable description of operation mode.
     * @param mode the operation mode
     */
    static QString operationModeToString(Solid::Control::WirelessNetworkInterfaceNm09::OperationMode mode);

    /**
     * @return string list with a human-readable description of wpa flags.
     * @param flags the wpa flags
     */
    static QStringList wpaFlagsToStringList(Solid::Control::AccessPointNm09::WpaFlags flags);

    /**
     * @return localized string showing a human-readable connection speed. 1000 is used as base.
     * @param bitrate bitrate of the connection per second
     */
    static QString connectionSpeed(double bitrate);

    /**
     * @return String representation of a mac address.
     * @param ba byte array containing the binary repesentation of the address
     */
    static QString macAddressAsString(const QByteArray & ba);

    /**
     * @return binary repesentation of a mac address.
     * @param s string representation of the address
     */
    static QByteArray macAddressFromString( const QString & s);

    /**
     * @param freq frequency of a wireless network
     * @return The frequency translated into band (first element of the QPair) and channel. The band value is
     * corresponding to the type enum in Knm::WirelessSetting::EnumBand
     */
    static QPair<int, int> findBandAndChannel(int freq);

    /**
     * @param band The band of a wireless network. The value corresponds to the type enum in Knm::WirelessSetting::EnumBand
     * @return A string representation
     */
    static QString wirelessBandToString(int band);

};
#endif // UIUTILS_H
