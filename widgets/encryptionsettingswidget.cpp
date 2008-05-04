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

#include "encryptionsettingswidget.h"

#include <KDebug>

EncryptionSettingsWidget::EncryptionSettingsWidget(QWidget *parent)
    : QWidget(parent),
      m_config(0)
{
}

EncryptionSettingsWidget::~EncryptionSettingsWidget()
{
}

KConfig* EncryptionSettingsWidget::config() const
{
    return m_config;
}
void EncryptionSettingsWidget::setConfig(KConfig* config)
{
    m_config = config;
}


WepSettingsWidget::WepSettingsWidget(QWidget *parent)
    : EncryptionSettingsWidget(parent),
      m_mainLayout(0)
{
    m_authTypes << i18n("Open") << i18n("Shared");
    m_encTypes << i18n("WEP-64") << i18n("WEP-128") << i18n("CKIP-128") << i18n("CKIP-128");
    m_keyTypes << i18n("ASCII") << i18n("Hex") << i18n("Passphrase");
    
    m_mainLayout = new QVBoxLayout(this);

    //Access Point Authorization
    m_apAuthLayout = new QHBoxLayout();
    m_apAuthLabel = new QLabel(i18n("Access Point Authentication"));
    m_apAuth = new QComboBox();
    m_apAuth->addItems(m_authTypes);
    m_apAuthLabel->setBuddy(m_apAuth);
    m_apAuthLayout->addWidget(m_apAuthLabel);
    m_apAuthLayout->addWidget(m_apAuth);

    //Data Encryption
    m_dataEncLayout = new QHBoxLayout();
    m_dataEncLabel = new QLabel(i18n("Data Encryption"));
    m_dataEnc = new QComboBox();
    m_dataEnc->addItems(m_encTypes);
    m_dataEncLabel->setBuddy(m_dataEnc);
    m_dataEncLayout->addWidget(m_dataEncLabel);
    m_dataEncLayout->addWidget(m_dataEnc);

    //Encryption Key Type
    m_encKeyTypeLayout = new QHBoxLayout();
    m_encKeyTypeLabel = new QLabel(i18n("Encryption Key Type"));
    m_encKeyType = new QComboBox();
    m_encKeyTypeLabel->setBuddy(m_encKeyType);
    m_encKeyType->addItems(m_keyTypes);
    m_encKeyTypeLayout->addWidget(m_encKeyTypeLabel);
    m_encKeyTypeLayout->addWidget(m_encKeyType);

    //static wep keys
    m_securityKeyGroup = new QGroupBox(i18n("Static Wep Keys"));
    m_securityKeyLayout = new QVBoxLayout(m_securityKeyGroup);
    m_keySelectionLayout = new QHBoxLayout();
    m_encryptKeyLabel = new QLabel(i18n("Encrypt Using Key"));
    m_encryptKey = new QComboBox();
    QStringList keys;
    keys << i18n("Key 1") << i18n("Key 2") << i18n("Key 3") << i18n("Key 4");
    m_encryptKey->insertItems(0, keys);
    m_keySelectionLayout->addWidget(m_encryptKeyLabel);
    m_keySelectionLayout->addWidget(m_encryptKey);
    //Key 1
    m_key1Layout = new QHBoxLayout();
    m_key1Label = new QLabel(i18n("Key 1"));
    m_key1Edit = new QLineEdit();
    m_key1Layout->addWidget(m_key1Label);
    m_key1Layout->addWidget(m_key1Edit);
    //Key 2
    m_key2Layout = new QHBoxLayout();
    m_key2Label = new QLabel(i18n("Key 2"));
    m_key2Edit = new QLineEdit();
    m_key2Layout->addWidget(m_key2Label);
    m_key2Layout->addWidget(m_key2Edit);
    //Key 3
    m_key3Layout = new QHBoxLayout();
    m_key3Label = new QLabel(i18n("Key 3"));
    m_key3Edit = new QLineEdit();
    m_key3Layout->addWidget(m_key3Label);
    m_key3Layout->addWidget(m_key3Edit);
    //Key 4
    m_key4Layout = new QHBoxLayout();
    m_key4Label = new QLabel(i18n("Key 4"));
    m_key4Edit = new QLineEdit();
    m_key4Layout->addWidget(m_key4Label);
    m_key4Layout->addWidget(m_key4Edit);
    m_securityKeyLayout->addLayout(m_keySelectionLayout);
    m_securityKeyLayout->addLayout(m_key1Layout);
    m_securityKeyLayout->addLayout(m_key2Layout);
    m_securityKeyLayout->addLayout(m_key3Layout);
    m_securityKeyLayout->addLayout(m_key4Layout);
    m_securityKeyGroup->setLayout(m_securityKeyLayout);

    //passphrase
    m_passphraseWidget = new QWidget();
    m_passphraseLayout = new QGridLayout(m_passphraseWidget);
    m_passphraseLabel = new QLabel(i18n("Passphrase:"));
    m_passphrase = new QLineEdit();
    m_showKey = new QCheckBox(i18n("Show Passphrase"));
    m_showKey->setChecked(false);
    m_passphrase->setEchoMode(QLineEdit::Password);
    m_passphraseLayout->addWidget(m_passphraseLabel,0,0);
    m_passphraseLayout->addWidget(m_passphrase,0,1);
    m_passphraseLayout->addWidget(m_showKey,1,0);
    m_passphraseWidget->setLayout(m_passphraseLayout);

    //add default sections
    m_mainLayout->addLayout(m_apAuthLayout);
    m_mainLayout->addLayout(m_dataEncLayout);
    m_mainLayout->addLayout(m_encKeyTypeLayout);
    m_mainLayout->addWidget(m_securityKeyGroup);

    connect(m_showKey, SIGNAL(stateChanged(int)), this, SLOT(onShowKeyChanged(int)));
    connect(m_encKeyType, SIGNAL(activated(int)), this, SLOT(onEncKeyTypeChanged(int)));
}

