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

#include "wirelesssettingswidget.h"

#include <KDebug>

WirelessSettingsWidget::WirelessSettingsWidget(QWidget *parent)
    : QWidget(parent),
      m_mainLayout(0),
      m_wirelessConfigLabel(0),
      m_essidLabel(0),
      m_connectionTypeLabel(0),
      m_wirelessModeLabel(0),
      m_securityTypeLabel(0),
      m_essid(0),
      m_connectionType(0),
      m_wirelessMode(0),
      m_securityType(0),
      m_scanButton(0),
      m_securitySettingsButton(0),
      m_scandlg(0),
      m_encryptdlg(0),
      m_scanView(0),
      m_scanModel(0),
      m_scanDelegate(0),
      m_scanSelectionModel(0),
      m_encryptionWidget(0)
{
    m_connectionTypes << i18n("Managed") << i18n("Adhoc");
    m_wirelessModes << i18n("Auto") << i18n("802.11a") << i18n("802.11b") << i18n("802.11g") << i18n("802.11n");
    m_securityTypes << i18n("None") << i18n("WEP") << i18n("WPA");

    //initialize elements
    m_mainLayout = new QGridLayout(this);

    m_wirelessConfigLabel = new QLabel(i18n("Wireless Network Configuration"));
    m_essidLabel = new QLabel(i18n("ESSID"));
    m_essid = new QLineEdit();
    m_scanButton = new QPushButton(i18n("Scan"));

    m_connectionTypeLabel = new QLabel(i18n("ConnectionType"));
    m_connectionType = new QComboBox();
    m_connectionType->addItems(m_connectionTypes);
    
    m_wirelessModeLabel = new QLabel(i18n("Wireless Mode"));
    m_wirelessMode = new QComboBox();
    m_wirelessMode->addItems(m_wirelessModes);
    
    m_securityTypeLabel = new QLabel(i18n("Security Type"));
    m_securityType = new QComboBox();
    m_securityType->addItems(m_securityTypes);
    m_securitySettingsButton = new QPushButton(i18n("Settings"));
    m_securitySettingsButton->setEnabled(false);

    //layout items
    //left pane
    m_mainLayout->addWidget(m_wirelessConfigLabel, 0, 0, 1, 3);
    m_mainLayout->addWidget(m_essidLabel, 1, 0);
    m_mainLayout->addWidget(m_essid, 1, 1, 1, 2);
    m_mainLayout->addWidget(m_scanButton, 2, 2);
    m_mainLayout->addWidget(m_connectionTypeLabel, 3, 0, 1, 3);
    m_mainLayout->addWidget(m_connectionType, 4, 1, 1, 2);
    m_mainLayout->addWidget(m_wirelessModeLabel, 5, 0, 1, 3);
    m_mainLayout->addWidget(m_wirelessMode, 6, 1, 1, 2);

    //right pane
    m_mainLayout->addWidget(m_securityTypeLabel, 0, 4, 1 , 3);
    m_mainLayout->addWidget(m_securityType, 1, 5, 1, 2);
    m_mainLayout->addWidget(m_securitySettingsButton, 2, 6);

    m_mainLayout->setColumnMinimumWidth(3, 40);
    m_mainLayout->setColumnStretch(3, 10);

    connect(m_securityType, SIGNAL(activated(int)), this, SLOT(onSecurityTypeChanged(int)));
    connect(m_scanButton, SIGNAL(clicked()), this, SLOT(onScanClicked()));
    connect(m_securitySettingsButton, SIGNAL(clicked()), this, SLOT(onEncryptClicked()));
}

