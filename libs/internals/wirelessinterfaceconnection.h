/*
Copyright 2008 Frederik Gladhorn <gladhorn@kde.org>
Copyright 2009 Will Stephenson <wstephenson@kde.org>
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

#ifndef KNM_EXTERNALS_WIRELESSINTERFACECONNECTION_H
#define KNM_EXTERNALS_WIRELESSINTERFACECONNECTION_H

#include "interfaceconnection.h"
#include "wirelessobject.h"

#include "knminternals_export.h"

namespace Knm {

class KNMINTERNALS_EXPORT WirelessInterfaceConnection : public InterfaceConnection, public WirelessObject
{
Q_OBJECT
Q_PROPERTY(QString ssid READ ssid)
Q_PROPERTY(int strength READ strength WRITE setStrength)
Q_PROPERTY(uint wpaFlags READ wpaFlags)
Q_PROPERTY(uint rsnFlags READ rsnFlags)

public:
    friend class WirelessInterfaceConnectionBuilder;

    // To create an object of WirelessInterfaceConnection class please use
    // Knm::WirelessInterfaceConnectionHelpers::build(..) function.
    virtual ~WirelessInterfaceConnection();
    QVariantMap toMap();
signals:
    void strengthChanged(int);
    void wicPropertiesChanged(const QVariantMap & properties);
public Q_SLOTS:
    void setStrength(int);

protected:
    WirelessInterfaceConnection(ActivatableType type, const QString & deviceUni, QObject * parent);

};
} // namespace

#endif
