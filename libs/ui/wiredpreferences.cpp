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

#include "wiredpreferences.h"

#include <nm-setting-wired.h>

#include <QFile>

#include <KDebug>
#include <KGlobal>
#include <KLocale>
#include <KPluginFactory>

#include "wiredwidget.h"
#include "ipv4widget.h"
#include "ipv6widget.h"
#include "connectionwidget.h"
#include "security/securitywired8021x.h"
#include "settings/ipv4.h"

#include "connection.h"

WiredPreferences::WiredPreferences(const QVariantList &args, QWidget *parent)
: ConnectionPreferences(args, parent)
{
    QString connectionId = args[0].toString();
    m_connection = new Knm::Connection(QUuid(connectionId), Knm::Connection::Wired);

    if (args.count() > 1 && args[1].toString() == QLatin1String("shared")) {
        static_cast<Knm::Ipv4Setting *>(m_connection->setting(Knm::Setting::Ipv4))->setMethod(Knm::Ipv4Setting::EnumMethod::Shared);
        m_contents->setConnection(m_connection);
        m_contents->setDefaultName(i18n("Shared Wired Connection"));
    } else {
        m_contents->setConnection(m_connection);
        m_contents->setDefaultName(i18n("New Wired Connection"));
    }

    prepareSettings();
}

WiredPreferences::WiredPreferences(Knm::Connection *con, QWidget *parent)
: ConnectionPreferences(QVariantList(), parent)
{
    if (!con)
    {
        kDebug() << "Connection pointer is NULL, creating a new connection.";
        m_connection = new Knm::Connection(QUuid::createUuid(), Knm::Connection::Wired);
    }
    else
        m_connection = con;

    QString connectionId = m_connection->uuid().toString();

    m_contents->setConnection(m_connection);
    //m_contents->setDefaultName(i18n("New Wired Connection"));

    prepareSettings();

}

WiredPreferences::~WiredPreferences()
{
}

void WiredPreferences::prepareSettings()
{

    WiredWidget * wiredWidget = new WiredWidget(m_connection, this);
    IpV4Widget * ipv4Widget = new IpV4Widget(m_connection, this);
    IpV6Widget * ipv6Widget = new IpV6Widget(m_connection, this);
    SecurityWired8021x * securityWidget = new SecurityWired8021x(m_connection, this);

    addToTabWidget(ipv4Widget);
    addToTabWidget(ipv6Widget);
    addToTabWidget(wiredWidget);
    addToTabWidget(securityWidget);
}

// vim: sw=4 sts=4 et tw=100
