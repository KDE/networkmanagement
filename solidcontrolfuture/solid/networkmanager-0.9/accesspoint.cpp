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

#include "accesspoint.h"

#include <KDebug>
#include "dbus/nm-access-pointinterface.h"
#include "manager.h"
#include "wirelessnetworkinterface.h"


class NMAccessPoint::Private
{
public:
    Private( const QString & path ) : iface( NM_DBUS_SERVICE, path, QDBusConnection::systemBus()), capabilities(0), wpaFlags(0), rsnFlags(0), frequency(0), maxBitRate(0), mode((Solid::Control::WirelessNetworkInterfaceNm09::OperationMode)0), signalStrength(0)
    {
    }
    OrgFreedesktopNetworkManagerAccessPointInterface iface;
    QString uni;
    Solid::Control::AccessPointNm09::Capabilities capabilities;
    Solid::Control::AccessPointNm09::WpaFlags wpaFlags;
    Solid::Control::AccessPointNm09::WpaFlags rsnFlags;
    QString ssid;
    QByteArray rawSsid;
    uint frequency;
    QString hardwareAddress;
    uint maxBitRate;
    Solid::Control::WirelessNetworkInterfaceNm09::OperationMode mode;
    int signalStrength;
};

NMAccessPoint::NMAccessPoint( const QString& path, QObject * parent ) : Solid::Control::Ifaces::AccessPointNm09(parent), d(new Private( path ))
{
    d->uni = path;
    if (d->iface.isValid()) {
        d->capabilities = convertCapabilities( d->iface.flags() );
        d->wpaFlags = convertWpaFlags( d->iface.wpaFlags() );
        d->rsnFlags = convertWpaFlags( d->iface.rsnFlags() );
        d->signalStrength = d->iface.strength();
        d->ssid = QString::fromUtf8(d->iface.ssid());
        d->rawSsid = d->iface.ssid();
        d->frequency = d->iface.frequency();
        d->hardwareAddress = d->iface.hwAddress();
        d->maxBitRate = d->iface.maxBitrate();
        d->mode = NMWirelessNetworkInterface::convertOperationMode(d->iface.mode());
        connect( &d->iface, SIGNAL(PropertiesChanged(QVariantMap)),
                this, SLOT(propertiesChanged(QVariantMap)));
    }
}

NMAccessPoint::~NMAccessPoint()
{
    delete d;
}

QString NMAccessPoint::uni() const
{
    return d->uni;
}

QString NMAccessPoint::hardwareAddress() const
{
    return d->hardwareAddress;
}

Solid::Control::AccessPointNm09::Capabilities NMAccessPoint::capabilities() const
{
    return d->capabilities;
}

Solid::Control::AccessPointNm09::WpaFlags NMAccessPoint::wpaFlags() const
{
    return d->wpaFlags;
}

Solid::Control::AccessPointNm09::WpaFlags NMAccessPoint::rsnFlags() const
{
    return d->rsnFlags;
}

QString NMAccessPoint::ssid() const
{
    return d->ssid;
}

QByteArray NMAccessPoint::rawSsid() const
{
    return d->rawSsid;
}

uint NMAccessPoint::frequency() const
{
    return d->frequency;
}

uint NMAccessPoint::maxBitRate() const
{
    return d->maxBitRate;
}

Solid::Control::WirelessNetworkInterfaceNm09::OperationMode NMAccessPoint::mode() const
{
    return d->mode;
}

int NMAccessPoint::signalStrength() const
{
    return d->signalStrength;
}

void NMAccessPoint::propertiesChanged(const QVariantMap &properties)
{
    QStringList propKeys = properties.keys();
    //kDebug(1441) << propKeys;
    QLatin1String flagsKey("Flags"),
                  wpaFlagsKey("WpaFlags"),
                  rsnFlagsKey("RsnFlags"),
                  ssidKey("Ssid"),
                  freqKey("Frequency"),
                  hwAddrKey("HwAddress"),
                  modeKey("Mode"),
                  maxBitRateKey("MaxBitrate"),
                  strengthKey("Strength");
    QVariantMap::const_iterator it = properties.find(flagsKey);
    if (it != properties.end()) {
        d->capabilities = convertCapabilities(it->toUInt());
        propKeys.removeOne(flagsKey);
    }
    it = properties.find(wpaFlagsKey);
    if (it != properties.end()) {
        d->wpaFlags = convertWpaFlags(it->toUInt());
        emit wpaFlagsChanged(d->wpaFlags);
        propKeys.removeOne(wpaFlagsKey);
    }
    it = properties.find(rsnFlagsKey);
    if (it != properties.end()) {
        d->rsnFlags = convertWpaFlags(it->toUInt());
        emit rsnFlagsChanged(d->rsnFlags);
        propKeys.removeOne(rsnFlagsKey);
    }
    it = properties.find(ssidKey);
    if (it != properties.end()) {
        d->ssid = it->toByteArray();
        emit ssidChanged(d->ssid);
        propKeys.removeOne(ssidKey);
    }
    it = properties.find(freqKey);
    if (it != properties.end()) {
        d->frequency = it->toUInt();
        emit frequencyChanged(d->frequency);
        propKeys.removeOne(freqKey);
    }
    it = properties.find(hwAddrKey);
    if (it != properties.end()) {
        d->hardwareAddress = it->toString();
        propKeys.removeOne(hwAddrKey);
    }
    it = properties.find(modeKey);
    if (it != properties.end()) {
        d->mode = NMWirelessNetworkInterface::convertOperationMode(it->toUInt());
        propKeys.removeOne(modeKey);
    }
    it = properties.find(maxBitRateKey);
    if (it != properties.end()) {
        d->maxBitRate = it->toUInt();
        emit bitRateChanged(d->maxBitRate);
        propKeys.removeOne(maxBitRateKey);
    }
    it = properties.find(strengthKey);
    if (it != properties.end()) {
        d->signalStrength = it->toInt();
        //kDebug(1441) << "UNI: " << d->uni << "MAC: " << d->hardwareAddress << "SignalStrength: " << d->signalStrength;
        emit signalStrengthChanged(d->signalStrength);
        propKeys.removeOne(strengthKey);
    }
    if (!propKeys.isEmpty()) {
        kDebug(1441) << "Unhandled properties: " << propKeys;
    }
}

Solid::Control::AccessPointNm09::Capabilities NMAccessPoint::convertCapabilities(int caps)
{
    if ( 1 == caps ) {
        return Solid::Control::AccessPointNm09::Privacy;
    } else {
        return 0;
    }
}

Solid::Control::AccessPointNm09::WpaFlags NMAccessPoint::convertWpaFlags(uint theirFlags)
{
    return (Solid::Control::AccessPointNm09::WpaFlags)theirFlags;
}

#include "accesspoint.moc"

