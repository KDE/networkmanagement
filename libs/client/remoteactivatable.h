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

#ifndef REMOTEACTIVATABLE_H
#define REMOTEACTIVATABLE_H

#include <QObject>

#include "activatable.h"

#include "knmclient_export.h"

class RemoteActivatablePrivate;

/**
 * Almost, but not exactly, entirely like a Knm::Activatable
 */
class KNMCLIENT_EXPORT RemoteActivatable : public QObject
{
Q_OBJECT
Q_PROPERTY(QString deviceUni READ deviceUni)
Q_PROPERTY(QString type READ activatableType)

friend class RemoteActivatableList;

public:
    virtual ~RemoteActivatable();
    Knm::Activatable::ActivatableType activatableType() const;
    QString deviceUni() const;
public Q_SLOTS:
    void activate();
Q_SIGNALS:
    void activated();
    void changed();
protected:
    Q_DECLARE_PRIVATE(RemoteActivatable)
    RemoteActivatable(const QString &dbusPath, QObject * parent);
    RemoteActivatable(RemoteActivatablePrivate & dd, const QString &dbusPath, QObject * parent);
    RemoteActivatablePrivate * d_ptr;
private:
    void init();
};

#endif // REMOTEACTIVATABLE_H
