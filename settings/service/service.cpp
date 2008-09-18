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

TestService::TestService() : KMainWindow( 0 )
{
    QWidget * window = new QWidget(this);
    QVBoxLayout * layout = new QVBoxLayout;

    if ( !QDBusConnection::systemBus().registerService( "org.freedesktop.NetworkManagerUserSettings" ) ) {
        kDebug() << "Unable to register service";
        QLabel * label = new QLabel("Unable to register service", window);
        layout->addWidget(label);
    }
    QPushButton * configure = new QPushButton(QLatin1String("Configure"), window );
    QPushButton * quit = new QPushButton(QLatin1String("Quit"), window );
    connect(configure,SIGNAL(clicked()),this,SLOT(configure()));
    connect(quit,SIGNAL(clicked()),this,SLOT(close()));
    layout->addWidget(configure);
    layout->addWidget(quit);
    window->setLayout(layout);

    setCentralWidget( window );

    connect( QDBusConnection::sessionBus().interface(), SIGNAL(serviceOwnerChanged(const QString&, const QString&, const QString & ) ), SLOT(serviceOwnerChanged(const QString&, const QString&, const QString & ) ) );

    setCaption( QLatin1String("NetworkManager 0.7 Test Service") );
    mNetworkSettings = new NetworkSettings(this);
    KConfigToService * kConfigConverter = new KConfigToService(mNetworkSettings);
    kConfigConverter->init();
}

TestService::~TestService()
{
}

void TestService::serviceOwnerChanged( const QString& service,const QString& oldOwner, const QString& newOwner )
{
    Q_UNUSED( oldOwner );
    if ( !newOwner.isEmpty() && service == "org.freedesktop.NetworkManager" ) {
        kDebug() << "NetworkManager restarted!";
    }
}

void TestService::configure()
{
    KDialog configDialog(this);
#if 1
    QList<KService::Ptr> services = KServiceTypeTrader::self()->query( "KCModule", "'knetworkmanager' in [X-KDE-ParentComponents]" );
    kDebug() << "found " << services.count() << " config plugins";
    KCModuleProxy *mp;
    foreach (const KService::Ptr &servicePtr, services) {
        kDebug() << "showing kcm for " << servicePtr->storageId();
        KCModuleInfo moduleInfo(servicePtr);
        QStringList args;
        args << "testconfigxml";
        mp = new KCModuleProxy(moduleInfo, this, args);
        mp->load();
        configDialog.setMainWidget(mp);
        break;
    }
#else
    KService::Ptr service = KService::serviceByDesktopName("kcm_networkmanager");
    if (service) {
        kDebug() << "Found service!";
    } else {
        kDebug() << "service kcm_networkmanager not found";
    }
#endif


    if ( configDialog.exec() == QDialog::Accepted ) {
        mp->save();
    }
}

static const char description[] =
    I18N_NOOP("Test Service for Network Status kded module");

static const char version[] = "v0.1";

int main( int argc, char** argv )
{
    KAboutData about("KNetworkManagerTestService", 0, ki18n("knetworkmanagertestservice"), version, ki18n(description), KAboutData::License_GPL, ki18n("(C) 2008 Will Stephenson"), KLocalizedString(), 0, "wstephenson@kde.org");
    about.addAuthor( ki18n("Will Stephenson"), KLocalizedString(), "wstephenson@kde.org" );
    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

    TestService * test = new TestService;
    test->show();
    return app.exec();
}

#include "service.moc"
