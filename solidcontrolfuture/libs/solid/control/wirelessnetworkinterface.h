/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>
    Copyright (C) 2011-2013 Lamarque V. Souza <lamarque@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef SOLID_CONTROL_WIRELESSNETWORKINTERFACE_H
#define SOLID_CONTROL_WIRELESSNETWORKINTERFACE_H

#include "solid_control_export.h"

#include "networkinterface.h"

typedef QStringList AccessPointNm09List;
namespace Solid
{
namespace Control
{
    class AccessPointNm09;
    class WirelessNetworkInterfaceNm09Private;
namespace Ifaces
{
    class AccessPointNm09;
}

    /**
     * This interface represents a wireless network interface
     */
    class SOLIDCONTROL_EXPORT WirelessNetworkInterfaceNm09 : public NetworkInterfaceNm09
    {
        Q_OBJECT
        Q_ENUMS(OperationMode DeviceInterface)
        Q_FLAGS(Capabilities)
        Q_DECLARE_PRIVATE(WirelessNetworkInterfaceNm09)

    public:
        enum OperationMode { Unassociated, Adhoc, Managed, Master, Repeater, ApMode };
        // corresponding to 802.11 capabilities defined in NetworkManager.h
        enum Capability { NoCapability = 0x0, Wep40 = 0x1, Wep104 = 0x2, Tkip = 0x4, Ccmp = 0x8, Wpa = 0x10, Rsn = 0x20, ApCap = 0x40, AdhocCap = 0x80 };
        Q_DECLARE_FLAGS(Capabilities, Capability)

        /**
         * Creates a new WirelessNetworkInterface object.
         *
         * @param backendObject the network object provided by the backend
         */
        WirelessNetworkInterfaceNm09(QObject *backendObject = 0);

        /**
         * Constructs a copy of a network.
         *
         * @param network the network to copy
         */
        WirelessNetworkInterfaceNm09(const WirelessNetworkInterfaceNm09 &network);

        /**
         * Destroys a WirelessNetworkInterface object.
         */
        virtual ~WirelessNetworkInterfaceNm09();

        /**
         * The NetworkInterface type.
         *
         * @return the NetworkInterface::Type.  This always returns NetworkInterface::Ieee80211
         */
        virtual NetworkInterfaceNm09::Type type() const;

        /**
         * List of wireless networks currently visible to the hardware
         */
        AccessPointNm09List accessPoints() const;

        /**
         * Identifier of the network this interface is currently associated with
         */
        QString activeAccessPoint() const;

        /**
         * The hardware address assigned to the network interface
         */
        QString hardwareAddress() const;

        /**
         * The pernament hardware address assigned to the network interface
         */
        QString permanentHardwareAddress() const;

        /**
         * Retrieves the operation mode of this network.
         *
         * @return the current mode
         * @see Solid::Control::WirelessNetworkInterface::OperationMode
         */
        Solid::Control::WirelessNetworkInterfaceNm09::OperationMode mode() const;

        /**
         * Retrieves the effective bit rate currently attainable by this device.
         *
         * @return the bitrate in Kbit/s
         */
        int bitRate() const;

        /**
         * Retrieves the capabilities of this wifi network.
         *
         * @return the flag set describing the capabilities
         * @see Solid::Control::WirelessNetworkInterface::DeviceInterface
         */
        Solid::Control::WirelessNetworkInterfaceNm09::Capabilities wirelessCapabilities() const;

        /**
         * Finds access point object given its Unique Network Identifier.
         *
         * @param uni the identifier of the AP to find from this network interface
         * @returns a valid AccessPoint object if a network having the given UNI for this device is known to the system, 0 otherwise
         */
        AccessPointNm09 *findAccessPoint(const QString & uni) const;

    Q_SIGNALS:
        /**
         * This signal is emitted when the bitrate of this network has changed.
         *
         * @param bitrate the new bitrate value for this network
         */
        void bitRateChanged(int);
        /**
         * The active network changed.
         */
        void activeAccessPointChanged(const QString &);
        /**
         * The device switched operating mode.
         */
        void modeChanged(Solid::Control::WirelessNetworkInterfaceNm09::OperationMode);
        /**
         * A new wireless access point appeared
         */
        void accessPointAppeared(const QString &);
        /**
         * A wireless access point disappeared
         */
        void accessPointDisappeared(const QString &);
    protected:
        /**
         * @internal
         */
        WirelessNetworkInterfaceNm09(WirelessNetworkInterfaceNm09Private &dd, QObject *backendObject);

        /**
         * @internal
         */
        WirelessNetworkInterfaceNm09(WirelessNetworkInterfaceNm09Private &dd, const WirelessNetworkInterfaceNm09 &network);

        void makeConnections(QObject * source);
        QPair<AccessPointNm09 *, Ifaces::AccessPointNm09 *> findRegisteredAccessPoint(const QString &uni) const;
    private Q_SLOTS:
        void _k_accessPointAdded(const QString &uni);
        void _k_accessPointRemoved(const QString &uni);
        void _k_destroyed(QObject *object);
    };
} //Control
} //Solid

#endif //SOLID_CONTROL_WIREDNETWORKINTERFACE_H

