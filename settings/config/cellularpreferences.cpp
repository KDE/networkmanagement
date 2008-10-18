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
    QString connectionId = args[0].toString();
    m_connectionType = "Cellular";
    // check if connection is gsm or cdma and set the appropriate widget
    bool cdma = false, gsm = false;
    foreach (Solid::Control::NetworkInterface * iface, Solid::Control::NetworkManager::networkInterfaces()) {
        switch (iface->type()) {
            case Solid::Control::NetworkInterface::Gsm:
                gsm = true;
                break;
            case Solid::Control::NetworkInterface::Cdma:
                cdma = true;
                break;
        }
    }
    if (cdma && gsm) {
        kDebug() << "TODO: ask user what type of cellular connection to add, since we have both hardware";
        Q_ASSERT(0);
    }
    QVBoxLayout * layout = new QVBoxLayout(this);
    m_contents = new ConnectionWidget(connectionId, this);
    layout->addWidget(m_contents);
    //CellularWidget * cellularWidget = new CellularWidget(connectionId, this);
    PppWidget * pppWidget = new PppWidget(connectionId, this);
    // Must setup initial widget before adding its contents, or all child widgets are added in this
    // run
    addConfig(m_contents->configXml(), m_contents);
    if (gsm && !cdma) {
        m_connectionTypeWidget = new GsmWidget(connectionId, this);
    }
    if (!gsm && cdma) {
        m_connectionTypeWidget = new CdmaWidget(connectionId, this);
    }
    addToTabWidget(m_connectionTypeWidget);
    //addToTabWidget(cellularWidget);
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
