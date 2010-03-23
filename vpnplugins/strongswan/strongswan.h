/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2010 Maurus Rohrer <maurus.rohrer@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KNM4_STRONGSWAN_H
#define KNM4_STRONGSWAN_H

#include "vpnuiplugin.h"

#include <QVariant>

namespace Knm
{
    class Connection;
} // namespace Knm

class StrongswanUiPlugin : public VpnUiPlugin
{
Q_OBJECT
public:
    StrongswanUiPlugin(QObject * parent = 0, const QVariantList& = QVariantList());
    virtual ~StrongswanUiPlugin();
    SettingWidget * widget(Knm::Connection *, QWidget * parent = 0);
};

#endif //  KNM4_STRONGSWAN_H
