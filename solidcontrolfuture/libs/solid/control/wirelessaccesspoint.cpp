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

#include "wirelessaccesspoint.h"
#include "frontendobject_p.h"
#include "soliddefs_p.h"
#include "ifaces/wirelessaccesspoint.h"

namespace Solid
{
namespace Control
{
    class AccessPointNm09Private : public FrontendObjectNm09Private
    {
    public:
        AccessPointNm09Private(QObject *parent)
            : FrontendObjectNm09Private(parent) { }

        void setBackendObject(QObject *object);
    };
}
}

Solid::Control::AccessPointNm09::AccessPointNm09(QObject *backendObject)
    : QObject(), d_ptr(new AccessPointNm09Private(this))
{
    Q_D(AccessPointNm09);
    d->setBackendObject(backendObject);
}

Solid::Control::AccessPointNm09::AccessPointNm09(const AccessPointNm09 &network)
    : QObject(), d_ptr(new AccessPointNm09Private(this))
{
    Q_D(AccessPointNm09);
    d->setBackendObject(network.d_ptr->backendObject());
}

Solid::Control::AccessPointNm09::AccessPointNm09(AccessPointNm09Private &dd, QObject *backendObject)
    : QObject(), d_ptr(&dd)
{
    Q_D(AccessPointNm09);
    d->setBackendObject(backendObject);
}

Solid::Control::AccessPointNm09::AccessPointNm09(AccessPointNm09Private &dd, const AccessPointNm09 &ap)
    : d_ptr(&dd)
{
    Q_D(AccessPointNm09);
    d->setBackendObject(ap.d_ptr->backendObject());
}

Solid::Control::AccessPointNm09::~AccessPointNm09()
{
    delete d_ptr;
}

QString Solid::Control::AccessPointNm09::uni() const
{
    Q_D(const AccessPointNm09);
    return_SOLID_CALL(Ifaces::AccessPointNm09 *, d->backendObject(), QString(), uni());
}

Solid::Control::AccessPointNm09::Capabilities Solid::Control::AccessPointNm09::capabilities() const
{
    Q_D(const AccessPointNm09);
    return_SOLID_CALL(Ifaces::AccessPointNm09 *, d->backendObject(), 0, capabilities());

}

Solid::Control::AccessPointNm09::WpaFlags Solid::Control::AccessPointNm09::wpaFlags() const
{
    Q_D(const AccessPointNm09);
    return_SOLID_CALL(Ifaces::AccessPointNm09 *, d->backendObject(), 0, wpaFlags());

}

Solid::Control::AccessPointNm09::WpaFlags Solid::Control::AccessPointNm09::rsnFlags() const
{
    Q_D(const AccessPointNm09);
    return_SOLID_CALL(Ifaces::AccessPointNm09 *, d->backendObject(), 0, rsnFlags());

}

QString Solid::Control::AccessPointNm09::ssid() const
{
    Q_D(const AccessPointNm09);
    return_SOLID_CALL(Ifaces::AccessPointNm09 *, d->backendObject(), QString(), ssid());
}

QByteArray Solid::Control::AccessPointNm09::rawSsid() const
{
    Q_D(const AccessPointNm09);
    return_SOLID_CALL(Ifaces::AccessPointNm09 *, d->backendObject(), QByteArray(), rawSsid());
}

uint Solid::Control::AccessPointNm09::frequency() const
{
    Q_D(const AccessPointNm09);
    return_SOLID_CALL(Ifaces::AccessPointNm09 *, d->backendObject(), 0, frequency());

}

QString Solid::Control::AccessPointNm09::hardwareAddress() const
{
    Q_D(const AccessPointNm09);
    return_SOLID_CALL(Ifaces::AccessPointNm09 *, d->backendObject(), QString(), hardwareAddress());

}

uint Solid::Control::AccessPointNm09::maxBitRate() const
{
    Q_D(const AccessPointNm09);
    return_SOLID_CALL(Ifaces::AccessPointNm09 *, d->backendObject(), 0, maxBitRate());
}

Solid::Control::WirelessNetworkInterfaceNm09::OperationMode Solid::Control::AccessPointNm09::mode() const
{
    Q_D(const AccessPointNm09);
    return_SOLID_CALL(Ifaces::AccessPointNm09 *, d->backendObject(), (Solid::Control::WirelessNetworkInterfaceNm09::OperationMode)0, mode());
}

int Solid::Control::AccessPointNm09::signalStrength() const
{
    Q_D(const AccessPointNm09);
    return_SOLID_CALL(Ifaces::AccessPointNm09 *, d->backendObject(), 0, signalStrength());
}

void Solid::Control::AccessPointNm09Private::setBackendObject(QObject *object)
{
    FrontendObjectNm09Private::setBackendObject(object);

    if (object) {
        QObject::connect(object, SIGNAL(signalStrengthChanged(int)),
                parent(), SIGNAL(signalStrengthChanged(int)));
        QObject::connect(object, SIGNAL(bitRateChanged(int)),
                parent(), SIGNAL(bitRateChanged(int)));
        QObject::connect(object, SIGNAL(wpaFlagsChanged(Solid::Control::AccessPointNm09::WpaFlags)),
                parent(), SIGNAL(wpaFlagsChanged(Solid::Control::AccessPointNm09::WpaFlags)));
        QObject::connect(object, SIGNAL(rsnFlagsChanged(Solid::Control::AccessPointNm09::WpaFlags)),
                parent(), SIGNAL(rsnFlagsChanged(Solid::Control::AccessPointNm09::WpaFlags)));
        QObject::connect(object, SIGNAL(ssidChanged(QString)),
                parent(), SIGNAL(ssidChanged(QString)));
        QObject::connect(object, SIGNAL(frequencyChanged(uint)),
                parent(), SIGNAL(frequencyChanged(uint)));
    }
}

void Solid::Control::AccessPointNm09::_k_destroyed(QObject *object)
{
    Q_UNUSED(object);
    // nothing to do yet
}

#include "wirelessaccesspoint.moc"
