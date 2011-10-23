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

#ifndef NM09_WIRELESS_CONNECTION_PREFERENCES_H
#define NM09_WIRELESS_CONNECTION_PREFERENCES_H

#include <solid/control/wirelessaccesspoint.h>

#include "connectionprefs.h"

namespace Solid
{
namespace Control
{
class AccessPoint;

} // namespace Control

} // namespace Solid

namespace Knm
{
    class Connection;
}

class Wireless80211Widget;
class WirelessSecuritySettingWidget;

/**
 * Configuration module for wireless connections
 */
class KNM_EXPORT WirelessPreferences : public ConnectionPreferences
{
Q_OBJECT
public:
    /**
     * @param setDefaults if set, the widget and its children should preset any defaults using the
     * information encoded in args
     */
    WirelessPreferences(bool setDefaults, const QVariantList & args = QVariantList(), QWidget * parent = 0);
    WirelessPreferences(Knm::Connection *con, QWidget * parent);
    virtual ~WirelessPreferences();
    virtual bool needsEdits() const;
private Q_SLOTS:
    void tabChanged(int index);
    void setDefaultName(Solid::Control::WirelessNetworkInterfaceNm09 *, Solid::Control::AccessPointNm09 *);
private:
    bool m_hasSecrets;
    int m_securityTabIndex;
    Wireless80211Widget * m_wirelessWidget;
    WirelessSecuritySettingWidget * m_securityWidget;
};

#endif
