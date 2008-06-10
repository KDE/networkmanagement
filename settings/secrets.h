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

#ifndef SECRETS_H
#define SECRETS_H

#include <QObject>
#include <QVariant>
#include <QMap>
#include <QString>

//DBus specific includes
#include <QDBusObjectPath>
#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusMessage>

#include <KConfigGroup>

class Secrets : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("Secrets Interface", "org.freedesktop.NetworkManagerSettings.Connection.Secrets")

    public:
        Secrets(QObject *parent=0);
        ~Secrets();

        bool loadSettings(const KConfigGroup &group);

        Q_SCRIPTABLE QMap<QString, QMap<QString, QVariant> > GetSecrets(QString setting_name, QStringList hints, bool request_new);

    private:
        Q_INVOKABLE void processRequest();
        KConfigGroup config;
        QMap<QString, QMap<QString, QVariant> > secrets;

        //dbus objects
        QDBusConnection conn;
        QDBusMessage message;

        //stored settings for the delayed response
        QString settingName;
        QStringList hints;
        bool requestNew;
};

#endif
