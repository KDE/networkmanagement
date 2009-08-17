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

#include <QVBoxLayout>
#include <QFile>

#include <KDebug>
#include <KGlobal>
#include <KLocale>
#include <KPluginFactory>

#include "wiredwidget.h"
#include "ipv4widget.h"
#include "connectionwidget.h"
#include "security/securitywired8021x.h"

#include "connection.h"

WiredPreferences::WiredPreferences(QWidget *parent, const QVariantList &args)
: ConnectionPreferences( KGlobal::mainComponent(), parent, args )
{
    QString connectionId = args[0].toString();
    m_connection = new Knm::Connection(QUuid(connectionId), Knm::Connection::Wired);
    QVBoxLayout * layout = new QVBoxLayout(this);
    m_contents = new ConnectionWidget(m_connection, i18n("New Wired Connection"), this);
    layout->addWidget(m_contents);
    WiredWidget * wiredWidget = new WiredWidget(m_connection, this);
    IpV4Widget * ipv4Widget = new IpV4Widget(m_connection, this);
    SecurityWired8021x * securityWidget = new SecurityWired8021x(m_connection, this);

    addToTabWidget(wiredWidget);
    addToTabWidget(ipv4Widget);
    addToTabWidget(securityWidget);
}

WiredPreferences::~WiredPreferences()
{
}

// vim: sw=4 sts=4 et tw=100
