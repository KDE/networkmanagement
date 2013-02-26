/*
    Copyright 2012-2013  Jan Grulich <jgrulich@redhat.com>

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

#include "wiredconnectionwidget.h"
#include "ui_wiredconnectionwidget.h"

WiredConnectionWidget::WiredConnectionWidget(NetworkManager::Settings::Setting* setting, QWidget* parent, Qt::WindowFlags f):
    SettingWidget(setting, parent, f),
    m_widget(new Ui::WiredConnectionWidget),
    m_setting(setting)
{
    m_widget->setupUi(this);
    m_widget->speedLabel->setHidden(true);
    m_widget->speed->setHidden(true);
    m_widget->duplexLabel->setHidden(true);
    m_widget->duplex->setHidden(true);
}

WiredConnectionWidget::~WiredConnectionWidget()
{
}

NetworkManager::Settings::Setting* WiredConnectionWidget::setting() const
{
    return m_setting;
}

void WiredConnectionWidget::readConfig()
{
}

void WiredConnectionWidget::writeConfig()
{
}
