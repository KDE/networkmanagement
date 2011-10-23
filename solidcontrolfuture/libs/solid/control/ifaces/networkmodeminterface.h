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

#ifndef SOLID_IFACES_MODEMNETWORKINTERFACE_H
#define SOLID_IFACES_MODEMNETWORKINTERFACE_H

#include "../solid_control_export.h"

#include <QtCore/QStringList>

#include "../networkmodeminterface.h"
#include "networkinterface.h"


namespace Solid
{
namespace Control
{
namespace Ifaces
{
    /**
     * Represents a modem network interface
     */
    class SOLIDCONTROLIFACES_EXPORT ModemNetworkInterfaceNm09 : virtual public NetworkInterfaceNm09
    {
    public:
        /**
         * Destroys a ModemNetworkInterface object
         */
        virtual ~ModemNetworkInterfaceNm09();

        /**
         * The generic family of access technologies the modem supports. Not all capabilities are
	 * available at the same time however; some modems require a firmware reload or other
	 * reinitialization to switch between eg CDMA/EVDO and GSM/UMTS.
         */
        virtual Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities modemCapabilities() const = 0;

        /**
	 * The generic family of access technologies the modem currently supports without a firmware
	 * reload or reinitialization.
	 */
        virtual Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities currentCapabilities() const = 0;

        virtual Solid::Control::ModemGsmCardInterface * getModemCardIface() = 0;

        virtual void setModemCardIface(Solid::Control::ModemGsmCardInterface * iface) = 0;

        virtual Solid::Control::ModemGsmNetworkInterface * getModemNetworkIface() = 0;

        virtual void setModemNetworkIface(Solid::Control::ModemGsmNetworkInterface * iface) = 0;

    protected:
    //Q_SIGNALS:
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
    };
} //Ifaces
} //Control
} //Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::ModemNetworkInterfaceNm09, "org.kde.Solid.Control.Ifaces.ModemNetworkInterface/0.1")

#endif //SOLID_IFACES_MODEMNETWORKINTERFACE_H

