/*
Copyright 2011 Lamarque Souza <lamarque@gmail.com>

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

#include "networkmodeminterface.h"
#include "solid/control/ifaces/networkbtinterface.h"

#include "dbus/generic-types.h"

#ifndef NM09_BTNETWORKINTERFACE_H
#define NM09_BTNETWORKINTERFACE_H

class NMNetworkManagerNm09;
class NMBtNetworkInterfacePrivate;

class KDE_EXPORT NMBtNetworkInterface : public NMModemNetworkInterface, virtual public Solid::Control::Ifaces::BtNetworkInterfaceNm09
{
Q_OBJECT
Q_DECLARE_PRIVATE(NMBtNetworkInterface)
Q_INTERFACES(Solid::Control::Ifaces::BtNetworkInterfaceNm09)

Q_PROPERTY(uint btCapabilities READ btCapabilities)
Q_PROPERTY(QString hardwareAddress READ hardwareAddress)
Q_PROPERTY(QString name READ name)

Q_FLAGS(Solid::Control::BtNetworkInterfaceNm09::btCapabilities)
public:
    NMBtNetworkInterface(const QString & path, NMNetworkManagerNm09 * manager, QObject * parent);
    virtual ~NMBtNetworkInterface();

    Solid::Control::BtNetworkInterfaceNm09::Capabilities btCapabilities() const;
    QString hardwareAddress() const;
    QString name() const;
public Q_SLOTS:
    void btPropertiesChanged(const QVariantMap & changedProperties);
Q_SIGNALS:
    void networkNameChanged(const QString &name);
};

#endif
