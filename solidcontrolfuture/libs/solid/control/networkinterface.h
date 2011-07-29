/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_CONTROL_NETWORKINTERFACE_H
#define SOLID_CONTROL_NETWORKINTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtNetwork/QNetworkAddressEntry>

#include "solid_control_export.h"

#include "networkipv4confignm09.h"

namespace Solid
{
namespace Control
{
    class NetworkInterfaceNm09Private;

    /**
     * This interface represents a generic network interface in the system
     */
    class SOLIDCONTROL_EXPORT NetworkInterfaceNm09 : public QObject
    {
    Q_OBJECT
    Q_ENUMS(ConnectionState Capability Type)
    Q_FLAGS(Capabilities)
    Q_FLAGS(Types)
    Q_DECLARE_PRIVATE(NetworkInterfaceNm09)

    public:
        // == NM ActivationStage
        /**
         * Device connection states describe the possible states of a
         * network connection from the user's point of view.  For
         * simplicity, states from several different layers are present -
         * this is a high level view
         */
        enum ConnectionState {
            UnknownState = 0   /* NM_DEVICE_STATE_UNKNOWN */,
            Unmanaged    = 10  /* NM_DEVICE_STATE_UNMANAGED */,
            Unavailable  = 20  /* NM_DEVICE_STATE_UNAVAILABLE */,
            Disconnected = 30  /* NM_DEVICE_STATE_DISCONNECTED */,
            Preparing    = 40  /* NM_DEVICE_STATE_PREPARE */,
            Configuring  = 50  /* NM_DEVICE_STATE_CONFIG */,
            NeedAuth     = 60  /* NM_DEVICE_STATE_NEED_AUTH */,
            IPConfig     = 70  /* NM_DEVICE_STATE_IP_CONFIG */,
            IPCheck      = 80  /* NM_DEVICE_STATE_IP_CHECK */,
            Secondaries  = 90  /* NM_DEVICE_STATE_SECONDARIES */,
            Activated    = 100 /* NM_DEVICE_STATE_ACTIVATED */,
            Deactivating = 110 /* NM_DEVICE_STATE_DEACTIVATING */,
            Failed       = 120 /* NM_DEVICE_STATE_FAILED */
        };
        /**
         * Enums describing the reason for a connection state change
         */
        enum ConnectionStateChangeReason{ NoReason=0, UnknownReason=1, NowManagedReason=2, NowUnmanagedReason=3,
                                    ConfigFailedReason=4, ConfigUnavailableReason=5,
                                    ConfigExpiredReason=6, NoSecretsReason=7, AuthSupplicantDisconnectReason=8,
                                    AuthSupplicantConfigFailedReason=9, AuthSupplicantFailedReason=10,
                                    AuthSupplicantTimeoutReason=11, PppStartFailedReason=12, PppDisconnectReason=13,
                                    PppFailedReason=14, DhcpStartFailedReason=15, DhcpErrorReason=16, DhcpFailedReason=17,
                                    SharedStartFailedReason=18, SharedFailedReason=19,
                                    AutoIpStartFailedReason=20, AutoIpErrorReason=21, AutoIpFailedReason=22,
                                    ModemBusyReason=23, ModemNoDialToneReason=24, ModemNoCarrierReason=25, ModemDialTimeoutReason=26,
                                    ModemDialFailedReason=27, ModemInitFailedReason=28,
                                    GsmApnSelectFailedReason=29, GsmNotSearchingReason=30, GsmRegistrationDeniedReason=31,
                                    GsmRegistrationTimeoutReason=32, GsmRegistrationFailedReason=33,
                                    GsmPinCheckFailedReason=34, FirmwareMissingReason=35, DeviceRemovedReason=36,
                                    SleepingReason=37, ConnectionRemovedReason=38, UserRequestedReason=39, CarrierReason=40,
				    ConnectionAssumedReason=41, AuthSupplicantAvailableReason=42, ModemNotFoundReason=43, BluetoothFailedReason=44,
                                    Reserved = 65536 };
        /**
         * Possible Device capabilities
         * - IsManageable: denotes that the device can be controlled by this API
         * - SupportsCarrierDetect: the device informs us when it is plugged in to the medium
         */
        enum Capability { IsManageable = 0x1, SupportsCarrierDetect = 0x2 };
        /**
         * Device medium types
         * - UnknownType: unknown device
         * - Ethernet: a wired ethernet device
         * - Wifi: an 802.11 WiFi device
         * - Unused1: not used
         * - Unused2: not used
         * - Bluetooth: a Bluetooth device supporting PAN or DUN access protocols
         * - OlpcMesh: an OLPC XO mesh networking device
         * - Wimax: an 802.16e Mobile WiMAX broadband device
         * - Modem: a modem supporting analog telephone, CDMA/EVDO, GSM/UMTS, or LTE network access protocols
         */
        enum Type { UnknownType = 0x0, Ethernet = 0x1, Wifi = 0x2, Unused1 = 0x3, Unused2 = 0x4, Bluetooth = 0x5, OlpcMesh = 0x6, Wimax = 0x7, Modem = 0x8 };

