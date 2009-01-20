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

#include "connectionpersistence.h"

#include <KConfigGroup>

#include "connection.h"
#include "setting.h"
#include "settingpersistence.h"

ConnectionPersistence::ConnectionPersistence(Connection * conn, KSharedConfig::Ptr config)
    : m_connection(conn), m_config(config)
{
}

ConnectionPersistence::~ConnectionPersistence()
{
    qDeleteAll(m_persistences.values());
}

SettingPersistence * ConnectionPersistence::persistenceFor(Setting * setting)
{
    SettingPersistence * sp = m_persistences.value(setting);
    if (!sp)
    switch (setting->type()) {
        case Setting::Wireless:
//            sp = new WirelessPersistence(static_cast<WirelessSetting*>(setting), m_config);
            break;
        default:
            break;
    }
    if (sp) {
        m_persistences.insert(setting, sp);
    }
    return sp;
}

void ConnectionPersistence::save()
{
    // save connection settings
    KConfigGroup cg(m_config, "connection");
    cg.writeEntry("id", m_connection->name());
    cg.writeEntry("uuid", m_connection->uuid().toString());
    cg.writeEntry("type", Connection::typeAsString(m_connection->type()));
    cg.writeEntry("autoconnect", m_connection->autoConnect());
    cg.writeEntry("timestamp", m_connection->timestamp());

    // save each setting
    foreach (Setting * setting, m_connection->settings()) {
        SettingPersistence * sp = persistenceFor(setting);
        sp->save();
    }
    m_config->sync();
}

void ConnectionPersistence::load()
{
    // load connection settings
    KConfigGroup cg(m_config, "connection");
    m_connection->setName(cg.readEntry("id"));
    m_connection->setAutoConnect(cg.readEntry<bool>("autoconnect", false));
    m_connection->setTimestamp(cg.readEntry<QDateTime>("timestamp", QDateTime()));

    // load each setting
    foreach (Setting * setting, m_connection->settings()) {
        SettingPersistence * sp = persistenceFor(setting);
        sp->load();
    }
}

// vim: sw=4 sts=4 et tw=100
