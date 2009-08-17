/*
Copyright 2009 Paul Marchouk <pmarchouk@gmail.com>

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

#ifndef KNM_EXTERNALS_INTERFACECONNECTIONHELPERS_P_H
#define KNM_EXTERNALS_INTERFACECONNECTIONHELPERS_P_H

#include <QString>

class QObject;

namespace Knm {

class Connection;
class InterfaceConnection;

// The main goal of this class is create and initialize an InterfaceConnection object
// from the corresponding Connection object.
// For example, someone added new fields to InterfaceConnection. And it would be nice
// that all related code be aware of these new fields and the correct initialization.
// It could be done by changing ctor. But in this case you would have to change
// derived classes ctors etc. Another way to use a builder object. I mean incapsulate all
// knowledge and actions into a Builder class.
class InterfaceConnectionBuilder
{
public:
    InterfaceConnectionBuilder(Knm::Connection *connection, const QString & deviceUni, QObject * parent);
    virtual ~InterfaceConnectionBuilder();

    Knm::InterfaceConnection* build();

protected:
    void init(InterfaceConnection *ic);

private:
    Q_DISABLE_COPY(InterfaceConnectionBuilder)

protected:
    Knm::Connection * m_connection;
    QString m_deviceUni;
    QObject * m_parent;
};

// This class is responsible for the synchronization
// between Knm::InterfaceConnection and Knm::Connection objects.
class InterfaceConnectionSync
{
public:
    InterfaceConnectionSync();
    virtual ~InterfaceConnectionSync();

    void sync(Knm::InterfaceConnection * interfaceConnection,
                      Knm::Connection *connection);

private:
    Q_DISABLE_COPY(InterfaceConnectionSync)
};

} //Knm

#endif // KNM_EXTERNALS_INTERFACECONNECTIONHELPERS_P_H
