/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#ifndef KNM_INTERNALS_CONNECTION_H
#define KNM_INTERNALS_CONNECTION_H

#include <QDateTime>
#include <QList>
#include <QString>
#include <QUuid>

#include <KUser>
#include <solid/control/networkinterface.h>

#include "setting.h"

#include "knminternals_export.h"
#include <NetworkManager.h>

namespace Knm
{

class KNMINTERNALS_EXPORT Connection
{

public:
    enum Type { Unknown = 0, Wired, Wireless, Gsm, Cdma, Vpn, Pppoe, Bluetooth };
    static QString typeAsString(Connection::Type);
    static Connection::Type typeFromString(const QString & type);
    static Connection::Type typeFromSolidType(const Solid::Control::NetworkInterfaceNm09 * iface);
    static QString iconName(const Connection::Type type);
    /**
     * Create a connection with a new Uuid
     */
    Connection(const QString & name, Connection::Type type, NMBluetoothCapabilities bt_cap = NM_BT_CAPABILITY_DUN);
    /**
     * Create a connection with a given Uuid
     */
    explicit Connection(const QUuid& uuid, Connection::Type type, NMBluetoothCapabilities bt_cap = NM_BT_CAPABILITY_DUN );
    explicit Connection(Connection *con);
    virtual ~Connection();

    QString name() const;
    QString iconName() const;
    QUuid uuid() const;
    Connection::Type type() const;
    bool autoConnect() const;
    QString zone() const;
    bool isShared() const;
    QDateTime timestamp() const;
    QHash<QString,QString> permissions() const;

    QString origin() const;
    void setOrigin(const QString &);

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
    void setIconName(const QString &);
    void setUuid(const QUuid &);
    void setAutoConnect(bool);
    void setZone(const QString &);
    void setTimestamp(const QDateTime&);

    /**
     * Sets connection type. If type to be set and the type of the connection is already the
     * same, then this method will do nothing, otherwise m_settings will be cleared and
     * m_type will be set as given parameter.
     * @param type Type of the connection to be set
     * @param bt_cap Bluetooth Capability (DUN or PAN) if @type is Knm::Connection::Bluetooth
     */
    void setType(Connection::Type type, NMBluetoothCapabilities bt_cap = NM_BT_CAPABILITY_DUN);

    void setPermissions(const QHash<QString,QString>&);
    void addToPermissions(const QString&, const QString&);
    void removeFromPermissions(const QString&);

    /**
     *  Syntactic sugar for setTimestamp(QDateTime::currentDateTime())
     */
    void updateTimestamp();

    /**
     * Check if any of the settings in this connection have secrets
     */
    bool hasPersistentSecrets() const;

    /**
     * Check if this connection has volatile secrets; if yes,
     * ask user every time secrets are requested
     * Connections which have no volatile secrets always return false
     */
    bool hasVolatileSecrets() const;

    QStringList hasPersistentSecretsSettings() const;

private:
    /**
     * Set up internal structure, including all settings specific to this connection type
     */
    void init(NMBluetoothCapabilities bt_cap = NM_BT_CAPABILITY_DUN);
    void init(Connection *con);

    /**
     * Add a setting to this connection.  The connection will delete the Setting
     * @return false if the setting can not use the given Setting type or if it already contains a
     * Setting of this type.
     */
    void addSetting(Setting*);
    void clearSettings();

    QString m_name;
    QString m_iconName;
    QUuid m_uuid;
    Connection::Type m_type;
    bool m_autoConnect;
    QString m_zone;
    QDateTime m_timestamp;
    QString m_origin;
    QList<Setting*> m_settings;
    QHash<QString,QString> m_permissions;
};
} // namespace Knm

#endif // CONNECTION_H