WirelessSettingsWidget::~WirelessSettingsWidget()
{
    delete m_wirelessConfigLabel;
    delete m_essidLabel;
    delete m_connectionTypeLabel;
    delete m_wirelessModeLabel;
    delete m_securityTypeLabel;
    delete m_essid;
    delete m_connectionType;
    delete m_wirelessMode;
    delete m_securityType;
    delete m_scanButton;
    delete m_securitySettingsButton;
    delete m_scandlg;
    delete m_encryptdlg;
    delete m_scanView;
    delete m_scanModel;
    delete m_scanDelegate;
    delete m_scanSelectionModel;
    delete m_encryptionWidget;
    delete m_mainLayout;
}

QString WirelessSettingsWidget::wirelessInterface() const
{
    return m_wirelessInterface;
}

void WirelessSettingsWidget::setWirelessInterface(const QString &uni)
{
    Solid::Control::NetworkInterface iface(uni);
    if (!iface.isValid()) {
        kDebug() << "Interface was invalid.";
        return;
    } else if (iface.type() != Solid::Control::NetworkInterface::Ieee80211) {
        kDebug() << "Interface was not of type IEEE 80211.";
    }
    
    m_wirelessInterface = uni;
}

void WirelessSettingsWidget::enableAdhoc(bool enable)
{
    if (!enable && m_connectionType->count() == 2) {
        m_connectionType->removeItem(1);
    } else if(m_connectionType->count() == 1) {
        m_connectionType->insertItem(1, m_connectionTypes[1]);
    }
}

void WirelessSettingsWidget::saveConfig(KConfigGroup &config)
{
    kDebug() << "Saving Wireless Config.";
    config.writeEntry("ESSID", m_essid->text());
    config.writeEntry("WirelessConnectionType", m_connectionType->currentIndex());
    config.writeEntry("WirelessMode", m_wirelessMode->currentIndex());
    config.writeEntry("WirelessSecurityType", m_securityType->currentIndex());
}

void WirelessSettingsWidget::onScanClicked()
{
    if (m_scandlg == 0) {
        //setup scanview if it doesn't already exist
        m_scanView = new ApItemView();
        m_scanModel = new ApItemModel(m_wirelessInterface);
        m_scanDelegate = new ApItemDelegate(m_scanView);
        m_scanSelectionModel = new QItemSelectionModel(m_scanModel);
        m_scanModel->init();
        m_scanView->setModel(m_scanModel);
        m_scanView->setItemDelegate(m_scanDelegate);
        m_scanView->setSelectionModel(m_scanSelectionModel);
        
        m_scandlg = new KDialog();
        m_scandlg->setButtons( KDialog::Ok | KDialog::Cancel);
        m_scandlg->setCaption(i18n("Available Access Points"));
        m_scandlg->setMainWidget(m_scanView);

        connect(m_scandlg, SIGNAL(okClicked()), this, SLOT(onApChosen()));
    }
    m_scandlg->show();
}

void WirelessSettingsWidget::onApChosen()
{
    QModelIndex index = m_scanView->currentIndex();
    if (!index.isValid()) {
        return;
    }
    m_essid->setText(index.data().toString());
}

void WirelessSettingsWidget::onSecurityTypeChanged(int index)
{
    switch (index) {
        case 0:
            m_securitySettingsButton->setEnabled(false);
            break;
        case 1:
        case 2:
            m_securitySettingsButton->setEnabled(true);
    }
}

void WirelessSettingsWidget::onEncryptClicked()
{
    if (m_encryptdlg == 0) {
        m_encryptionWidget = new WepSettingsWidget(m_encryptdlg);
        m_encryptdlg = new KDialog();
        m_encryptdlg->setButtons( KDialog::Ok | KDialog::Cancel);
        m_encryptdlg->setCaption(i18n("Encryption Settings"));
        m_encryptdlg->setMainWidget(m_encryptionWidget);
        connect(m_encryptdlg, SIGNAL(okClicked()), this, SLOT(onEncryptionSet()));
    }
    m_encryptdlg->show();
}

void WirelessSettingsWidget::onEncryptionSet()
{
    kDebug() << "Encryption was accepted.";
}

#include "wirelesssettingswidget.moc"
