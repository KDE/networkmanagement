/*
 * Copyright 2011 Sebastian Kügler <sebas@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License version 2 as
 * published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "networkmanagementjob.h"

#include <kdebug.h>

NetworkManagementJob::NetworkManagementJob(const QString &id, const QString &operation, QMap<QString, QVariant> &parameters, QObject *parent) :
    ServiceJob(parent->objectName(), operation, parameters, parent),
    m_id(id)
{
}

NetworkManagementJob::~NetworkManagementJob()
{
}

void NetworkManagementJob::start()
{
    const QString operation = operationName();
    const QString connId = parameters()["ConnectionId"].toString();

    kDebug() << "starting operation" << operation << "on the connection" << connId;

    if (operation == "activate") {
        // Gaga ... connnect to that network.
        return;
    }
    setResult(false);
}

#include "networkmanagementjob.moc"
