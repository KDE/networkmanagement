/*
Copyright 2008-2010 Sebastian Kügler <sebas@kde.org>
Copyright 2011 Will Stephenson <wstephenson@kde.org>

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

#include <libnm-qt/wirelessdevice.h>
#include <libnm-qt/accesspoint.h>

namespace Solid
{
    class Device;
} // namespace Solid

namespace UiUtils
{
    /**
     * @return a human-readable description for the network interface type for use as label
     * @param type the type of the network interface
     */
    KNMINTERNALS_EXPORT QString interfaceTypeLabel(const NetworkManager::Device::Type type);

    /**
     * @return a human-readable name for a given network interface according to the configured
     * naming style
     * @param uni uni of the network interface
     */
    KNMINTERNALS_EXPORT QString interfaceNameLabel(const QString & uni);

    /**
     * @return a human-readable name for a given network interface according to the configured
     * naming style
     * @param uni uni of the network interface
     * @param interfaceNamingStyle name style to use when generating the name
     */
    KNMINTERNALS_EXPORT QString interfaceNameLabel(const QString & uni, const KNetworkManagerServicePrefs::InterfaceNamingChoices interfaceNamingStyle);

    /**
     * @return a Solid::Device from a NetworkManager uni. The returned object must be deleted after use
     * @param type the type of the network interface
     */
    KNMINTERNALS_EXPORT Solid::Device* findSolidDevice(const QString & uni);

    /**
     * @return a human-readable description of the connection state of a given network interface
     * @param state The connection state
     */
    KNMINTERNALS_EXPORT QString connectionStateToString(NetworkManager::Device::State state, const QString &connectionName = QString());

    /**
     * @return an icon name suitable for the interface type
     * @param iface the network interface
     */
    KNMINTERNALS_EXPORT QString iconName(NetworkManager::Device *iface);

    /** This method can be used to retrieve an icon size that fits into a given size.
     * The resulting size can be used to render Pixmaps from KIconLoader without
     * rescaling them (and thereby losing quality)
     *
     * @return a size available in KIconLoader.
     * @param size The size of the rect it should fit in
     */
     KNMINTERNALS_EXPORT int iconSize(const QSizeF size);

    /** This method can be used to retrieve the progress of a connection attempt
     * as a qreal, for painting progress bars.
     *
     * @return the progress between 0 (disconnected) and 1 (activated).
     * @param interface the network interface
     */
    KNMINTERNALS_EXPORT qreal interfaceState(const NetworkManager::Device *interface);

    /**
     * @return a human-readable description of operation mode. 
     * @param mode the operation mode
     */
    KNMINTERNALS_EXPORT QString operationModeToString(NetworkManager::WirelessDevice::OperationMode mode);

    /**
     * @return string list with a human-readable description of wpa flags.
     * @param flags the wpa flags
     */
    KNMINTERNALS_EXPORT QStringList wpaFlagsToStringList(NetworkManager::AccessPoint::WpaFlags flags);

    /**
     * @return localized string showing a human-readable connection speed. 1000 is used as base.
     * @param bitrate bitrate of the connection per second
     */
    KNMINTERNALS_EXPORT QString connectionSpeed(double bitrate);

} //namespace UiItils
#endif // UIUTILS_H
