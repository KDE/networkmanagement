/*
    Copyright 2012-2013  Jan Grulich <jgrulich@redhat.com>

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

#include "connectiondetaileditor.h"
#include "ui_connectiondetaileditor.h"
#include "wiredconnectionwidget.h"

#include <QtGui/QTreeWidgetItem>

#include <QtNetworkManager/settings.h>
#include <QtNetworkManager/connection.h>
#include <QtNetworkManager/activeconnection.h>

using namespace NetworkManager;

ConnectionDetailEditor::ConnectionDetailEditor(Settings::ConnectionSettings* connection, QDialog* parent, Qt::WindowFlags f):
    QDialog(parent, f),
    m_detailEditor(new Ui::ConnectionDetailEditor),
    m_connection(connection)
{
    m_detailEditor->setupUi(this);

    initTabs();
}

ConnectionDetailEditor::~ConnectionDetailEditor()
{
}

void ConnectionDetailEditor::initTabs()
{
    foreach (Settings::Setting * setting, m_connection->settings()) {
        addTab(setting->type());
    }
}

void ConnectionDetailEditor::addTab(Settings::Setting::SettingType type)
{
    /*Adsl, Cdma, Gsm, Infiniband, Ipv4, Ipv6, Ppp, Pppoe, Security8021x, Serial,
      Vpn, Wired, Wireless, WirelessSecurity, Bluetooth, OlpcMesh, Vlan, Wimax, Bond, Bridge, BridgePort;*/

    switch (type) {
        case Settings::Setting::Wired:
            WiredConnectionWidget * wiredWidget = new WiredConnectionWidget(m_connection->setting(type));
            m_detailEditor->tabWidget->addTab(wiredWidget, i18n("Wired"));
            break;
    }
}


