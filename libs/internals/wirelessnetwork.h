/*
Copyright 2008 Frederik Gladhorn <gladhorn@kde.org>
Copyright 2012 Lamarque V. Souza <lamarque@kde.org>

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

#ifndef WIRELESSNETWORK_H
#define WIRELESSNETWORK_H

#include "activatable.h"
#include "wirelessobject.h"

#include <QtCore/QString>
#include <QObject>

#include "knminternals_export.h"

namespace Knm {

class KNMINTERNALS_EXPORT WirelessNetwork : public Activatable, public WirelessObject
{
Q_OBJECT
Q_PROPERTY(QString ssid READ ssid)
Q_PROPERTY(int strength READ strength)
Q_PROPERTY(uint wpaFlags READ wpaFlags)
Q_PROPERTY(uint rsnFlags READ rsnFlags)
public:
    WirelessNetwork(const QString & ssid, int strength, Solid::Control::WirelessNetworkInterfaceNm09::Capabilities interfaceCapabilities, Solid::Control::AccessPointNm09::Capabilities apCapabilities, Solid::Control::AccessPointNm09::WpaFlags wpaFlags, Solid::Control::AccessPointNm09::WpaFlags rsnFlags, Solid::Control::WirelessNetworkInterfaceNm09::OperationMode mode, const QString & deviceUni, QObject * parent);
    virtual ~WirelessNetwork();
    QVariantMap toMap();
signals:
    void strengthChanged(int);
    void wnPropertiesChanged(const QVariantMap & properties);
public Q_SLOTS:
    void setStrength(int);
};
} // namespace

#endif // WIRELESSNETWORK_H
