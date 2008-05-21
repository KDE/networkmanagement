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
    m_savedSecurityType = EncryptionSettingsWidget::None;

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
    connect(m_essid, SIGNAL(textChanged(const QString&)), this, SLOT(onEssidChanged(const QString&)));
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
    delete m_encryptionWidget;
    delete m_encryptdlg;
    delete m_mainLayout;
}

QString WirelessSettingsWidget::wirelessInterface() const
{
    return m_wirelessInterface;
}

void WirelessSettingsWidget::setWirelessInterface(const QString &uni)
{
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
    if (m_securityType->currentIndex() != 0) {
        KConfigGroup encConfig(&config, "Encryption");
        m_encryptionWidget->saveConfig(encConfig);
    }
}

void WirelessSettingsWidget::loadConfig(const KConfigGroup &config)
{
    kDebug() << "Loading wireless settings.";
    m_essid->setText(config.readEntry("ESSID", QString()));
    m_connectionType->setCurrentIndex(config.readEntry("WirelessConnectionType", 0));
    m_wirelessMode->setCurrentIndex(config.readEntry("WirelessMode", 0));
    m_securityType->setCurrentIndex(config.readEntry("WirelessSecurityType",0));
    if (m_securityType->currentIndex() != 0 && config.hasGroup("Encryption")) {
        //create the encryption widget and enable the settings button if applicable
        createEncryptionWidget();
        m_securitySettingsButton->setEnabled(m_securityType->currentIndex() != EncryptionSettingsWidget::None);
        
        KConfigGroup encConfig(&config, "Encryption");
        m_encryptionWidget->loadConfig(encConfig);
    }
}

bool WirelessSettingsWidget::isValid() const
{
    if (m_essid->text().isEmpty()) {
        kDebug() << "ESSID is empty.";
        return false;
    }
    if (m_securityType->currentIndex() != 0 && m_securityType->currentIndex() != (int)m_savedSecurityType) {
        kDebug() << "Security Type does not match encryption.";
        return false;
    }
    return true;
}

void WirelessSettingsWidget::onScanClicked()
{
    if (m_scandlg == 0) {
        m_scanWidget = new ScanWidget();
        m_scanWidget->setWirelessInterface(m_wirelessInterface);
        
        m_scandlg = new KDialog();
        m_scandlg->setButtons( KDialog::Ok | KDialog::Cancel);
        m_scandlg->setCaption(i18n("Available Access Points"));
        m_scandlg->setMainWidget(m_scanWidget);
        m_scandlg->resize(640, 420);

        connect(m_scandlg, SIGNAL(okClicked()), this, SLOT(onApChosen()));
    }
    m_scandlg->show();
}

void WirelessSettingsWidget::onApChosen()
{
    m_essid->setText(m_scanWidget->currentAccessPoint());
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
    emit validationChanged(isValid());
}

void WirelessSettingsWidget::onEncryptClicked()
{
    createEncryptionWidget();
    m_encryptdlg->show();
}

void WirelessSettingsWidget::onEncryptionSet()
{
    kDebug() << "Encryption was accepted.";
    m_savedSecurityType = m_encryptionWidget->type();
    bool valid = isValid();
    kDebug() << "Widget is valid: " << valid;
    emit validationChanged(valid);
}

void WirelessSettingsWidget::onEssidChanged(const QString &text)
{
    Q_UNUSED(text)
            
    emit validationChanged(isValid());
}

void WirelessSettingsWidget::createEncryptionWidget()
{
    if (m_encryptdlg == 0) {
        m_encryptdlg = new KDialog();
        m_encryptdlg->setButtons( KDialog::Ok | KDialog::Cancel);
        m_encryptdlg->setCaption(i18n("Encryption Settings"));
    }
    if (m_encryptionWidget == 0) {
        if (m_securityType->currentIndex() == EncryptionSettingsWidget::Wep) {
            m_encryptionWidget = new WepSettingsWidget(m_encryptdlg);
        } else if (m_securityType->currentIndex() == EncryptionSettingsWidget::Wpa) {
            m_encryptionWidget = new WpaSettingsWidget(m_encryptdlg);
        }

        m_encryptdlg->setMainWidget(m_encryptionWidget);
        connect(m_encryptdlg, SIGNAL(okClicked()), this, SLOT(onEncryptionSet()));
        connect(m_encryptionWidget, SIGNAL(validationChanged(bool)), m_encryptdlg, SLOT(enableButtonOk(bool)));
    } else if (m_securityType->currentIndex() != m_encryptionWidget->type()) {
        //clean up
        disconnect(m_encryptionWidget, SIGNAL(validationChanged(bool)), m_encryptdlg, SLOT(enableButtonOk(bool)));
        delete m_encryptionWidget;
        
        if (m_securityType->currentIndex() == EncryptionSettingsWidget::Wep) {
            m_encryptionWidget = new WepSettingsWidget(m_encryptdlg);
        } else if (m_securityType->currentIndex() == EncryptionSettingsWidget::Wpa) {
            m_encryptionWidget = new WpaSettingsWidget(m_encryptdlg);
        }
        
        m_encryptdlg->setMainWidget(m_encryptionWidget);
        connect(m_encryptionWidget, SIGNAL(validationChanged(bool)), m_encryptdlg, SLOT(enableButtonOk(bool)));
    }
    
    m_encryptdlg->enableButtonOk(m_encryptionWidget->isValid());
}

#include "wirelesssettingswidget.moc"
