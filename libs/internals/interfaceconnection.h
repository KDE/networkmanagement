/*
Copyright 2008 Frederik Gladhorn <gladhorn@kde.org>
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#ifndef KNM_EXTERNALS_INTERFACECONNECTION_H
#define KNM_EXTERNALS_INTERFACECONNECTION_H

#include "activatable.h"

#include <QtCore/QString>
#include <QUuid>

#include "knminternals_export.h"

#include "connection.h"

namespace Knm {

class KNMINTERNALS_EXPORT InterfaceConnection : public Activatable
{
Q_OBJECT
Q_PROPERTY(uint type READ connectionType)
Q_PROPERTY(QString uuid READ connectionUuid)
Q_PROPERTY(QString name READ connectionName)
Q_PROPERTY(uint activationState READ activationState)

public:
    enum ActivationState { Unknown, Activating, Activated };
    InterfaceConnection(const QUuid & connectionUuid, const QString & connectionName, const QString & deviceUni, QObject * parent);
    virtual ~InterfaceConnection();

    void setConnectionType(Knm::Connection::Type type);
    Knm::Connection::Type connectionType() const;

    QUuid connectionUuid() const;

    void setConnectionName(const QString& name);
    QString connectionName() const;

    void setActivationState(ActivationState state);
    ActivationState activationState() const;

Q_SIGNALS:
    void activationStateChanged(Knm::InterfaceConnection::ActivationState);

protected:
    InterfaceConnection(const QUuid & connectionUuid, const QString & connectionName, ActivatableType type, const QString & deviceUni, QObject * parent);

private:
    Knm::Connection::Type m_type;
    QUuid m_uuid;
    QString m_name;
    ActivationState m_state;
};
} // namespace

#endif
