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

#include "cellularpreferences.h"

#include <nm-setting-cdma.h>
#include <nm-setting-connection.h>
#include <nm-setting-gsm.h>

#include <QVBoxLayout>
#include <QFile>

#include <KDebug>
#include <KPluginFactory>
#include <KTabWidget>
#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>


#include "configxml.h"
#include "secretstoragehelper.h"
#include "cdmawidget.h"
#include "gsmwidget.h"
#include "pppwidget.h"
#include "connectionwidget.h"

//K_PLUGIN_FACTORY( CellularPreferencesFactory, registerPlugin<CellularPreferences>();)
//K_EXPORT_PLUGIN( CellularPreferencesFactory( "kcm_knetworkmanager_cellular" ) )

CellularPreferences::CellularPreferences(QWidget *parent, const QVariantList &args)
: ConnectionPreferences( KGlobal::mainComponent(), parent, args )
{
    kDebug() << args;
    QString connectionId = args[0].toString();
    m_connectionType = "Cellular";
    QVBoxLayout * layout = new QVBoxLayout(this);
    m_contents = new ConnectionWidget(connectionId, this);
    layout->addWidget(m_contents);
    PppWidget * pppWidget = new PppWidget(connectionId, this);
    // Must setup initial widget before adding its contents, or all child widgets are added in this
    // run
    addConfig(m_contents->configXml(), m_contents);
    QVariant cellularType;
    if (args.count() > 1 ) {
        cellularType = args[1];
    }
    else {
        KConfigSkeletonItem * typeItem = m_contents->configXml()->findItem(m_contents->settingName(), QLatin1String(NM_SETTING_CONNECTION_TYPE));
        if (typeItem) {
            cellularType = typeItem->property().toString();
        }
    }
    Q_ASSERT(cellularType.isValid());

    if (cellularType == QVariant(NM_SETTING_GSM_SETTING_NAME)) {
        m_connectionTypeWidget = new GsmWidget(connectionId, this);
    } else if (cellularType == QVariant(NM_SETTING_CDMA_SETTING_NAME)) {
        m_connectionTypeWidget = new CdmaWidget(connectionId, this);
    } else {
        kDebug() << "passed unrecognised cellular type in ctor args!";
    }
    addToTabWidget(m_connectionTypeWidget);
    addToTabWidget(pppWidget);
}

CellularPreferences::~CellularPreferences()
{
}

void CellularPreferences::load()
{
    ConnectionPreferences::load();
}

void CellularPreferences::save()
{
    ConnectionPreferences::save();
    // this is where tab specific stuff should happen?
    // that should be in the shared config widget code not connection code, as groups are shared.
    // editing existing connections
    // creating new connection
    // popup to prompt for single missing secret
    // interaction between tray and kcm
    //   tray: new connection: launch kcm
    //   tray: Edit connections?
    //   Enable connection - does this need to go through UserSettingsService
    //   Enable wireless
    // interaction between kcm and service
    // interaction between tray and service
    // location of service (in-tray, in plasma)
    //
}

// vim: sw=4 sts=4 et tw=100
