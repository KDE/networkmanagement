/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

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

#ifndef REMOTE_CONNECTION_H
#define REMOTE_CONNECTION_H

#include "types.h"
#include "knm_export.h"

#include "nm-settings-connectioninterface.h"

#include <solid/control/networkinterface.h>

class KNM_EXPORT RemoteConnection : public OrgFreedesktopNetworkManagerSettingsConnectionInterface
{
Q_OBJECT
public:
    RemoteConnection(const QString &service, const QString &path, QObject * parent = 0);
    ~RemoteConnection();
    QString id() const;
    Solid::Control::NetworkInterfaceNm09::Type type() const;
    QString path() const;
    QVariantMapMap settings() const;
    bool active() const;
//public Q_SLOTS:
//  settingsUpdated(const QVariantMapMap&);
private:
    QString m_id;
    Solid::Control::NetworkInterfaceNm09::Type m_type;
    QVariantMapMap m_connection;
    QString m_path;
};

#endif // REMOTE_CONNECTION_H
