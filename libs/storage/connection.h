/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

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

#ifndef CONNECTION_H
#define CONNECTION_H

#include <QDateTime>
#include <QList>
#include <QString>
#include <QUuid>

#include "setting.h"

#include "knm_export.h"

class KNM_EXPORT Connection
{
public:
    enum Type { Wired, Wireless, Gsm, Cdma, Vpn, Pppoe };
    static QString typeAsString(Connection::Type);
    static Connection::Type typeFromString(const QString & type);

    /**
     * Create a connection with a new Uuid
     */
    Connection(const QString & name, Connection::Type type);
    /**
     * Create a connection with a given Uuid
     */
    Connection(const QUuid& uuid, Connection::Type type);
    virtual ~Connection();

    QString name() const;
    QUuid uuid() const;
    Connection::Type type() const;
    bool autoConnect() const;
    QDateTime timestamp() const;

    /**
     * Access all settings
     */
    QList<Setting*> settings() const;
    /**
     * Access a specific setting
     * @param the type of setting to retrieve
     * @return 0 if this Connection does not contain the given Setting type
     */
    Setting * setting(Setting::Type type) const;

    void setName(const QString &);
    void setAutoConnect(bool);
    void setTimestamp(const QDateTime&);
    /**
     *  Syntactic sugar for setTimestamp(QDateTime::currentDateTime())
     */
    void updateTimestamp();

    /**
     * Add a setting to this connection
     * @return false if the setting can not use the given Setting type or if it already contains a
     * Setting of this type.
     */
    bool addSetting(Setting*);
private:
    QString m_name;
    QUuid m_uuid;
    Connection::Type m_type;
    bool m_autoConnect;
    QDateTime m_timestamp;

    QList<Setting*> m_settings;
};

#endif // CONNECTION_H
