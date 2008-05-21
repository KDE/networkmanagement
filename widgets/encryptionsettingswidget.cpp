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
      m_hexLetters("abcdef")
{
}

EncryptionSettingsWidget::~EncryptionSettingsWidget()
{
}

bool EncryptionSettingsWidget::isStringHex(const QString &str) const
{
    for (int index=0; index < str.size(); index++) {
        QChar charachter = str.at(index);
        if (!charachter.isDigit() && !m_hexLetters.contains(charachter, Qt::CaseInsensitive)) {
            kDebug() << "Key contains non-hex charachter: " << charachter;
            return false;
        }
    }
    return true;
}

bool EncryptionSettingsWidget::isStringAscii(const QString &str) const
{
    for (int index=0; index < str.size(); index++) {
        QChar charachter = str.at(index);
        if (!charachter.isLetterOrNumber()) {
            kDebug() << "Key contains non-alphanumeric charachter: " << charachter;
            return false;
        }
    }
    return true;
}

WepSettingsWidget::WepSettingsWidget(QWidget *parent)
    : EncryptionSettingsWidget(parent),
      m_mainLayout(0)
{
    m_authTypes << i18n("Open") << i18n("Shared");
    m_encTypes << i18n("WEP-64") << i18n("WEP-128"); // << i18n("CKIP-128") << i18n("CKIP-128");TODO
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

    //initialize validation variables
    m_keyType = Ascii;
    m_keyLength = 5; //wep-64
    m_keyUsed=1;

    connect(m_showKey, SIGNAL(stateChanged(int)), this, SLOT(onShowKeyChanged(int)));
    connect(m_encKeyType, SIGNAL(activated(int)), this, SLOT(onKeyTypeChanged(int)));
    connect(m_encryptKey, SIGNAL(activated(int)), this, SLOT(onKeyChanged(int)));
    connect(m_dataEnc, SIGNAL(activated(int)), this, SLOT(onWepTypeChanged(int)));
    connect(m_key1Edit, SIGNAL(textChanged(const QString&)), this, SLOT(onDataEntered(const QString&)));
    connect(m_key2Edit, SIGNAL(textChanged(const QString&)), this, SLOT(onDataEntered(const QString&)));
    connect(m_key3Edit, SIGNAL(textChanged(const QString&)), this, SLOT(onDataEntered(const QString&)));
    connect(m_key4Edit, SIGNAL(textChanged(const QString&)), this, SLOT(onDataEntered(const QString&)));
    connect(m_passphrase, SIGNAL(textChanged(const QString&)), this, SLOT(onDataEntered(const QString&)));
}

WepSettingsWidget::~WepSettingsWidget()
{
    delete m_apAuthLabel;
    delete m_dataEncLabel;
    delete m_apAuth;
    delete m_dataEnc;
    delete m_encryptKeyLabel;
    delete m_key1Label;
    delete m_key2Label;
    delete m_key3Label;
    delete m_key4Label;
    delete m_key1Edit;
    delete m_key2Edit;
    delete m_key3Edit;
    delete m_key4Edit;
    delete m_passphraseLabel;
    delete m_passphrase;
    delete m_showKey;
    delete m_passphraseLayout;
    delete m_key1Layout;
    delete m_key2Layout;
    delete m_key3Layout;
    delete m_key4Layout;
    delete m_keySelectionLayout;
    delete m_securityKeyLayout;
    delete m_securityKeyGroup;
    delete m_apAuthLayout;
    delete m_dataEncLayout;
    delete m_mainLayout;
}

void WepSettingsWidget::saveConfig(KConfigGroup &config)
{
    kDebug() << "Saving encryption settings.";
    config.writeEntry("WEPAuthentication", m_apAuth->currentIndex());
    config.writeEntry("WEPType", m_dataEnc->currentIndex());
    config.writeEntry("WEPEncryptionKeyType", m_encKeyType->currentIndex());
    config.writeEntry("WEPKey", m_encryptKey->currentIndex());
    config.writeEntry("WEPStaticKey1", m_key1Edit->text());
    config.writeEntry("WEPStaticKey2", m_key2Edit->text());
    config.writeEntry("WEPStaticKey3", m_key3Edit->text());
    config.writeEntry("WEPStaticKey4", m_key4Edit->text());
    config.writeEntry("WEPPassphrase", m_passphrase->text());
}

