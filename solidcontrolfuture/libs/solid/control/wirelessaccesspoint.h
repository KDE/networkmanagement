/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public 
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SOLID_CONTROL_WIRELESSAP_H
#define SOLID_CONTROL_WIRELESSAP_H

#include <QtCore/QObject>

#include "wirelessnetworkinterface.h"

namespace Solid
{
namespace Control
{
    class AccessPointNm09Private;
    class SOLIDCONTROL_EXPORT AccessPointNm09 : public QObject
    {
        Q_OBJECT
        Q_DECLARE_PRIVATE(AccessPointNm09)
        Q_FLAGS(Capabilities WpaFlags)
    public:
        /**
         * General capabilities of an access point
         */
        enum Capability { Privacy = 0x1 };
        /**
         * Flags describing the access point's capabilities according to WPA (Wifi Protected Access)
         */
        enum WpaFlag { PairWep40 = 0x1, PairWep104 = 0x2, PairTkip = 0x4, PairCcmp = 0x8,
           GroupWep40 = 0x10, GroupWep104 = 0x20, GroupTkip = 0x40, GroupCcmp = 0x80,
           KeyMgmtPsk = 0x100, KeyMgmt8021x = 0x200 };
        Q_DECLARE_FLAGS(Capabilities, Capability)
        Q_DECLARE_FLAGS(WpaFlags, WpaFlag)

        /**
         * Creates a new Network object.
         *
         * @param backendObject the network object provided by the backend
         */
        AccessPointNm09(QObject *backendObject = 0);
        /**
         * Copy ctor
         */
        AccessPointNm09(const AccessPointNm09 &);

        virtual ~AccessPointNm09();

        /**
         * Opaque and unique string identifier for this access point
         */
        QString uni() const;

        /**
         * Flags describing the capabilities of the access point
         */
        Solid::Control::AccessPointNm09::Capabilities capabilities() const;

        /**
         * Flags describing the access point's capabilities according to WPA (Wifi Protected Access)
         */
        Solid::Control::AccessPointNm09::WpaFlags wpaFlags() const;

        /**
         * Flags describing the access point's capabilities according to RSN (Robust Secure Network)
         * aka WPA2
         */
        Solid::Control::AccessPointNm09::WpaFlags rsnFlags() const;

        /**
         * The Service Set Identifier of the access point
         */
        QString ssid() const;

        /**
         * The raw bytes of the Service Set Identifier of the access point
         */
        QByteArray rawSsid() const;

        /**
         * Retrieves the frequency in MHz of the radio channel that this access point is operating on
         *
         * @return the frequency
         */
        uint frequency() const;

        /**
         * The hardware address assigned to the access point's wireless interface
         */
        QString hardwareAddress() const;

        /**
         * Retrieves the maximum bit rate currently attainable with this access point
         *
         * @return the maximum bitrate in kilobit/s
         */
        uint maxBitRate() const;

        /**
         * Retrieves the operation mode of this access point
         *
         * @return the current mode
         * @see Solid::Control::WirelessNetworkInterface::OperationMode
         */
        Solid::Control::WirelessNetworkInterfaceNm09::OperationMode mode() const;

        /**
         * Retrieves the current signal strength of this access point
         *
         * @return the signal strength as a percentage
         */
        int signalStrength() const;

    Q_SIGNALS:
        /**
         * This signal is emitted when the signal strength of this access point has changed.
         *
         * @param strength the new signal strength value for this access point.
         */
        void signalStrengthChanged(int strength);

        /**
         * This signal is emitted when the bitrate of this access point has changed.
         *
         * @param bitrate the new bitrate value for this access point
         */
        void bitRateChanged(int bitrate);

        /**
         * This signal is emitted when the WPA flags in use by this access point change
         *
         * @param flags the new flags
         */
        void wpaFlagsChanged(Solid::Control::AccessPointNm09::WpaFlags flags) const;

        /**
         * This signal is emitted when the RSN(WPA2) flags in use by this access point change
         *
         * @param flags the new flags
         */
        void rsnFlagsChanged(Solid::Control::AccessPointNm09::WpaFlags flags) const;
        /**
         * This signal is emitted when the ssid of this Access Point changes
         *
         * @param ssid the new SSID
         */
        void ssidChanged(const QString & ssid) const;

        /**
         * This signal is emitted when the frequency used by this Access Point changes
         *
         * @param frequency the new frequency
         */
        void frequencyChanged(uint frequency) const;
    protected:
        /**
         * @internal
         */
        AccessPointNm09(AccessPointNm09Private &dd, QObject *backendObject);
        /**
         * @internal
         */
        AccessPointNm09(AccessPointNm09Private &dd, const AccessPointNm09 & ap);
        AccessPointNm09Private * d_ptr;
    private Q_SLOTS:
        void _k_destroyed(QObject *object);
    };
    typedef QStringList AccessPointNm09List;
} // Control
} // Solid

Q_DECLARE_OPERATORS_FOR_FLAGS(Solid::Control::AccessPointNm09::Capabilities)
Q_DECLARE_OPERATORS_FOR_FLAGS(Solid::Control::AccessPointNm09::WpaFlags)
#endif // SOLID_CONTROL_WIRELESSAP_H

