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

#include "802_11_wirelesswidget.h"

Wireless80211Widget::Wireless80211Widget(const QString& connectionId, QWidget * parent) : SettingWidget(connectionId, parent)
{
    m_ui.setupUi(this);
    init();
}

Wireless80211Widget::~Wireless80211Widget()
{
}

QString Wireless80211Widget::label() const
{
    return i18nc("Label for wireless network setting", "Wireless");
}

QString Wireless80211Widget::settingName() const
{
    return QLatin1String("802-11-wireless");
}

// vim: sw=4 sts=4 et tw=100
