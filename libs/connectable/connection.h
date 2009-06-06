/*
Copyright 2008 Frederik Gladhorn <gladhorn@kde.org>

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

#ifndef CONNECTION_H
#define CONNECTION_H

#include "connectable.h"

#include <solid/control/networkinterface.h>
#include <QtCore/QString>

namespace Knm {

class Connection :public Connectable
{
    Q_OBJECT

public:
    Connection();
    
    virtual ~Connection(){}

    void setConnectionType(const QString& type);
    QString connectionType();

    void setConnectionUni(const QString& uni);
    QString connectionUni();

    void setConnectionName(const QString& name);
    QString connectionName();

    void setConnectionState(Solid::Control::NetworkInterface::ConnectionState state);
    Solid::Control::NetworkInterface::ConnectionState connectionState();
    
Q_SIGNALS:
    void connectionStateChanged();
    
private:
    QString m_type;
    QString m_name;
    QString m_uni;
    Solid::Control::NetworkInterface::ConnectionState m_state;
};
} // namespace

#endif
