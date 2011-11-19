/*
Copyright 2011 Lamarque Souza <lamarque@gmail.com>

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

#include <nm-setting-bluetooth.h>

#include "bluetoothconnectioneditor.h"

#include <QMap>

#include <KDebug>
#include <KGlobal>
#include <KLocale>

#include "internals/settings/bluetooth.h"
#include "bluetoothwidget.h"
#include "gsmwidget.h"
#include "ipv4widget.h"
#include "pppwidget.h"
#include "connectionwidget.h"

#include "connection.h"

#include <uiutils.h>

using namespace Knm;

BluetoothConnectionEditor::BluetoothConnectionEditor(const QVariantList &args, QWidget *parent)
: ConnectionPreferences(args, parent)
{
    Q_ASSERT(args.count());
    QString connectionId = args[0].toString();
    if (args.count() > 2) {
        if (args[2].toString() == NM_SETTING_BLUETOOTH_TYPE_PANU)
            m_connection = new Knm::Connection(QUuid(connectionId), Knm::Connection::Bluetooth, NM_BT_CAPABILITY_NAP);
        else
            m_connection = new Knm::Connection(QUuid(connectionId), Knm::Connection::Bluetooth, NM_BT_CAPABILITY_DUN);
    } else {
        m_connection = new Knm::Connection(QUuid(connectionId), Knm::Connection::Bluetooth);
    }
    m_contents->setConnection(m_connection);
    BluetoothSetting *b = static_cast<BluetoothSetting *>(m_connection->setting(Setting::Bluetooth));
    BluetoothWidget * bluetoothWidget = new BluetoothWidget(m_connection, this);
    IpV4Widget * ipV4Widget = new IpV4Widget(m_connection, this);
    QString defaultName;

    // Bluetooth part
    if (args.count() > 3) {
        b->setNetworkname(args[1].toString());
        b->setNetworktype(args[2].toString());
        b->setBdaddr(UiUtils::macAddressFromString(args[3].toString()));
        defaultName = i18n("%1 Network", args[1].toString());
    } else {
        defaultName = i18n("New Bluetooth Connection");
    }

    if (b->networktype() == NM_SETTING_BLUETOOTH_TYPE_PANU) {
        m_contents->setDefaultName(defaultName);
        addToTabWidget(bluetoothWidget);
        addToTabWidget(ipV4Widget);
        return;
    }

    // Gsm part for DUN
    GsmWidget * gsmWidget = new GsmWidget(m_connection, this);
    PppWidget * pppWidget = new PppWidget(m_connection, this);

    if (args.count() > 1) {
        defaultName = args[1].toString();
    }

    if (args.count() > 4) {
        defaultName += " " + args[4].toString();

        if (args.count() > 5) {
            QList<QVariant> networkIds = args[5].toList();
            if (!networkIds.isEmpty()) {
                gsmWidget->setNetworkIds(networkIds);
            }
        }

        if (args.count() > 6) {
            QMap<QString, QVariant> apnInfo = args[6].toMap();
            gsmWidget->setApnInfo(apnInfo);

            if (!apnInfo["dnsList"].isNull()) {
                ipV4Widget->setDns(apnInfo["dnsList"].toList());
            }

            if (!apnInfo["name"].isNull()) {
                defaultName += " - " + apnInfo["name"].toString();
            }
        }
    }

    m_contents->setDefaultName(defaultName);
    addToTabWidget(bluetoothWidget);
    addToTabWidget(gsmWidget);
    addToTabWidget(pppWidget);
    addToTabWidget(ipV4Widget);
}

BluetoothConnectionEditor::BluetoothConnectionEditor(Knm::Connection *con, QWidget *parent)
: ConnectionPreferences(QVariantList(), parent)
{
    if (!con)
    {
        kDebug() << "Connection pointer is NULL, creating a new connection.";
        m_connection = new Knm::Connection(QUuid::createUuid(), Knm::Connection::Bluetooth);
    }
    else
        m_connection = con;

    m_contents->setConnection(m_connection);
    BluetoothSetting *b = static_cast<BluetoothSetting *>(m_connection->setting(Setting::Bluetooth));

    BluetoothWidget * bluetoothWidget = new BluetoothWidget(m_connection, this);
    IpV4Widget * ipv4Widget = new IpV4Widget(m_connection, this);

    if (b->networktype() == NM_SETTING_BLUETOOTH_TYPE_PANU) {
        addToTabWidget(bluetoothWidget);
        addToTabWidget(ipv4Widget);
    } else if (b->networktype() == NM_SETTING_BLUETOOTH_TYPE_DUN){
        GsmWidget * gsmWidget = new GsmWidget(m_connection, this);
        PppWidget * pppWidget = new PppWidget(m_connection, this);
        addToTabWidget(bluetoothWidget);
        addToTabWidget(gsmWidget);
        addToTabWidget(pppWidget);
        addToTabWidget(ipv4Widget);
    }
}
BluetoothConnectionEditor::~BluetoothConnectionEditor()
{
}

// vim: sw=4 sts=4 et tw=100
