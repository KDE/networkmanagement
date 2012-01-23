/*
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

#ifndef SESSIONABSTRACTEDSERVICE_H
#define SESSIONABSTRACTEDSERVICE_H

#include "activatableobserver.h"

#include <QStringList>
#include <QDBusObjectPath>

#include "knm_export.h"

class ActivatableList;
class SessionAbstractedServicePrivate;

class KNM_EXPORT SessionAbstractedService : public QObject, public ActivatableObserver
{
Q_OBJECT
Q_PROPERTY(QStringList activatables READ ListActivatables)
Q_CLASSINFO("D-Bus Interface", "org.kde.networkmanagement")
public:
    SessionAbstractedService(ActivatableList *, QObject *parent);
    ~SessionAbstractedService();

    /**
     * Export a newly added Activatable on the session bus
     */
    void handleAdd(Knm::Activatable *);
    void handleUpdate(Knm::Activatable *);
    /**
     * Announce that an Activatable was removed from the session bus
     */
    void handleRemove(Knm::Activatable *);
    static const QString SESSION_SERVICE_DBUS_PATH;
public slots:
    Q_SCRIPTABLE QStringList ListActivatables() const;
    Q_SCRIPTABLE void ReadConfig();
    Q_SCRIPTABLE void FinishInitialization();
    Q_SCRIPTABLE void ReEmitActivatableList();
signals:
    Q_SCRIPTABLE void ActivatableAdded(const QVariantMap & properties);
    Q_SCRIPTABLE void ActivatableRemoved(const QString &);
    Q_SCRIPTABLE void ReloadConfig();
    Q_SCRIPTABLE void ModuleReady();
    void DoFinishInitialization();
private:
    Q_DECLARE_PRIVATE(SessionAbstractedService)
    QString nextObjectPath();
    SessionAbstractedServicePrivate * d_ptr;
};

#endif // SESSIONABSTRACTEDSERVICE_H
