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

#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QVariant>
#include <QMap>
#include <QString>

//DBus specific includes
#include <QtDBus/QtDBus>
#include <QDBusObjectPath>

#include "marshalarguments.h"

class Client : public QObject
{
    Q_OBJECT

    public:
        Client(QObject *parent=0);
        ~Client();

        QStringList connections() const;
        QVariantMapMap settings(const QString &connPath) const;

    public Q_SLOTS:
        void onNewConnection(const QString &name);

    private:
        QDBusConnection m_bus;
        QDBusInterface *m_settings;
};

#endif
