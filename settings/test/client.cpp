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

#include "client.h"
#include "marshalarguments.h"

#include <QDBusObjectPath>

#include <KDebug>

#include <NetworkManager.h>

Client::Client(QObject *parent)
    : QObject(parent),
      m_bus(QDBusConnection::systemBus())
{
    m_settings = new QDBusInterface(NM_DBUS_SERVICE_USER_SETTINGS, NM_DBUS_PATH_SETTINGS, NM_DBUS_IFACE_SETTINGS, m_bus, this);
}

Client::~Client()
{
    delete m_settings;
}

void Client::onNewConnection(const QString &name)
{
    kDebug() << "New connection " << name << " was added.";
}

QStringList Client::connections() const
{
    QStringList retVal;
    QDBusReply<QList<QDBusObjectPath> > reply = m_settings->call("ListConnections");
    if (reply.isValid()) {
        QList<QDBusObjectPath> paths = reply.value();
        foreach (const QDBusObjectPath &path, paths) {
            retVal << path.path();
        }
    } else {
        kDebug() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        return QStringList();
    }
    return retVal;
}

QVariantMapMap Client::settings(const QString &connPath) const
{
    kDebug() << "connPath = " << connPath;
    QVariantMapMap retVal;
    QDBusInterface *connection = new QDBusInterface(NM_DBUS_SERVICE_USER_SETTINGS, connPath, NM_DBUS_IFACE_SETTINGS_CONNECTION, m_bus, (QObject*)this);

    QDBusReply<QVariantMapMap> reply = connection->call("GetSettings");
    if(reply.isValid()) {
        retVal =  reply.value();
    } else {
        kDebug() << QDBusError::errorString(reply.error().type());
    }
    delete connection;
    return retVal;
}

#include "client.moc"
