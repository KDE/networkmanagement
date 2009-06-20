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

#ifndef CONNECTABLELIST_H
#define CONNECTABLELIST_H

#include <QObject>
#include "knm_export.h"

namespace Knm
{
    class Connectable;
} // namespace Knm

class ConnectableListPrivate;

class KNM_EXPORT ConnectableList : public QObject
{
Q_OBJECT
Q_DECLARE_PRIVATE(ConnectableList);

public:
    ConnectableList(QObject * parent);
    ~ConnectableList();
    QList<Knm::Connectable *> connectables() const;
    void addConnectable(Knm::Connectable *);
    void removeConnectable(Knm::Connectable *);

signals:
    void connectableAdded(Knm::Connectable *);
    void connectableRemoved(Knm::Connectable *);

private:
    ConnectableListPrivate * d_ptr;
};

#endif // CONNECTABLELIST_H
