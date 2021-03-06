/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

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

#ifndef WIRELESSSECURITYSETTINGWIDGET_H
#define WIRELESSSECURITYSETTINGWIDGET_H

#include "settingwidget.h"
#include <NetworkManagerQt/wirelessdevice.h>

#include "knm_export.h"

namespace NetworkManager
{
    class AccessPoint;
} // namespace NetworkManager

namespace Knm
{
    class Connection;
} // namespace Knm

class WirelessSecuritySettingWidgetPrivate;

class KNM_EXPORT WirelessSecuritySettingWidget : public SettingWidget
{
Q_OBJECT
Q_DECLARE_PRIVATE(WirelessSecuritySettingWidget)
public:
    /**
     * @param setDefaults if set, the widget sets its individual members according to caps, wpa and
     * rsn
     */
    explicit WirelessSecuritySettingWidget(Knm::Connection * connection,
            const NetworkManager::WirelessDevice::Ptr &iface,
            const NetworkManager::AccessPoint::Ptr ap,
            QWidget * parent = 0 );
    virtual ~WirelessSecuritySettingWidget();
    void readConfig();
    void writeConfig();
    void readSecrets();
public Q_SLOTS:
    void setIfaceAndAccessPoint(const NetworkManager::WirelessDevice::Ptr &iface, const NetworkManager::AccessPoint::Ptr &ap);
protected Q_SLOTS:
    void securityTypeChanged(int);
    void validate();
};

#endif // WIRELESSSECURITYSETTINGWIDGET_H
