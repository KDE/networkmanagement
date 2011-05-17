/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2009 Sebastian Kügler <sebas@kde.org>

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

#ifndef HIDDENWIRELESSNETWORKITEM_H
#define HIDDENWIRELESSNETWORKITEM_H

#include "wirelessnetwork.h"
#include "wirelessnetworkitem.h"

class QGraphicsLinearLayout;

namespace Plasma
{
    class IconWidget;
    class LineEdit;
}

class HiddenWirelessNetworkItem : public ActivatableItem
{
Q_OBJECT

    public:
        HiddenWirelessNetworkItem(QGraphicsWidget *parent = 0);
        virtual ~HiddenWirelessNetworkItem();
        void setupItem();
        QString ssid() const;
        void setSsid(const QString & ssid);
        int strength() const;
        Solid::Control::AccessPointNm09 * referenceAccessPoint() const;

    public Q_SLOTS:
        /*
        * called if the user hits Esc or the popup is hidden
        * safe to call repeatedly
        */
        void resetSsidEntry();
    private Q_SLOTS:
        /*
        * switch into SSID entry mode
        * safe to call repeatedly
        */
        void connectClicked();
        // called if the user hits Enter
        void ssidEntered();

    Q_SIGNALS:
        void connectToHiddenNetwork(const QString&);

    private:
        static QString s_defaultText;
        QGraphicsLinearLayout* m_layout;
        Plasma::IconWidget* m_connect;
        Plasma::LineEdit* m_ssidEdit;
        QString m_ssid;
};
#endif // HIDDENWIRELESSNETWORKITEM_H
