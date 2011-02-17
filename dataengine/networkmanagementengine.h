/*
    Copyright 2011 Sebastian Kügler <sebas@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/


#ifndef NETWORKMANAGEMENTENGINE_H
#define NETWORKMANAGEMENTENGINE_H

#include <plasma/dataengine.h>


class RemoteActivatableList;
class RemoteActivatable;

class NetworkManagementEnginePrivate;

class NetworkManagementEngine : public Plasma::DataEngine
{
    Q_OBJECT

    public:
        NetworkManagementEngine(QObject* parent, const QVariantList& args);
        ~NetworkManagementEngine();
        QStringList sources() const;
        virtual void init();

    private Q_SLOTS:
        void activatableAdded(RemoteActivatable*);
        void activatableRemoved(RemoteActivatable*);
        void listDisappeared();
        void listAppeared();

    protected:
        bool sourceRequestEvent(const QString &name);

    private:
        NetworkManagementEnginePrivate* d;
};

K_EXPORT_PLASMA_DATAENGINE(networkmanagementengine, NetworkManagementEngine)

#endif
