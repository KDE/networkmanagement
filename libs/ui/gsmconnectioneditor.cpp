/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

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

#include "gsmconnectioneditor.h"

#include <QMap>

#include <KDebug>
#include <KGlobal>
#include <KLocale>

#include "gsmwidget.h"
#include "pppwidget.h"
#include "ipv4widget.h"
#include "ipv6widget.h"
#include "connectionwidget.h"

#include "connection.h"

GsmConnectionEditor::GsmConnectionEditor(const QVariantList &args, QWidget *parent)
: ConnectionPreferences(args, parent)
{
    Q_ASSERT(args.count());
    QString connectionId = args[0].toString();
    m_connection = new Knm::Connection(QUuid(connectionId), Knm::Connection::Gsm);
    m_contents->setConnection(m_connection);
    GsmWidget * gsmWidget = new GsmWidget(m_connection, this);
    PppWidget * pppWidget = new PppWidget(m_connection, this);
    IpV4Widget * ipV4Widget = new IpV4Widget(m_connection, this);
    IpV6Widget * ipV6Widget = new IpV6Widget(m_connection, this);

    if (args.count() > 1) {
        if (args.count() > 2) {
            QList<QVariant> networkIds = args[2].toList();
            if (!networkIds.isEmpty()) {
                gsmWidget->setNetworkIds(networkIds);
            }
        }

        if (args.count() > 3) {
            QMap<QString, QVariant> apnInfo = args[3].toMap();
            if (apnInfo["name"].isNull()) {
                m_contents->setDefaultName(args[1].toString());
            } else {
                m_contents->setDefaultName(args[1].toString() + " - " + apnInfo["name"].toString());
            }
            gsmWidget->setApnInfo(apnInfo);
    
            if (!apnInfo["dnsList"].isNull()) {
                ipV4Widget->setDns(apnInfo["dnsList"].toList());
            }
        } else {
            m_contents->setDefaultName(args[1].toString());
        }
    } else {
        m_contents->setDefaultName(i18n("New Cellular Connection"));
    }

    addToTabWidget(gsmWidget);
    addToTabWidget(pppWidget);
    addToTabWidget(ipV4Widget);
    addToTabWidget(ipV6Widget);
}

GsmConnectionEditor::GsmConnectionEditor(Knm::Connection *con, QWidget *parent)
: ConnectionPreferences(QVariantList(), parent)
{
    if (!con)
    {
        kDebug() << "Connection pointer is NULL, creating a new connection.";
        m_connection = new Knm::Connection(QUuid::createUuid(), Knm::Connection::Gsm);
    }
    else
        m_connection = con;

    QString connectionId = m_connection->uuid().toString();

    m_contents->setConnection(m_connection);
    GsmWidget * gsmWidget = new GsmWidget(m_connection, this);
    PppWidget * pppWidget = new PppWidget(m_connection, this);
    IpV4Widget * ipV4Widget = new IpV4Widget(m_connection, this);
    IpV6Widget * ipV6Widget = new IpV6Widget(m_connection, this);

    addToTabWidget(gsmWidget);
    addToTabWidget(pppWidget);
    addToTabWidget(ipV4Widget);
    addToTabWidget(ipV6Widget);
}
GsmConnectionEditor::~GsmConnectionEditor()
{
}

// vim: sw=4 sts=4 et tw=100