void WepSettingsWidget::loadConfig(const KConfigGroup &config)
{
    m_apAuth->setCurrentIndex(config.readEntry("WEPAuthentication", 0));
    m_dataEnc->setCurrentIndex(config.readEntry("WEPType", 0));
    m_encKeyType->setCurrentIndex(config.readEntry("WEPEncryptionKeyType", 0));
    onKeyTypeChanged(m_encKeyType->currentIndex());
    m_encryptKey->setCurrentIndex(config.readEntry("WEPKey", 0));
    m_key1Edit->setText(config.readEntry("WEPStaticKey1", QString()));
    m_key2Edit->setText(config.readEntry("WEPStaticKey2", QString()));
    m_key3Edit->setText(config.readEntry("WEPStaticKey3", QString()));
    m_key4Edit->setText(config.readEntry("WEPStaticKey4", QString()));
    m_passphrase->setText(config.readEntry("WEPPassphrase", QString()));
}

EncryptionSettingsWidget::EncryptionType WepSettingsWidget::type() const
{
    return Wep;
}


bool WepSettingsWidget::validate(const QString &input) const
{
    kDebug() << "Validating input: " << input;
    //get the easy less expensive disqualifications out of the way
    if (m_keyType == Passphrase && !input.isEmpty()){
        kDebug() << "Passphrase is valid.";
        return true;
    }
    if (input.isEmpty() || input.size() > m_keyLength) {
        kDebug() << "Key is empty or too long.";
        return false;
    }

    switch (m_keyType) {
        case Ascii:
        case Passphrase:
            if (!isStringAscii(input)) {
                kDebug() << "Key contains non-alphanumeric charachters.";
                return false;
            }
            break;
        case Hex:
            if (!isStringHex(input)) {
                kDebug() << "Key contains non-hex charachters.";
                return false;
            }
            break;
    }
    if (input.size() == m_keyLength) {
        kDebug() << "Key is valid.";
        return true;
    }
    kDebug() << "Key is not yet valid.";
    return false;
}

bool WepSettingsWidget::isValid() const
{
    if(m_keyType == Passphrase && validate(m_passphrase->text())) {
        return true;
    }
    switch (m_keyUsed) {
        case 1:
             return validate(m_key1Edit->text());
        case 2:
            return validate(m_key2Edit->text());
        case 3:
            return validate(m_key3Edit->text());
        case 4:
            return validate(m_key4Edit->text());
        default:
            return false;
    }
}

void WepSettingsWidget::onShowKeyChanged(int state)
{
    if (state == Qt::Unchecked) {
        m_passphrase->setEchoMode(QLineEdit::Password);
    } else {
        m_passphrase->setEchoMode(QLineEdit::Normal);
    }
}

void WepSettingsWidget::onKeyTypeChanged(int index)
{
    m_keyType = (KeyType)index;
    kDebug() << "Index was changed to: " << index;
    QLayoutItem *layoutItem = m_mainLayout->itemAt(3);
    if (layoutItem == 0) {
        return;
    }

    //change the widget
    switch (index) {
        case Ascii:
        case Hex:
            if (m_securityKeyGroup != (QGroupBox*)(layoutItem)) {
                //remove the last item
                m_mainLayout->takeAt(3);
                layoutItem->widget()->setVisible(false);
                m_mainLayout->addWidget(m_securityKeyGroup);
                m_securityKeyGroup->setVisible(true);
            }
            break;
        case Passphrase:
            if (m_passphraseWidget != (QWidget*)(layoutItem)) {
                //remove the last item
                m_mainLayout->takeAt(3);
                layoutItem->widget()->setVisible(false);
                m_mainLayout->addWidget(m_passphraseWidget);
                m_passphraseWidget->setVisible(true);
            }
            break;
        default:
            break;
    }

    //validation
    kDebug() << "Key type change.  Validating . . . ";
    bool valid = isValid();
    kDebug() << "Input is Valid: " << valid;
    emit validationChanged(valid);
}

