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

#ifndef ACTIVATABLEOBSERVER_H
#define ACTIVATABLEOBSERVER_H

#include <QObject>

#include "knm_export.h"

namespace Knm
{
    class Activatable;
} // namespace Knm

/**
 * Interface for classes which handle Activatables.  Implement as a QObject and connect to
 * ActivatableList's signals
 */
class KNM_EXPORT ActivatableObserver : public QObject
{
Q_OBJECT
public:
    ActivatableObserver(QObject * parent = 0);
    virtual ~ActivatableObserver();
    /**
     * implement as slots
     */
    virtual void handleAdd(Knm::Activatable *) = 0;
    virtual void handleUpdate(Knm::Activatable *) = 0;
    virtual void handleRemove(Knm::Activatable *) = 0;
};

#endif // ACTIVATABLEOBSERVER_H
