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


#include <KDebug>
#include <KGlobal>
#include <KLocale>

#include "gsmwidget.h"
#include "pppwidget.h"
#include "connectionwidget.h"

#include "connection.h"

GsmConnectionEditor::GsmConnectionEditor(const QVariantList &args, QWidget *parent)
: ConnectionPreferences(args, parent)
{
    Q_ASSERT(args.count());
    QString connectionId = args[0].toString();
    m_connection = new Knm::Connection(QUuid(connectionId), Knm::Connection::Gsm);
    m_contents->setConnection(m_connection);
    m_contents->setDefaultName(i18n("New Cellular Connection"));

    GsmWidget * gsmWidget = new GsmWidget(m_connection, this);
    PppWidget * pppWidget = new PppWidget(m_connection, this);

    addToTabWidget(gsmWidget);
    addToTabWidget(pppWidget);
}

GsmConnectionEditor::~GsmConnectionEditor()
{
}

// vim: sw=4 sts=4 et tw=100
