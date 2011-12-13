/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#include "securitydynamicwep.h"
#include "securityeap_p.h"

SecurityDynamicWep::SecurityDynamicWep(Knm::Connection* connection, QWidget * parent)
    : SecurityEap(connection, true /*wireless*/, parent)
{

}

SecurityDynamicWep::~SecurityDynamicWep()
{

}

void SecurityDynamicWep::writeConfig()
{
    Q_D(SecurityEap);
    SecurityEap::writeConfig();

    QStringList pairwise = d->settingSecurity->pairwise();

    if (!pairwise.contains("wep40"))
        pairwise << "wep40";

    if (!pairwise.contains("wep104"))
        pairwise << "wep104";

    QStringList group = d->settingSecurity->group();

    if(!group.contains("wep40"))
        group << "wep40";

    if (!group.contains("wep104"))
        group << "wep104";

    d->settingSecurity->setPairwise(pairwise);
    d->settingSecurity->setGroup(group);
}

void SecurityDynamicWep::syncWidgetData(const QPair<QString, QString> &widgetData)
{
    Q_UNUSED(widgetData);
}

QPair<QString, QString> SecurityDynamicWep::widgetData()
{
    return QPair<QString, QString>();
}

// vim: sw=4 sts=4 et tw=100
