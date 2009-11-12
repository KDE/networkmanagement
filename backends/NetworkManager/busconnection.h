/*  This file is part of the KDE project
    Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

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
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.NetworkManagerSettings.Connection")
    // Can QtDbus handle multiple interfaces being provided by one object like this?
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.NetworkManagerSettings.Connection.Secrets")

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
        KJob * m_job;
        // a connection persistence while it is doing a wallet look up for us
};

#endif
