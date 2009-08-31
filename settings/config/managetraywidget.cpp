/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

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

#include "managetraywidget.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QHBoxLayout>
#include <QTabWidget>

#include <KLocale>
#include <KPluginFactory>
#include <KPluginInfo>
#include <KServiceTypeTrader>
#include <KStandardDirs>
#include <KToolInvocation>

#include "knmserviceprefs.h"
#include <tooltips.h>

#include "menutooltipsettingswidget.h"
#include "traysettingswidget.h"
#include "ui_storage.h"

K_PLUGIN_FACTORY( ManageTrayWidgetFactory, registerPlugin<ManageTrayWidget>();)
K_EXPORT_PLUGIN( ManageTrayWidgetFactory( "kcm_networkmanagementtray" ) )

ManageTrayWidget::ManageTrayWidget(QWidget *parent, const QVariantList &args)
: KCModule( ManageTrayWidgetFactory::componentData(), parent, args ), mTraySettingsWidget(0)
{
    QHBoxLayout * layout = new QHBoxLayout(this);
    QTabWidget * tabWidget = new QTabWidget(this);
    layout->addWidget(tabWidget);
    setLayout(layout);

    KGlobal::locale()->insertCatalog("libknmui");
    KNetworkManagerServicePrefs::instance(Knm::ConnectionPersistence::NETWORKMANAGEMENT_RCFILE);
    mTraySettingsWidget  = new TraySettingsWidget(this);

    QStringList selectedKeys = KNetworkManagerServicePrefs::self()->toolTipKeys();

    mMenuToolTipSettingsWidget = new MenuToolTipSettingsWidget(Knm::ToolTips::allKeys(), selectedKeys, this);
    connect(mMenuToolTipSettingsWidget, SIGNAL(changed()), SLOT(otherSettingsChanged()));

    connect(mTraySettingsWidget, SIGNAL(changed()), SLOT(otherSettingsChanged()));

    tabWidget->addTab(mTraySettingsWidget, i18nc("@title:tab tab containing general UI settings", "&Tray Icon"));

    QWidget * storageWidget = new QWidget(this);
    Ui_Storage * ui_storage = new Ui_Storage;
    ui_storage->setupUi(storageWidget);

    tabWidget->addTab(storageWidget, i18nc("@title:tab connection storage settings", "Connection &Secrets"));

    tabWidget->addTab(mMenuToolTipSettingsWidget, i18nc("@title:tab tab containing menu tooltip settings", "&Tool Tips"));

    // KConfigXT magic
    addConfig(KNetworkManagerServicePrefs::self(), mTraySettingsWidget);
    addConfig(KNetworkManagerServicePrefs::self(), storageWidget);

    setButtons(KCModule::Help | KCModule::Apply);
}

ManageTrayWidget::~ManageTrayWidget()
{
}

void ManageTrayWidget::save()
{
    if (mTraySettingsWidget) {
        QList<uint> iconInterfaceAllocations = mTraySettingsWidget->iconInterfaceAllocations();
        KNetworkManagerServicePrefs::self()->setIconCount(iconInterfaceAllocations.count());
        for (int i = 0; i < iconInterfaceAllocations.count(); ++i) {
            KNetworkManagerServicePrefs::self()->setIconTypes(i, iconInterfaceAllocations.at(i));
        }
    }

    KNetworkManagerServicePrefs::self()->setToolTipKeys(mMenuToolTipSettingsWidget->toolTipKeys());

    KNetworkManagerServicePrefs::self()->writeConfig();
    KCModule::save();
    QDBusInterface remoteApp("org.kde.knetworkmanager", "/tray",
                                       "org.kde.knetworkmanager");
    if (remoteApp.isValid()) {
        remoteApp.call("reloadConfig");
    } else if (KNetworkManagerServicePrefs::self()->autostart()) {
        KToolInvocation::kdeinitExec("knetworkmanager");
    }
}

void ManageTrayWidget::otherSettingsChanged()
{
    emit changed();
}

