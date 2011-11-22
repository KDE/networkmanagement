/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QObject>
#include <QPointer>

#include <KComponentData>
#include <KNotification>

#include <interfaceconnection.h>
#include <connectionlist.h>

#include "activatableobserver.h"
#include <Solid/Networking>
#include <solid/control/networkinterface.h>

#include "knm_export.h"

class NotificationManagerPrivate;

class KNM_EXPORT NotificationManager : public QObject, public ActivatableObserver
{
Q_OBJECT
Q_DECLARE_PRIVATE(NotificationManager)
public:
    NotificationManager(ConnectionList *connectionList, QObject * parent = 0);
    virtual ~NotificationManager();
    void handleAdd(Knm::Activatable *);
    void handleUpdate(Knm::Activatable *);
    void handleRemove(Knm::Activatable *);
    static void performNotification(const QString &eventId, const QString &title, const QString &text, const QString &icon);

    KComponentData componentData() const;
protected Q_SLOTS:
    void networkInterfaceAdded(const QString &);
    void networkInterfaceRemoved(const QString &);
    void networkAppeared(const QString &);
    void networkDisappeared(const QString &);

    void notifyNewNetworks();
    void notifyDisappearedNetworks();

    void wirelessHardwareEnabledChanged(bool);
    void statusChanged(Solid::Networking::Status);
    void createCellularConnection();
private:
    NotificationManagerPrivate * d_ptr;
};

/**
 * Tracks notifications concerning an interface
 * Keeps at most one notification per interface
 * If closed opens another
 * Else reuses
 * Knows which interfaceconnections are active/activating per interface
 * Uses this knowledge to show which interfaceconnection interface status changes refer to
 * on ASC, add sending InterfaceConnection to list of actives
 * On ICSC, show name of currently activating connection together with new interface state
 */
class InterfaceNotificationHost : public QObject
{
Q_OBJECT
public:
    InterfaceNotificationHost(Solid::Control::NetworkInterfaceNm09 * iface, NotificationManager * parent);
    ~InterfaceNotificationHost();

    void addInterfaceConnection(Knm::InterfaceConnection* added);
    void removeInterfaceConnection(Knm::InterfaceConnection* removed);

    QString label() const;
    Knm::Connection::Type type() const;
public Q_SLOTS:
    /*
     * Update notification with state changes for iface's active connections
     */
    void interfaceConnectionStateChanged(int,int,int);
    /*
     * So we know which ICs interface state changes refer to
     */
    void interfaceConnectionActivationStateChanged(Knm::InterfaceConnection::ActivationState, Knm::InterfaceConnection::ActivationState);

    void strengthChanged(int strength);

private Q_SLOTS:
    void enableStrengthNotification();

private:
    void performInterfaceNotification(const QString & title, const QString & text, const QPixmap & pixmap, KNotification::NotificationFlag flag);

    NotificationManager * m_manager;
    Solid::Control::NetworkInterfaceNm09 * m_interface;
    // used to refer to the interface if it is removed
    QString m_interfaceNameLabel;
    Knm::Connection::Type m_type;
    QPointer<KNotification> m_notification;
    QSet<Knm::InterfaceConnection*> m_activating;
    QSet<Knm::InterfaceConnection*> m_interfaceConnections;
    bool m_suppressStrengthNotification;
};

#endif // NOTIFICATIONMANAGER_H
