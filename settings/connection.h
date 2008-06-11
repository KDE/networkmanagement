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

#include <QObject>
#include <QVariant>
#include <QMap>
#include <QString>

#include <NetworkManager.h>

class Connection : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", NM_DBUS_SERVICE_USER_SETTINGS)

    public:
        enum ConnectionType {Unknown=0. Wired, Wireless};

        Connection(const QString &connName, const KConfigGroup &config, QObject *parent=0);
        ~Connections();

        QString objectPath();

        //export to dbus
        Q_SCRIPTABLE QString GetID() const;
        Q_SCRIPTABLE void Update(QMap<QString, QMap<QString, QVariant> > updates);
        Q_SCRIPTABLE void Delete();
        Q_SCRIPTABLE QMap<QString, QMap<QString, QVariant> > GetSettings() const;

    Q_SIGNALS:
        void Updated(QMap<QString, QMap<QString, QVariant> >);
        void Removed();
    private:
        QMap<QString, QMap<QString, QVariant> > toMap() const;

        QString connName;
        ConnectionType connType;
        KConfigGroup settings;

        WiredConnectionSetting *wired;
};

#endif