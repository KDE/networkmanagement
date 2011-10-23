/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

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

#include "networkinterface.h"
#include "solid/control/ifaces/networkmodeminterface.h"
#include "solid/control/modemgsmnetworkinterface.h"

#include "dbus/generic-types.h"

#ifndef NM09_NETWORKMODEMINTERFACE_H
#define NM09_NETWORKMODEMINTERFACE_H

class NMNetworkManagerNm09;
class NMModemNetworkInterfacePrivate;

class KDE_EXPORT NMModemNetworkInterface : public NMNetworkInterface, virtual public Solid::Control::Ifaces::ModemNetworkInterfaceNm09
{
Q_OBJECT
Q_DECLARE_PRIVATE(NMModemNetworkInterface)
Q_INTERFACES(Solid::Control::Ifaces::ModemNetworkInterfaceNm09)

Q_FLAGS(Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities)
Q_FLAGS(Solid::Control::ModemNetworkInterfaceNm09::CurrentCapabilities)
public:
    NMModemNetworkInterface(const QString & path, NMNetworkManagerNm09 * manager, QObject * parent);
    NMModemNetworkInterface(NMModemNetworkInterfacePrivate &dd, NMNetworkManagerNm09 * manager, QObject * parent);
    virtual ~NMModemNetworkInterface();
    Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities modemCapabilities() const;
    Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities currentCapabilities() const;
    Solid::Control::ModemGsmCardInterface * getModemCardIface();
    void setModemCardIface(Solid::Control::ModemGsmCardInterface * iface);
    Solid::Control::ModemGsmNetworkInterface * getModemNetworkIface();
    void setModemNetworkIface(Solid::Control::ModemGsmNetworkInterface * iface);
public Q_SLOTS:
    void modemPropertiesChanged(const QVariantMap & changedProperties);
    void modemRemoved(const QString & modemUdi);
Q_SIGNALS:
    void modemCapabilitiesChanged(const Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities caps);
    void currentCapabilitiesChanged(const Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities caps);
protected:
    Solid::Control::ModemGsmCardInterface *modemGsmCardIface;
    Solid::Control::ModemGsmNetworkInterface *modemGsmNetworkIface;
    QString getUdiForModemManager();
private:
    void init();
    static Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities convertModemCapabilities(uint caps);
};

#endif
