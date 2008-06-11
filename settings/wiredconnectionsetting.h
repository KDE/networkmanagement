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

#ifndef WIRED_CONNECTION_H
#define WIRED_CONNECTION_H

#include <QObject>
#include <QVariant>
#include <QMap>
#include <QString>

class WiredConnectionSetting : public QObject
{
    Q_OBJECT

    public:
        WiredConnectionSetting(const KConfigGroup &config, QObject *parent=0);
        ~WiredConnectionSetting();

        QMap<QString, QVariant> toMap() const;
        void update(const QMap<QString, QVariant> &updates);
    private:
        QString name;
        QByteArray macAddress;
        uint32 mtu;
};

#endif