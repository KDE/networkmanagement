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

#ifndef REMOTE_CONNECTION_H
#define REMOTE_CONNECTION_H

#include "nm-exported-connectioninterface.h"

#include "../libs/types.h"

#include <solid/control/networkinterface.h>

class RemoteConnection : public OrgFreedesktopNetworkManagerSettingsConnectionInterface
{
Q_OBJECT
public:
    RemoteConnection(const QString &service, const QString &path, QObject * parent = 0);
    ~RemoteConnection();
    QString id() const;
    Solid::Control::NetworkInterface::Type type() const;
    QVariantMapMap settings() const;
    bool active() const;
//public Q_SLOTS:
//  settingsUpdated(const QVariantMapMap&);
private:
    QString m_id;
    Solid::Control::NetworkInterface::Type m_type;
    QVariantMapMap m_connection;
};

#endif // REMOTE_CONNECTION_H
