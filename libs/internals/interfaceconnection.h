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
Q_PROPERTY(QString iconName READ iconName WRITE setIconName)
Q_PROPERTY(int activationState READ activationState)
Q_PROPERTY(int oldActivationState READ oldActivationState)
Q_PROPERTY(bool hasDefaultRoute READ hasDefaultRoute WRITE setHasDefaultRoute)

public:
    friend class InterfaceConnectionBuilder;

    enum ActivationState { Unknown, Activating, Activated, Deactivating, Deactivated };


    // To create an object of InterfaceConnection class please use
    // Knm::InterfaceConnectionHelpers::build(..) function.
    virtual ~InterfaceConnection();

    void setConnectionType(Knm::Connection::Type type);
    Knm::Connection::Type connectionType() const;

    QUuid connectionUuid() const;

    void setConnectionName(const QString& name);
    QString connectionName() const;

    void setIconName(const QString &);
    QString iconName() const;

    void setActivationState(ActivationState state);
    ActivationState activationState() const;
    ActivationState oldActivationState() const;

    /**
     * Indicates if this InterfaceConnection provides the default route
     * Only valid if Activated
     */
    void setHasDefaultRoute(bool hasDefault);
    bool hasDefaultRoute() const;
    void deactivate();
    QVariantMap toMap();

Q_SIGNALS:
    void activationStateChanged(Knm::InterfaceConnection::ActivationState oldState, Knm::InterfaceConnection::ActivationState newState);
    // this one is for the DBus adaptor, as autorelaying signals only works if the the type signature is the same
    void activationStateChanged(uint, uint);
    void hasDefaultRouteChanged(bool);
    void deactivated();
    void icPropertiesChanged(const QVariantMap & properties);
protected:
    InterfaceConnection(ActivatableType type, const QString & deviceUni, QObject * parent);

private:
    // After you've added a member please correct InterfaceConnectionBuilder::init(..)
    // function which is responsible for correct initialization.
    Knm::Connection::Type m_type;
    QUuid m_uuid;
    QString m_name;
    QString m_iconName;
    ActivationState m_state;
    ActivationState m_oldState;
    bool m_hasDefaultRoute;
};
} // namespace

#endif
