/*
Copyright 2008,2009 Sebastian Kügler <sebas@kde.org>

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

class QPixmap;
class QSizeF;

#include "knm_export.h"

#include <solid/control/networkinterface.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirelessaccesspoint.h>

class KNM_EXPORT UiUtils
{
public:
    /**
     * @param state
     * @return A human-readable string describing the network state
     *
     */
    static QString stateDescription();

    /**
     * @return a human-readable name for the network interface type for use as label
     * @param type the type of the network interface
     */
    static QString descriptiveInterfaceName(const Solid::Control::NetworkInterface::Type type);

    /**
     * @return a human-readable description of the connection state of a given network interface
     * @param state The connection state
     */
    static QString connectionStateToString(int state);

    /**
     * @return an icon name suitable for the interface type
     * @param iface the network interface
     */
    static QString iconName(Solid::Control::NetworkInterface *iface);

    /** This method can be used to retrieve an icon size that fits into a given size.
     * The resulting size can be used to render Pixmaps from KIconLoader without
     * rescaling them (and thereby losing quality)
     *
     * @return a size available in KIconLoader.
     * @param size The size of the rect it should fit in
     */
    static int iconSize(const QSizeF size);

    /** This method can be used to retrieve the progess of a connection attempt
     * as a qreal, for painting progress bars.
     *
     * @return the progress between 0 (disconnected) and 1 (activated).
     * @param interface the network interface
     */
    static qreal interfaceState(const Solid::Control::NetworkInterface *interface);

    static QPixmap interfacePixmap(const QSizeF size, Solid::Control::NetworkInterface *iface);

    /** 
     * @return a human-readable description of operation mode. 
     * @param mode the operation mode
     */
    static QString operationModeToString(Solid::Control::WirelessNetworkInterface::OperationMode mode);
    
    /** 
     * @return string list with a human-readable description of wpa flags. 
     * @param flags the wpa flags
     */
    static QStringList wpaFlagsToStringList(Solid::Control::AccessPoint::WpaFlags flags);

};
#endif // UIUTILS_H
