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

#include "secrets.h"
#include "secretsadaptor.h"
#include "marshallarguments.h"

#include <NetworkManager.h>
#include <nm-setting-wireless-security.h>

Secrets::Secrets(QObject *parent)
    : QObject(parent)
{
    qDBusRegisterMetaType< QMap<QString, QMap<QString, QDBusVariant> > >();
    qDBusRegisterMetaType< QMap<QString, QDBusVariant> >();

    new SecretsAdaptor(this);
}

Secrets::~Secrets()
{
}

bool Secrets::loadSettings(const KConfigGroup &group)
{
    QString authType = group.readEntry("AuthenticationType", QString("WEP"));

    if (authType == "WEP") {
        secrets[
    } else if ( authType == "WPA") {
        //TODO
        return false;
    } else {
        return false;
    }
}

QString Secrets::objectPath()
{
    return QString(NM_DBUS_PATH_SETTINGS_CONNECTION);
}

QMap<QString, QMap<QString, QVariant> > Secrets::GetSecrets(QString setting_name, QStringList hints, bool request_new)
{
    //stored variabled for a delayed response
    this->settingName = setting_name;
    this->hints = hints;
    this->requestNew = request_new;
    
    conn = connection();
    msg = message();
    setDelayedReply(true);
    QMetaObject::invokeMethod(this, "processRequest", Qt::QueuedConnection);
    return QMap<QString, QMap<QString, QDBusVariant> >();
}

void Secrets::processRequest()
{
    return secrets;
}
