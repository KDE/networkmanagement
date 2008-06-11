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
    m_upButton->setEnabled(false);
    m_downButton = new QPushButton(KIcon("arrow-down"), QString());
    m_downButton->setEnabled(false);

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
    connect(m_priorityList, SIGNAL(activated(const QModelIndex&)), this, SLOT(onPriorityListActivated(const QModelIndex&)));
    connect(m_priorityList->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(onCurrentChanged(const QModelIndex&, const QModelIndex&)));
    connect(m_upButton, SIGNAL(clicked()), this, SLOT(onUpButtonClicked()));
    connect(m_downButton, SIGNAL(clicked()), this, SLOT(onDownButtonClicked()));
    connect(m_profileName, SIGNAL(textChanged(const QString&)), this, SLOT(onDataEntered(const QString&)));
}

GeneralSettingsWidget::~GeneralSettingsWidget()
{
}

void GeneralSettingsWidget::setExistingProfiles(const QStringList profiles)
{
    m_existingProfiles = profiles;
}

QString GeneralSettingsWidget::profileName() const
{
    return m_profileName->text();
}

bool GeneralSettingsWidget::wiredProfile() const
{
    return (m_connectionType->currentIndex() == Wired);
}

void GeneralSettingsWidget::saveConfig(KConfigGroup &config)
{
    kDebug() << "Saving General Config.";
    config.writeEntry("ProfileType", m_profileType->currentIndex());
    config.writeEntry("ConnectionType", m_connectionType->currentIndex());

    //get the list of interfaces
    QAbstractItemModel *ifaceModel = m_priorityList->model();
    QStringList ifaceList, ifaceTypeList, ifaceNameList;
    for(int index=0; index < ifaceModel->rowCount(); index++) {
        ifaceList << ifaceModel->data(ifaceModel->index(index,0), IfaceItemModel::UniRole).toString();
        ifaceTypeList << ifaceModel->data(ifaceModel->index(index,0)).toString();
        ifaceNameList << ifaceModel->data(ifaceModel->index(index,2)).toString();
    }
    config.writeEntry("InterfaceList", ifaceList);
    config.writeEntry("InterfaceTypeList", ifaceTypeList);
    config.writeEntry("InterfaceNameList", ifaceNameList);
}

void GeneralSettingsWidget::loadConfig(const KConfigGroup &config)
{
    m_profileName->setText(config.name());
    m_profileType->setCurrentIndex(config.readEntry("ProfileType", 0));
    m_connectionType->setCurrentIndex(config.readEntry("ConnectionType", 0));

    if(!wiredProfile()) {
        m_wifiSettings->loadConfig(config);
    }
}

void GeneralSettingsWidget::setStaticMode(bool disable)
{
    m_profileName->setEnabled(!disable);
    m_connectionType->setEnabled(!disable);
}

bool GeneralSettingsWidget::isValid() const
{
    if (m_profileName->text().isEmpty()) {
        kDebug() << "Profile name is empty.";
        return false;
    }
    if (m_existingProfiles.contains(m_profileName->text())) {
        kDebug() << "Profile already exists.";
        return false;
    }
    return true;
}

void GeneralSettingsWidget::onConnectionTypeChanged(int index)
{
    switch (index) {
        case 0: //Best Available
            m_ifaceModel->filter((IfaceItemModel::FilterTypes)(IfaceItemModel::Ieee8023 | IfaceItemModel::Ieee80211));
            m_wifiSettings->setWirelessInterface(m_ifaceModel->priorityInterface(IfaceItemModel::Ieee80211));
            m_wifiSettings->enableAdhoc(false);
            emit wirelessAppropriate(true);
            break;
        case 1: // Wireless
            m_ifaceModel->filter(IfaceItemModel::Ieee80211);
            m_wifiSettings->setWirelessInterface(m_ifaceModel->priorityInterface(IfaceItemModel::Ieee80211));
            m_wifiSettings->enableAdhoc(true);
            emit wirelessAppropriate(true);
            break;
        case 2: // Wired
            m_ifaceModel->filter(IfaceItemModel::Ieee8023);
            emit wirelessAppropriate(false);
        default:
            break;
    }
    m_priorityList->setCurrentIndex(QModelIndex());
}

void GeneralSettingsWidget::onPriorityListActivated(const QModelIndex &index)
{
    m_priorityList->setCurrentIndex(m_ifaceModel->index(index.row(), 0));
}

void GeneralSettingsWidget::onCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)

    //enable and disable if appropriate
    m_upButton->setEnabled(true);
    m_downButton->setEnabled(true);
    if (!current.isValid()) {
        m_upButton->setEnabled(false);
        m_downButton->setEnabled(false);
        return;
    }
    if(current.row() == 0) {
        m_upButton->setEnabled(false);
    }
    if(current.row() == m_ifaceModel->rowCount()-1) {
        m_downButton->setEnabled(false);
    }
}

void GeneralSettingsWidget::onUpButtonClicked()
{
    QModelIndex index = m_priorityList->currentIndex();
    //kDebug() << "Index row " << index.row() << " with data " << index.data() << " was clicked.";
    if (index.row() > 0) {
        m_ifaceModel->moveIndexUp(index);
        m_priorityList->setCurrentIndex(m_ifaceModel->index(index.row()-1, 0));
    }
}

void GeneralSettingsWidget::onDownButtonClicked()
{
    QModelIndex index = m_priorityList->currentIndex();
    //kDebug() << "Index row " << index.row() << " with data " << index.data() << " was clicked.";
    if (index.row() != m_ifaceModel->rowCount()-1) {
        m_ifaceModel->moveIndexDown(index);
        m_priorityList->setCurrentIndex(m_ifaceModel->index(index.row()+1, 0));
    }
}

void GeneralSettingsWidget::onDataEntered(const QString &text)
{
    Q_UNUSED(text)

    emit validationChanged(isValid());
}

void GeneralSettingsWidget::setWirelessSettings(WirelessSettingsWidget *wifiSettings)
{
    m_wifiSettings = wifiSettings;
    m_wifiSettings->setWirelessInterface(m_ifaceModel->priorityInterface(IfaceItemModel::Ieee80211));
    m_wifiSettings->enableAdhoc(false);
}

#include "generalsettingswidget.moc"
