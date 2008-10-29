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

#include "vpnpreferences.h"

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

VpnPreferences::VpnPreferences(QWidget *parent, const QVariantList &args)
: ConnectionPreferences( KGlobal::mainComponent(), parent, args )
{
    QString connectionId = args[0].toString();
    m_connectionType = "Vpn";
    // check if connection is gsm or cdma and set the appropriate widget
    QString vpnType = args[1].toString();
    bool cdma = false, gsm = false;

    QVBoxLayout * layout = new QVBoxLayout(this);
    m_contents = new ConnectionWidget(connectionId, this);
    layout->addWidget(m_contents);
    PppWidget * pppWidget = new PppWidget(connectionId, this);
    // Must setup initial widget before adding its contents, or all child widgets are added in this
    // run
    addConfig(m_contents->configXml(), m_contents);
    // load the plugin in vpnType, get its SettingWidget and add it

//    addToTabWidget(m_connectionTypeWidget);
    addToTabWidget(pppWidget);
}

VpnPreferences::~VpnPreferences()
{
}

void VpnPreferences::load()
{
    ConnectionPreferences::load();
}

void VpnPreferences::save()
{
    ConnectionPreferences::save();
}

// vim: sw=4 sts=4 et tw=100
