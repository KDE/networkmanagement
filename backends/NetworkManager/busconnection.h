/*
Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>

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

#ifndef BUSCONNECTION_H
#define BUSCONNECTION_H

#include "marshalarguments.h"

#include <QObject>
#include <QVariant>
#include <QMap>
#include <QString>

//DBus specific includes
#include <QtDBus/QtDBus>
#include <QDBusObjectPath>

#include <KConfigGroup>

#include <NetworkManager.h>


class KJob;
class ConnectionSecretsJob;

namespace Knm
{
    class Connection;
} // namespace Knm

/**
 * A representation of a connection on the bus
 */
class BusConnection : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.NetworkManager.Settings.Connection")
    // Can QtDbus handle multiple interfaces being provided by one object like this?
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.NetworkManager.Settings.Connection.Secrets")

    public:
        /**
         * Takes ownership of the Knm::Connection passed to it.
         */
        BusConnection(Knm::Connection * connection, QObject *parent = 0);
        ~BusConnection();

        //export to dbus
        Q_SCRIPTABLE void Update(QVariantMapMap updates);
        void updateInternal(Knm::Connection * connection);
        Q_SCRIPTABLE void Delete();
        Q_SCRIPTABLE QVariantMapMap GetSettings() const;
        Q_SCRIPTABLE QVariantMapMap GetSecrets(const QString &setting_name, const QStringList &hints, bool request_new, const QDBusMessage&);
        QString uuid() const;
        Knm::Connection * connection() const;
    Q_SIGNALS:
        Q_SCRIPTABLE void Updated(const QVariantMapMap &settings);
        Q_SCRIPTABLE void Removed();

    public Q_SLOTS:
        void gotSecrets(KJob*);
    private:
        Knm::Connection * m_connection;
        ConnectionSecretsJob * m_job;
        // a connection persistence while it is doing a wallet look up for us
};

#endif
