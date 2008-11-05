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

#ifndef WIRELESS_802_11_SECURITY_WIDGET_H
#define WIRELESS_802_11_SECURITY_WIDGET_H

#include "settingwidget.h"

#include <QVariant>

#include "knm_export.h"

class KNM_EXPORT Wireless80211SecurityWidget : public SettingWidget
{
Q_OBJECT
public:
    Wireless80211SecurityWidget(const QString& connectionId, 
            /*Add flags here for AP and device caps so security options offered can be filtered*/
            QWidget * parent = 0 );
    virtual ~Wireless80211SecurityWidget();
    QString settingName() const;
    void readConfig();
    void writeConfig();
    QVariantMap secrets() const;
    static const QString KEY_MGMT_NONE;
    static const QString KEY_MGMT_802_1X;
    static const QString KEY_MGMT_WPA_NONE;
    static const QString KEY_MGMT_WPA_PSK;
    static const QString KEY_MGMT_WPA_EAP;
protected Q_SLOTS:
    void securityTypeChanged(int index);
private:
    class Private;
    Private * d;
};

#endif // 802_11_WIRELESS_SECURITY_WIDGET_H