WepSettingsWidget::~WepSettingsWidget()
{
    delete m_mainLayout;
    delete m_apAuthLayout;
    delete m_dataEncLayout;
    delete m_apAuthLabel;
    delete m_dataEncLabel;
    delete m_apAuth;
    delete m_dataEnc;
    delete m_securityKeyGroup;
    delete m_securityKeyLayout;
    delete m_keySelectionLayout;
    delete m_key1Layout;
    delete m_key2Layout;
    delete m_key3Layout;
    delete m_key4Layout;
    delete m_encryptKeyLabel;
    delete m_key1Label;
    delete m_key2Label;
    delete m_key3Label;
    delete m_key4Label;
    delete m_key1Edit;
    delete m_key2Edit;
    delete m_key3Edit;
    delete m_key4Edit;
    delete m_passphraseLayout;
    delete m_passphraseLabel;
    delete m_passphrase;
    delete m_showKey;
}

void WepSettingsWidget::onShowKeyChanged(int state)
{
    if (state == Qt::Unchecked) {
        m_passphrase->setEchoMode(QLineEdit::Password);
    } else {
        m_passphrase->setEchoMode(QLineEdit::Normal);
    }
}

void WepSettingsWidget::onEncKeyTypeChanged(int index)
{
    kDebug() << "Index was changed to: " << index;
    QLayoutItem *layoutItem = m_mainLayout->itemAt(3);
    if (layoutItem == 0) {
        return;
    }

    switch (index) {
        case 0:
        case 1:
            if (m_securityKeyGroup != (QGroupBox*)(layoutItem)) {
                //remove the last item
                m_mainLayout->takeAt(3);
                layoutItem->widget()->setVisible(false);
                m_mainLayout->addWidget(m_securityKeyGroup);
                m_securityKeyGroup->setVisible(true);
            }
            return;
        case 2:
            if (m_passphraseWidget != (QWidget*)(layoutItem)) {
                //remove the last item
                m_mainLayout->takeAt(3);
                layoutItem->widget()->setVisible(false);
                m_mainLayout->addWidget(m_passphraseWidget);
                m_passphraseWidget->setVisible(true);
            }
            return;
        default:
            return;
    }
}

#include "encryptionsettingswidget.moc"
