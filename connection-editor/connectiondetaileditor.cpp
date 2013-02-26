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
#include "connectionwidget.h"
#include "wiredconnectionwidget.h"
#include "wificonnectionwidget.h"

#include <QtNetworkManager/settings.h>
#include <QtNetworkManager/activeconnection.h>

using namespace NetworkManager;

ConnectionDetailEditor::ConnectionDetailEditor(Settings::ConnectionSettings* connection, QDialog* parent, Qt::WindowFlags f):
    QDialog(parent, f),
    m_detailEditor(new Ui::ConnectionDetailEditor),
    m_connection(connection)
{
    m_detailEditor->setupUi(this);

    initTabs();

    if (connection->id().isEmpty()) {
        setWindowTitle(i18n("New Connection (%1)", connection->typeAsString(connection->connectionType())));
        m_detailEditor->connectionName->setText(i18n("New %1 connection", connection->typeAsString(connection->connectionType())));
    }
    else {
        setWindowTitle(i18n("Edit Connection '%1'", connection->id()));
        m_detailEditor->connectionName->setText(connection->id());
    }

    connect(this, SIGNAL(accepted()), SLOT(saveSetting()));
}

ConnectionDetailEditor::~ConnectionDetailEditor()
{
}

void ConnectionDetailEditor::initTabs()
{
    ConnectionWidget * connectionWidget = new ConnectionWidget(m_connection);
    m_detailEditor->tabWidget->addTab(connectionWidget, i18n("General"));

    foreach (Settings::Setting * setting, m_connection->settings()) {
        addTab(setting->type());
    }
}

void ConnectionDetailEditor::addTab(Settings::Setting::SettingType type)
{
    /*Adsl, Cdma, Gsm, Infiniband, Ipv4, Ipv6, Ppp, Pppoe, Security8021x, Serial,
      Vpn, Wired, Wireless, WirelessSecurity, Bluetooth, OlpcMesh, Vlan, Wimax, Bond, Bridge, BridgePort;*/
    if (type == Settings::Setting::Wired) {
        WiredConnectionWidget * wiredWidget = new WiredConnectionWidget(m_connection->setting(type), this);
        m_detailEditor->tabWidget->addTab(wiredWidget, i18n("Wired"));
    } else if (type == Settings::Setting::Wireless) {
        WifiConnectionWidget * wifiWidget = new WifiConnectionWidget(m_connection->setting(type), this);
        m_detailEditor->tabWidget->addTab(wifiWidget, i18n("Wireless"));
    }
}

void ConnectionDetailEditor::saveSetting()
{
    ConnectionWidget * connectionWidget = static_cast<ConnectionWidget*>(m_detailEditor->tabWidget->widget(0));

    QVariantMapMap settings = connectionWidget->setting();

    for (int i = 1; i < m_detailEditor->tabWidget->count(); ++i) {
        SettingWidget * widget = static_cast<SettingWidget*>(m_detailEditor->tabWidget->widget(i));
        settings.insert(widget->type(), widget->setting());
    }

    m_connection->fromMap(settings);
    m_connection->setId(m_detailEditor->connectionName->text());
    m_connection->setUuid(QUuid::createUuid().toString().mid(1, QUuid::createUuid().toString().length() - 2));
}

