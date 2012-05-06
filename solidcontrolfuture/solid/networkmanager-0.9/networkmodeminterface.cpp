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

#include "networkmodeminterface.h"

#include <KDebug>

#include "networkmodeminterface_p.h"
#include "manager.h"

#include "solid/control/modemmanager.h"

NMModemNetworkInterfacePrivate::NMModemNetworkInterfacePrivate(const QString & path, QObject * owner)
    : NMNetworkInterfacePrivate(path, owner), modemIface(NM_DBUS_SERVICE, path, QDBusConnection::systemBus())
{
}

NMModemNetworkInterface::NMModemNetworkInterface(const QString & path, NMNetworkManagerNm09 * manager, QObject * parent)
    : NMNetworkInterface(*new NMModemNetworkInterfacePrivate(path, this), manager, parent),
      modemGsmCardIface(0), modemGsmNetworkIface(0)
{
    init();
}

NMModemNetworkInterface::NMModemNetworkInterface(NMModemNetworkInterfacePrivate & dd, NMNetworkManagerNm09 * manager, QObject * parent) : NMNetworkInterface(dd, manager, parent),
      modemGsmCardIface(0), modemGsmNetworkIface(0)
{
    init();
}

void NMModemNetworkInterface::init()
{
    Q_D(NMModemNetworkInterface);
    d->modemCapabilities = convertModemCapabilities(d->modemIface.modemCapabilities());
    d->currentCapabilities = convertModemCapabilities(d->modemIface.currentCapabilities());
    d->m_modemUdi = getUdiForModemManager();
    connect( &d->modemIface, SIGNAL(PropertiesChanged(QVariantMap)),
                this, SLOT(modemPropertiesChanged(QVariantMap)));
}

NMModemNetworkInterface::~NMModemNetworkInterface()
{
}

void NMModemNetworkInterface::modemPropertiesChanged(const QVariantMap & properties)
{
    Q_D(NMModemNetworkInterface);
    QStringList propKeys = properties.keys();
    //kDebug(1441) << propKeys;
    QLatin1String modemCapabilitiesKey("ModemCapabilities"),
                  currentCapabilitiesKey("CurrentCapabilities");
    QVariantMap::const_iterator it = properties.find(modemCapabilitiesKey);
    if (it != properties.end()) {
        d->modemCapabilities = convertModemCapabilities(it->toUInt());
        emit modemCapabilitiesChanged(d->modemCapabilities);
        propKeys.removeOne(modemCapabilitiesKey);
    }
    it = properties.find(currentCapabilitiesKey);
    if (it != properties.end()) {
        d->currentCapabilities = convertModemCapabilities(it->toUInt());
        emit currentCapabilitiesChanged(d->currentCapabilities);
        propKeys.removeOne(currentCapabilitiesKey);
    }
    if (propKeys.count()) {
        kDebug(1441) << "Unhandled properties: " << propKeys;
    }
}

Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities NMModemNetworkInterface::convertModemCapabilities(uint theirCaps)
{
    Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities ourCaps
        = (Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities) theirCaps;
    return ourCaps;
}

Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities NMModemNetworkInterface::modemCapabilities() const
{
    Q_D(const NMModemNetworkInterface);
    return d->modemCapabilities;
}

Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities NMModemNetworkInterface::currentCapabilities() const
{
    Q_D(const NMModemNetworkInterface);
    return d->currentCapabilities;
}

QString NMModemNetworkInterface::getUdiForModemManager()
{
    if (driver() != QLatin1String("bluez")) {
        return udi();
    }

    /* BlueZ knows about the rfcommX string that we could use to find the device in ModemManager
     * but does not export this info, so let's use the first bluetooth device we find in ModemManager.
     * Modem will be registered in ModemManager only after someone execute its org.bluez.Serial.Connect method. */
    foreach(const Solid::Control::ModemInterface *modem, Solid::Control::ModemManager::modemInterfaces()) {
        if (modem->driver() == QLatin1String("bluetooth")) {
            return modem->udi();
        }
    }

    modemRemoved(udi());
    return QString();
}

Solid::Control::ModemGsmCardInterface * NMModemNetworkInterface::getModemCardIface()
{
    Q_D(NMModemNetworkInterface);
    d->m_modemUdi = getUdiForModemManager();
    if (d->m_modemUdi.isEmpty()) {
        return 0;
    }
    if (modemGsmCardIface == 0) {
        modemGsmCardIface = qobject_cast<Solid::Control::ModemGsmCardInterface*> (Solid::Control::ModemManager::findModemInterface(d->m_modemUdi, Solid::Control::ModemInterface::GsmCard));
        connect(Solid::Control::ModemManager::notifier(), SIGNAL(modemInterfaceRemoved(QString)), this, SLOT(modemRemoved(QString)));
    }

    return modemGsmCardIface;
}

Solid::Control::ModemGsmNetworkInterface * NMModemNetworkInterface::getModemNetworkIface()
{
    Q_D(NMModemNetworkInterface);
    d->m_modemUdi = getUdiForModemManager();
    if (d->m_modemUdi.isEmpty()) {
        return 0;
    }
    if (modemGsmNetworkIface == 0) {
        modemGsmNetworkIface = qobject_cast<Solid::Control::ModemGsmNetworkInterface*> (Solid::Control::ModemManager::findModemInterface(d->m_modemUdi, Solid::Control::ModemInterface::GsmNetwork));
        if (modemGsmNetworkIface) {
            connect(Solid::Control::ModemManager::notifier(), SIGNAL(modemInterfaceRemoved(QString)), this, SLOT(modemRemoved(QString)));
        }
    }

    return modemGsmNetworkIface;
}

void NMModemNetworkInterface::modemRemoved(const QString & modemUdi)
{
    Q_D(const NMModemNetworkInterface);
    if (modemUdi == d->m_modemUdi) {
        modemGsmNetworkIface = 0;
        modemGsmCardIface = 0;
    }
}

void NMModemNetworkInterface::setModemCardIface(Solid::Control::ModemGsmCardInterface * iface)
{
    modemGsmCardIface = iface;
}

void NMModemNetworkInterface::setModemNetworkIface(Solid::Control::ModemGsmNetworkInterface * iface)
{
    modemGsmNetworkIface = iface;
}

#include "networkmodeminterface.moc"

// vim: sw=4 sts=4 et tw=100
