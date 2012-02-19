/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2010-2011 Lamarque Souza <lamarque@gmail.com>

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

#include "remotegsminterfaceconnection.h"
#include "remotegsminterfaceconnection_p.h"

#include <KDebug>

RemoteGsmInterfaceConnection::RemoteGsmInterfaceConnection(const QVariantMap &properties, QObject * parent)
: RemoteInterfaceConnection(*new RemoteGsmInterfaceConnectionPrivate, properties, parent)
{
    Q_D(RemoteGsmInterfaceConnection);
    d->gsmInterfaceConnectionIface = new GsmInterfaceConnectionInterface("org.kde.networkmanagement", properties["path"].toString(), QDBusConnection::sessionBus(), this);
    connect(d->gsmInterfaceConnectionIface, SIGNAL(gsmPropertiesChanged(QVariantMap)), SLOT(gsmPropertiesChanged(QVariantMap)));
    gsmPropertiesChanged(properties);
}

RemoteGsmInterfaceConnection::~RemoteGsmInterfaceConnection()
{

}

void RemoteGsmInterfaceConnection::gsmPropertiesChanged(const QVariantMap &changedProperties)
{
    Q_D(RemoteGsmInterfaceConnection);
    QStringList propKeys = changedProperties.keys();
    QLatin1String signalQualityKey("signalQuality"),
                  accessTechnologyKey("accessTechnology");
    QVariantMap::const_iterator it = changedProperties.find(signalQualityKey);
    if (it != changedProperties.end()) {
        d->signalQuality = it->toInt();
        emit signalQualityChanged(d->signalQuality);
        propKeys.removeOne(signalQualityKey);
    }
    it = changedProperties.find(accessTechnologyKey);
    if (it != changedProperties.end()) {
        d->accessTechnology = it->toInt();
        emit accessTechnologyChanged(d->accessTechnology);
        propKeys.removeOne(accessTechnologyKey);
    }
    /*if (propKeys.count()) {
        kDebug() << "Unhandled properties: " << propKeys;
    }*/
    emit changed();
}
int RemoteGsmInterfaceConnection::getAccessTechnology() const
{
    Q_D(const RemoteGsmInterfaceConnection);
    return d->accessTechnology;
}

int RemoteGsmInterfaceConnection::getSignalQuality() const
{
    Q_D(const RemoteGsmInterfaceConnection);
    return d->signalQuality;
}

// vim: sw=4 sts=4 et tw=100
