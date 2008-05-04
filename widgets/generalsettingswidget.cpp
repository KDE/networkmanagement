/*  This file is part of the KDE project
    Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "generalsettingswidget.h"
#include "networkprofile.h"

#include <KDebug>

GeneralSettingsWidget::GeneralSettingsWidget(QWidget *parent)
    : QWidget(parent)
{
    m_connectionTypes << i18n("Best Available") << i18n("Wireless") << i18n("Wired");
    m_connectionTypeIcons << "best-available" << "network-wireless" << "network-wired";
    //initialize elements
    m_mainLayout = new QGridLayout(this);

    m_profileNameLabel = new QLabel(i18n("Location Name"));
    m_profileName = new QLineEdit();
    m_profileType = new QComboBox();
    foreach (const QString &type, NetworkProfile::types()) {
        m_profileType->addItem(KIcon(NetworkProfile::icon(type)), type);
    }
    
    m_connectionTypeLabel = new QLabel(i18n("Type of Connection"));
    m_connectionType = new QComboBox();
    for (int index=0; index < m_connectionTypes.size(); index++) {
        m_connectionType->addItem(KIcon(m_connectionTypeIcons[index]), m_connectionTypes[index]);
    }

    m_priorityList = new QListView();
    m_ifaceModel = new IfaceItemModel();
    m_priorityList->setModel(m_ifaceModel);

    m_upButton = new QPushButton(KIcon("arrow-up"), QString());
    m_downButton = new QPushButton(KIcon("arrow-down"), QString());

    //layout items
    m_mainLayout->addWidget(m_profileNameLabel, 0, 0, 1, 2);
    m_mainLayout->addWidget(m_profileName, 1, 0, 1, 2);
    m_mainLayout->addWidget(m_profileType, 1, 3);
    m_mainLayout->addWidget(m_connectionTypeLabel, 3, 0, 1, 2);
    m_mainLayout->addWidget(m_connectionType, 4, 0, 1, 2);
    m_mainLayout->addWidget(m_priorityList, 6, 0, 3, 2);
    m_mainLayout->addWidget(m_upButton, 6, 3);
    m_mainLayout->addWidget(m_downButton, 7, 3);

    connect(m_connectionType, SIGNAL(activated(int)), this, SLOT(onConnectionTypeChanged(int)));
}

GeneralSettingsWidget::~GeneralSettingsWidget()
{
}

void GeneralSettingsWidget::onConnectionTypeChanged(int index)
{
    kDebug() << "Connection Type changed to: " << index;
    switch (index) {
        case 0:
            m_ifaceModel->filter((IfaceItemModel::FilterTypes)(IfaceItemModel::Ieee8023 | IfaceItemModel::Ieee80211));
            break;
        case 1:
            m_ifaceModel->filter(IfaceItemModel::Ieee80211);
            break;
        case 2:
            m_ifaceModel->filter(IfaceItemModel::Ieee8023);
        default:
            break;
    }
    return;
}

#include "generalsettingswidget.moc"
