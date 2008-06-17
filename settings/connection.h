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

#ifndef CONNECTION_H
#define CONNECTION_H

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

class Connection : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.NetworkManagerSettings.Connection")
    // Can QtDbus handle multiple interfaces being provided by one object like this?
    //Q_CLASSINFO("D-Bus Interface", "org.freedesktop.NetworkManagerSettings.Connection.Secrets")

    public:
        enum ConnectionType {Unknown=0, Wired, Wireless};

        Connection(const QString & id, const QVariantMapMap & settingsMap, QObject *parent=0);
        ~Connection();
        QString id() const;

        //export to dbus
        Q_SCRIPTABLE QString GetID() const;
        Q_SCRIPTABLE void Update(QVariantMapMap updates);
        Q_SCRIPTABLE void Delete();
        Q_SCRIPTABLE QVariantMapMap GetSettings() const;
        Q_SCRIPTABLE QVariantMap GetSecrets(const QString &setting_name, const QStringList &hints, bool request_new);

    Q_SIGNALS:
        Q_SCRIPTABLE void Updated(QMap<QString, QMap<QString, QVariant> >);
        Q_SCRIPTABLE void Removed();
    private:
        QString mId;
        QMap<QString, QMap<QString, QVariant> > mSettingsMap;

        //ConnectionType connType;
        //WiredConnectionSetting *wired;
};

#endif
