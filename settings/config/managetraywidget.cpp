/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>
Copyright 2010 Sebastian Kügler <sebas@kde.org>
Copyright 2011-2012 Lamarque V. Souza <lamarque@kde.org>

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

#include <QHBoxLayout>
#include <QDBusInterface>
#include <QDBusPendingReply>

#include <KAboutData>
#include <KLocale>
#include <KMessageBox>
#include <KPluginFactory>

#include "knmserviceprefs.h"
#include "paths.h"
#include "secretstorage.h"
#include "../../plasma_nm_version.h"
#include <tooltips.h>


K_PLUGIN_FACTORY( ManageTrayWidgetFactory, registerPlugin<ManageTrayWidget>();)
K_EXPORT_PLUGIN( ManageTrayWidgetFactory( "kcm_networkmanagement_tray", "libknetworkmanager" ) )

ManageTrayWidget::ManageTrayWidget(QWidget *parent, const QVariantList &args)
: KCModule( ManageTrayWidgetFactory::componentData(), parent, args )
{
    KAboutData* ab = new KAboutData("kcm_networkmanagement", "networkmanagement", ki18n("Manage Connections"), plasmaNmVersion,
        ki18n("Manage Connections Control Panel Module"),
        KAboutData::License_GPL, ki18n("(c) 2008-2009 Will Stephenson<p>(c) 2010 Sebastian Kügler</p><p>(c) 2011-2012 Lamarque V. Souza</p>"));

    ab->addAuthor(ki18n("Lamarque V. Souza"), ki18n("Developer and Maintainer"), "lamarque@kde.org");
    ab->addAuthor(ki18n("Ilia Kats"), ki18n("Developer"));
    ab->addAuthor(ki18n("Rajeesh K Nambiar"), ki18n("Developer"));
    ab->addAuthor(ki18n("Will Stephenson"), ki18n("Developer"), "wstephenson@kde.org");
    ab->addAuthor(ki18n("Sebastian Kügler"), ki18n("Developer"), "sebas@kde.org");
    setAboutData(ab);

    KNetworkManagerServicePrefs::instance(Knm::NETWORKMANAGEMENT_RCFILE);
    secretStorageMode = KNetworkManagerServicePrefs::self()->secretStorageMode();

    QHBoxLayout * layout = new QHBoxLayout(this);
    ui = new OtherSettingsWidget(this);
    layout->addWidget(ui);
    setLayout(layout);

    // KConfigXT magic
    addConfig(KNetworkManagerServicePrefs::self(), ui);

    setButtons(KCModule::Help | KCModule::Apply);
}

ManageTrayWidget::~ManageTrayWidget()
{
}

void ManageTrayWidget::save()
{
    KNetworkManagerServicePrefs::self()->writeConfig();
    KCModule::save();

    if (secretStorageMode != KNetworkManagerServicePrefs::self()->secretStorageMode()) {
        if (!SecretStorage::switchStorage((SecretStorage::SecretStorageMode)secretStorageMode, (SecretStorage::SecretStorageMode)KNetworkManagerServicePrefs::self()->secretStorageMode())) {
            KMessageBox::error(this, i18nc("@info", "Error switching storage type"));
        }
    }
    secretStorageMode = KNetworkManagerServicePrefs::self()->secretStorageMode();
    // To make the plasmoid reread the "Show network interface using:" property.
    QDBusInterface dbus("org.kde.kded", "/org/kde/networkmanagement", "org.kde.networkmanagement");
    dbus.asyncCall(QLatin1String("ReadConfig"));
}
