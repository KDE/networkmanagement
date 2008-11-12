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

KNetworkManagerService::KNetworkManagerService(QObject * parent, const QVariantList&) : KDEDModule(parent), m_active(true)
{
    if ( !QDBusConnection::systemBus().registerService( "org.freedesktop.NetworkManagerUserSettings" ) ) {
        // trouble;
        kDebug() << "Unable to register service";
        m_active = false;
    }

    connect( QDBusConnection::sessionBus().interface(), SIGNAL(serviceOwnerChanged(const QString&, const QString&, const QString & ) ), SLOT(serviceOwnerChanged(const QString&, const QString&, const QString & ) ) );

    mNetworkSettings = new NetworkSettings(this);
    KConfigToService * kConfigConverter = new KConfigToService(mNetworkSettings, m_active);
    kConfigConverter->init();
}

KNetworkManagerService::~KNetworkManagerService()
{
    if ( !QDBusConnection::systemBus().unregisterService( "org.freedesktop.NetworkManagerUserSettings" ) ) {
        // trouble;
        kDebug() << "Unable to unregister service";
    }

}

void KNetworkManagerService::serviceOwnerChanged( const QString& service,const QString& oldOwner, const QString& newOwner )
{
    Q_UNUSED( oldOwner );
    if ( !newOwner.isEmpty() && service == "org.freedesktop.NetworkManager" ) {
        kDebug() << "NetworkManager restarted!";
    }
}

#include "service.moc"
