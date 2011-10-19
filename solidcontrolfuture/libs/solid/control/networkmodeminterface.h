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

#ifndef SOLID_CONTROL_MODEMNETWORKINTERFACE_H
#define SOLID_CONTROL_MODEMNETWORKINTERFACE_H

#include "networkinterface.h"
#include "modemgsmcardinterface.h"
#include "modemgsmnetworkinterface.h"

namespace Solid
{
namespace Control
{
    class ModemNetworkInterfaceNm09Private;
    /**
     * This interface represents a cellular network interface
     */
    class SOLIDCONTROL_EXPORT ModemNetworkInterfaceNm09 : public NetworkInterfaceNm09
    {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ModemNetworkInterfaceNm09)
        Q_FLAGS(ModemCapabilities)
        Q_FLAGS(CurrentCapabilities)

    public:
        /**
         * Flags describing one or more of the general access technology families that a modem device supports.
         */
        enum ModemCapability {
            None = 0x0, /* Modem has no capabilties */
            Pots = 0x1, /* Modem supports the analog wired telephone network (ie 56k dialup) and does not have wireless/cellular capabilities. */
            CdmaEvdo = 0x2, /* Modem supports at least one of CDMA 1xRTT, EVDO revision 0, EVDO revision A, or EVDO revision B. */
            GsmUmts = 0x4, /* Modem supports at least one of GSM, GPRS, EDGE, UMTS, HSDPA, HSUPA, or HSPA+ packet switched data capability. */
            Lte = 0x8 /* Modem has at LTE data capability. */
        };

        Q_DECLARE_FLAGS(ModemCapabilities, ModemCapability)
        Q_DECLARE_FLAGS(CurrentCapabilities, ModemCapability)

        /**
         * Creates a new ModemNetworkInterface object.
         *
         * @param backendObject the network object provided by the backend
         */
        ModemNetworkInterfaceNm09(QObject *backendObject = 0);

        /**
         * Constructs a copy of a network.
         *
         * @param network the network to copy
         */
        ModemNetworkInterfaceNm09(const ModemNetworkInterfaceNm09 &network);

        /**
         * Destroys a ModemNetworkInterface object.
         */
        virtual ~ModemNetworkInterfaceNm09();

        /**
         * The NetworkInterface type.
         *
         * @return the NetworkInterface::Type.  This always returns NetworkInterface::Ieee8023
         */
        virtual NetworkInterfaceNm09::Type type() const;

        ModemCapability subType() const;

        /**
         * The generic family of access technologies the modem supports. Not all capabilities are
         * available at the same time however; some modems require a firmware reload or other
         * reinitialization to switch between eg CDMA/EVDO and GSM/UMTS.
         */
        Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities modemCapabilities() const;

        /**
         * The generic family of access technologies the modem currently supports without a firmware
         * reload or reinitialization.
         */
        Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities currentCapabilities() const;

        ModemGsmCardInterface * getModemCardIface();

        void setModemCardIface(Solid::Control::ModemGsmCardInterface * iface);

        ModemGsmNetworkInterface * getModemNetworkIface();

        void setModemNetworkIface(Solid::Control::ModemGsmNetworkInterface * iface);

   Q_SIGNALS:
        /**
         * This signal is emitted when the network name of this network changes
         *
         * @param networkName the new network name
         */
        void networkNameChanged(const QString & networkName);

        /**
         * This signal is emitted when the modem capabilities changes
         *
         * @param caps the new capabilities
         */
        void modemCapabilitiesChanged(const Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities caps);

        /**
         * This signal is emitted when the current modem capabilities changes
         *
         * @param caps the new current capabilities
         */
        void currentCapabilitiesChanged(const Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities caps);

    protected:
        /**
         * @internal
         */
        ModemNetworkInterfaceNm09(ModemNetworkInterfaceNm09Private &dd, QObject *backendObject);

        /**
         * @internal
         */
        ModemNetworkInterfaceNm09(ModemNetworkInterfaceNm09Private &dd, const ModemNetworkInterfaceNm09 &network);

        void makeConnections(QObject * source);
    private Q_SLOTS:
        void _k_destroyed(QObject *object);
    private:
        friend class NetworkInterfaceNm09;
        friend class NetworkInterfaceNm09Private;
    };
} //Control
} //Solid

Q_DECLARE_OPERATORS_FOR_FLAGS(Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities)
#endif // SOLID_CONTROL_MODEMNETWORKINTERFACE_H

