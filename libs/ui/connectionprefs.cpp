/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "connectionprefs.h"

#include <QFile>
#include <KTabWidget>
#include <KDebug>
#include <KStandardDirs>

#include "connectionwidget.h"

#include "connection.h"
#include "connectionpersistence.h"
#include "knmserviceprefs.h"

ConnectionPreferences::ConnectionPreferences(const KComponentData& cdata, QWidget * parent, const QVariantList & args)
    : KCModule( cdata, parent, args ),
      m_contents(0)
{

}

ConnectionPreferences::~ConnectionPreferences()
{
}

Knm::Connection * ConnectionPreferences::connection() const
{
    return m_connection;
}

void ConnectionPreferences::addSettingWidget(SettingInterface* iface)
{
    m_settingWidgets.append(iface);
}

void ConnectionPreferences::addToTabWidget(SettingWidget * widget)
{
    m_contents->connectionSettingsWidget()->addTab(widget, widget->windowTitle());
    m_settingWidgets.append(widget);
}

void ConnectionPreferences::load()
{
    // restore the Connection if possible
    QString connectionFile(KStandardDirs::locateLocal("data",
                QLatin1String("knetworkmanager/connections/") + m_connection->uuid()));
    Knm::ConnectionPersistence cp(m_connection, KSharedConfig::openConfig(connectionFile),
            (KNetworkManagerServicePrefs::self()->storeInWallet()
             ? Knm::ConnectionPersistence::Secure
             : Knm::ConnectionPersistence::PlainText)
            );
    cp.load();
    // and initialise the UI from the Connection
    m_contents->readConfig();
    foreach (SettingInterface * wid, m_settingWidgets) {
        wid->readConfig();
    }
}

void ConnectionPreferences::save()
{
    // save the UI to the Cconnection
    m_contents->writeConfig();
    foreach (SettingInterface * wid, m_settingWidgets) {
        wid->writeConfig();
    }
    // persist the Connection
    QString connectionFile = KStandardDirs::locateLocal("data",
        QLatin1String("knetworkmanager/connections/") + m_connection->uuid());

    Knm::ConnectionPersistence cp(
            m_connection,
            KSharedConfig::openConfig(connectionFile),
            (KNetworkManagerServicePrefs::self()->storeInWallet()
             ? Knm::ConnectionPersistence::Secure
             : Knm::ConnectionPersistence::PlainText)
            );
    cp.save();
}

// vim: sw=4 sts=4 et tw=100