        Q_DECLARE_FLAGS(Capabilities, Capability)
        Q_DECLARE_FLAGS(Types, Type)

        /**
         * Creates a new NetworkInterface object.
         *
         * @param backendObject the network object provided by the backend
         */
        explicit NetworkInterfaceNm09(QObject *backendObject);

        /**
         * Constructs a copy of a network.
         *
         * @param network the network to copy
         */
        NetworkInterfaceNm09(const NetworkInterfaceNm09 &network);

        /**
         * Destroys a NetworkInterface object.
         */
        virtual ~NetworkInterfaceNm09();

        /**
         * Retrieves the interface type.  This is a virtual function that will return the
         * proper type of all sub-classes.
         *
         * @returns the NetworkInterface::Type that corresponds to this device.
         */
        virtual Type type() const;

        /**
         * Retrieves the Unique Network Identifier (UNI) of the NetworkInterface.
         * This identifier is unique for each network and network interface in the system.
         *
         * @returns the Unique Network Identifier of the current network
         */
        QString uni() const;

        /**
         * Retrieves the Unique Device Identifier (UDI).
         * This is needed for ModemManager support.
         *
         * @returns the Unique Device Identifier of the current device
         */
        QString udi() const;

        /**
         * The system name for the network interface
         */
        QString interfaceName() const;

        /**
         * The name of the device's data interface when available. This property
         * may not refer to the actual data interface until the device has
         * successfully established a data connection, indicated by the device's
         * State becoming ACTIVATED.
         */
        QString ipInterfaceName() const;

        /**
         * If TRUE, indicates the device is likely missing firmware necessary for
         * its operation.
         */
        bool firmwareMissing() const;

        /**
         * Current active connection.
         */
        QString activeConnection() const;

        /**
         * Handle for the system driver controlling this network interface
         */
        QString driver() const;

        /**
         * Disconnects a device and prevents the device from automatically
         * activating further connections without user intervention.
         */
        void disconnectInterface() const;

        Solid::Control::IPv4ConfigNm09 ipV4Config() const;
        /**
         * Retrieves the activation status of this network interface.
         *
         * @return true if this network interface is active, false otherwise
         */
        bool isActive() const;

        /**
         * Retrieves the current state of the network connection held by this device.
         * It's a high level view of the connection. It is user oriented, so
         * actually it provides state coming from different layers.
         *
         * @return the current connection state
         * @see Solid::Control::NetworkInterface::ConnectionState
         */
        ConnectionState connectionState() const;

        /**
         * Retrieves the maximum speed as reported by the device.
         * Note that this is only a design related piece of information, and that
         * the device might not reach this maximum.
         *
         * @return the device's maximum speed
         */
        int designSpeed() const;

        /**
         * Retrieves the capabilities supported by this device.
         *
         * @return the capabilities of the device
         */
        Capabilities capabilities() const;

    Q_SIGNALS:
        /**
         * This signal is emitted when the settings of this network have changed.
         */
        void ipDetailsChanged();

        /**
         * This signal is emitted when the device's link status changed. For example, if there
         * is no carrier anymore.
         *
         * @param linkActivated true if the carrier got detected, false otherwise
         */
        void linkUpChanged(bool linkActivated);

        /**
         * This signal is emitted when the device's link status changed. For example, if there
         * is no carrier anymore.
         *
         * @param state the new state of the connection
         * @see Solid::Control::NetworkInterface::ConnectionState
         */
        void connectionStateChanged(int state);

        /**
         * This signal is emitted when the device's link status changed.
         * 
         * @param new_state the new state of the connection
         * @param old_state the previous state of the connection
         * @param reason the reason for the state change, if any.  ReasonNone where the backend
         * provides no reason.
         * @see Solid::Control::NetworkInterface::ConnectionState
         */
        void connectionStateChanged(int new_state, int old_state, int reason );

    protected:
        /**
         * @internal
         */
        NetworkInterfaceNm09(NetworkInterfaceNm09Private &dd, QObject *backendObject);

        /**
         * @internal
         */
        NetworkInterfaceNm09(NetworkInterfaceNm09Private &dd, const NetworkInterfaceNm09 &network);

        NetworkInterfaceNm09Private *d_ptr;

    private:
    //HACK: to make NetworkList polymorphic (containing both wired and wireless networks, I used Network * here - Will.
    };
    typedef QList<NetworkInterfaceNm09 *> NetworkInterfaceNm09List;


} //Control
} //Solid

Q_DECLARE_OPERATORS_FOR_FLAGS(Solid::Control::NetworkInterfaceNm09::Capabilities)
Q_DECLARE_OPERATORS_FOR_FLAGS(Solid::Control::NetworkInterfaceNm09::Types)

#endif //SOLID_CONTROL_NETWORKINTERFACE_H

