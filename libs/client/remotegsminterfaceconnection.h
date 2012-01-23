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

#ifndef REMOTEGSMINTERFACECONNECTION_H
#define REMOTEGSMINTERFACECONNECTION_H

#include "remoteinterfaceconnection.h"

#include <solid/control/networkmodeminterface.h>

#include "knmclient_export.h"

class RemoteGsmInterfaceConnectionPrivate;

class KNMCLIENT_EXPORT RemoteGsmInterfaceConnection : public RemoteInterfaceConnection
{
Q_OBJECT
Q_PROPERTY(int getAccessTechnology READ getAccessTechnology)
Q_PROPERTY(int getSignalQuality READ getSignalQuality)

friend class RemoteActivatableList;

public:
    virtual ~RemoteGsmInterfaceConnection();
    int getAccessTechnology() const;
    int getSignalQuality() const;
Q_SIGNALS:
    void signalQualityChanged(int);
    void accessTechnologyChanged(const int);
protected Q_SLOTS:
    void gsmPropertiesChanged(const QVariantMap &properties);
protected:
    RemoteGsmInterfaceConnection(const QVariantMap & properties, QObject * parent);
    Q_DECLARE_PRIVATE(RemoteGsmInterfaceConnection)
};

#endif // REMOTEGSMINTERFACECONNECTION_H
