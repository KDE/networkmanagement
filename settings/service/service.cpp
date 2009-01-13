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

#include "service.h"

#include <NetworkManager.h>

#include <QtGui>
#include <QtDBus>

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KDebug>
#include <KDialog>
#include <KLocale>
#include <KCModuleInfo>
#include <KCModuleProxy>
#include <KServiceTypeTrader>

#include "kconfigtoservice.h"
#include "networksettings.h"

K_PLUGIN_FACTORY(KNetworkManagerServiceFactory,
                 registerPlugin<KNetworkManagerService>();
    )
K_EXPORT_PLUGIN(KNetworkManagerServiceFactory("knetworkmanager"))

const QString KNetworkManagerService::SERVICE_USER_SETTINGS = QLatin1String(NM_DBUS_SERVICE_USER_SETTINGS);

KNetworkManagerService::KNetworkManagerService(QObject * parent, const QVariantList&) : KDEDModule(parent), m_active(true)
{
    if ( !QDBusConnection::systemBus().interface()->registerService( SERVICE_USER_SETTINGS, QDBusConnectionInterface::QueueService, QDBusConnectionInterface::AllowReplacement ) ) {
        // trouble;
        kDebug() << "Unable to register service" << QDBusConnection::systemBus().lastError();
        m_active = false;
    }
    kDebug() << "registered" << SERVICE_USER_SETTINGS;

    connect(QDBusConnection::systemBus().interface(), SIGNAL(serviceRegistered(const QString&)),
            SLOT(serviceRegistered(const QString&)));
    connect(QDBusConnection::systemBus().interface(), SIGNAL(serviceUnregistered(const QString&)),
            SLOT(serviceUnregistered(const QString&)));
    connect( QDBusConnection::systemBus().interface(),
            SIGNAL(serviceOwnerChanged(const QString&, const QString&, const QString & ) ),
            SLOT(serviceOwnerChanged(const QString&, const QString&, const QString & ) ) );

    mNetworkSettings = new NetworkSettings(this);
    ( void ) new KConfigToService(mNetworkSettings, m_active);
    // don't init, wait for start
}

KNetworkManagerService::~KNetworkManagerService()
{
    kDebug();
    if ( !QDBusConnection::systemBus().unregisterService( "org.freedesktop.NetworkManagerUserSettings" ) ) {
        // trouble;
        kDebug() << "Unable to unregister service";
    }
    delete mNetworkSettings;
}

void KNetworkManagerService::serviceOwnerChanged( const QString& service,const QString& oldOwner, const QString& newOwner )
{
    Q_UNUSED( oldOwner );
    if ( !newOwner.isEmpty() && service == "org.freedesktop.NetworkManager" ) {
        kDebug() << "NetworkManager restarted!";
    }
    if (newOwner.isEmpty() && service == SERVICE_USER_SETTINGS && !m_active) {
        kDebug() << "User settings service was released, trying to register it ourselves";
        if (QDBusConnection::systemBus().interface()->registerService(SERVICE_USER_SETTINGS, QDBusConnectionInterface::QueueService, QDBusConnectionInterface::AllowReplacement)) {
            m_active = true;
        }
    }
}

void KNetworkManagerService::serviceRegistered(const QString & name)
{
    if (name == SERVICE_USER_SETTINGS) {
        kDebug() << "service registered";
        m_active = true;
    }
}

void KNetworkManagerService::serviceUnregistered(const QString & name)
{
    if (name == SERVICE_USER_SETTINGS) {
        kDebug() << "service lost, queueing reregistration";
        QDBusConnection::systemBus().interface()->registerService( SERVICE_USER_SETTINGS, QDBusConnectionInterface::QueueService, QDBusConnectionInterface::AllowReplacement );
        m_active = false;
    }
}

#include "service.moc"