void WepSettingsWidget::onWepTypeChanged(int type)
{
    kDebug() << "Changing wep type.";
    
    switch (type) {
        case Wep64:
            m_keyLength = 5;
            break;
        case Wep128:
            m_keyLength = 10;
            break;
        default:
            kDebug() << "Wep type not recognized.";
            emit validationChanged(false);
    }

    bool valid =  isValid();
    kDebug() << "Input is Valid: " << valid;
    emit validationChanged(valid);
}

void WepSettingsWidget::onKeyChanged(int key)
{
    kDebug() << "Key changed to: " << key+1 << ".  Validating . . . ";
    m_keyUsed = key+1; //HACK: the index returned from the QComboBox is 0 indexed
    
    bool valid =  isValid();
    kDebug() << "Input is Valid: " << valid;
    emit validationChanged(valid);
}

void WepSettingsWidget::onDataEntered(const QString &text)
{
    Q_UNUSED(text)
            
    if (m_keyType == Passphrase && (QLineEdit*)sender() == m_passphrase) {
        kDebug() << "Passphrase was chosen.";
        emit validationChanged(isValid());
    } else if(m_keyUsed == 1 && (QLineEdit*)sender() == m_key1Edit) {
        kDebug() << "Key1 was chosen.";
        emit validationChanged(isValid());
    } else if(m_keyUsed == 2 && (QLineEdit*)sender() == m_key2Edit) {
        kDebug() << "Key2 was chosen.";
        emit validationChanged(isValid());
    } else if(m_keyUsed == 3 && (QLineEdit*)sender() == m_key3Edit) {
        kDebug() << "Key3 was chosen.";
        emit validationChanged(isValid());
    } else if(m_keyUsed == 4 && (QLineEdit*)sender() == m_key4Edit) {
        kDebug() << "Key4 was chosen.";
        emit validationChanged(isValid());
    }
}

WpaSettingsWidget::WpaSettingsWidget(QWidget *parent)
    : EncryptionSettingsWidget(parent),
      m_mainLayout(0)
{
    m_authTypes << "WPA1" << "WPA2";
    m_encTypes << "TKIP" << "AES";
    
    m_mainLayout = new QGridLayout(this);

    m_apAuthLabel = new QLabel(i18n("Access Point Authentication"));
    m_apAuth = new QComboBox();
    m_apAuth->addItems(m_authTypes);
    m_apAuthLabel->setBuddy(m_apAuth);

    m_encTypeLabel = new QLabel(i18n("Data Encryption"));
    m_encType = new QComboBox();
    m_encType->addItems(m_encTypes);
    m_encTypeLabel->setBuddy(m_encType);
    
    m_sharedKeyLabel = new QLabel(i18n("Pre-Shared Key"));
    m_sharedKey = new QLineEdit();
    m_sharedKeyLabel->setBuddy(m_sharedKey);

    m_mainLayout->addWidget(m_apAuthLabel, 0, 0, 1, 2);
    m_mainLayout->addWidget(m_apAuth, 0, 2);
    m_mainLayout->addWidget(m_encTypeLabel, 1, 0, 1, 2);
    m_mainLayout->addWidget(m_encType, 1, 2);
    m_mainLayout->addWidget(m_sharedKeyLabel, 2, 0, 1, 2);
    m_mainLayout->addWidget(m_sharedKey, 2, 2);
}

WpaSettingsWidget::~WpaSettingsWidget()
{
}

void WpaSettingsWidget::saveConfig(KConfigGroup &config)
{
    Q_UNUSED(config)
            
    kDebug() << "Saving encryption settings.";
}

void WpaSettingsWidget::loadConfig(const KConfigGroup &config)
{
    Q_UNUSED(config)
            
    return;
}

EncryptionSettingsWidget::EncryptionType WpaSettingsWidget::type() const
{
    return Wpa;
}


//bool WpaSettingsWidget::validate(const QString &input) const
bool WpaSettingsWidget::isValid() const
{
    //kDebug() << "Validating input: " << input;
    return true;
}

#include "encryptionsettingswidget.moc"
