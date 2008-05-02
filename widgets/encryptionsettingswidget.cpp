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
      m_mainLayout(0),
      m_encTypeLayout(0),
      m_keyTypeLayout(0),
      m_passwordLayout(0),
      m_encType(0),
      m_keyType(0),
      m_password(0),
      m_encTypeLabel(0),
      m_keyTypeLabel(0),
      m_passwordLabel(0),
      m_showKey(0)
{
    m_encTypes << "None" << "WEP" << "WPA" << "WPA2";
    m_keyTypes << "Ascii" << "Hex" << "Passphrase";
    
    m_mainLayout = new QVBoxLayout(this);

    m_encTypeLayout = new QHBoxLayout();
    m_encTypeLabel = new QLabel(i18n("Encryption Type"));
    m_encType = new QComboBox();
    m_encType->insertItems(0,m_encTypes);
    m_encTypeLayout->addWidget(m_encTypeLabel);
    m_encTypeLayout->addWidget(m_encType);

    m_keyTypeLayout = new QHBoxLayout();
    m_keyTypeLabel = new QLabel(i18n("Key Type"));
    m_keyType = new QComboBox();
    m_keyType->insertItems(0,m_keyTypes);
    m_keyTypeLayout->addWidget(m_keyTypeLabel);
    m_keyTypeLayout->addWidget(m_keyType);

    m_passwordLayout = new QHBoxLayout();
    m_passwordLabel = new QLabel(i18n("Key"));
    m_password = new QLineEdit();
    m_password->setEchoMode(QLineEdit::Password);
    m_passwordLayout->addWidget(m_passwordLabel);
    m_passwordLayout->addWidget(m_password);

    m_showKey = new QCheckBox(i18n("Show Password"));
    m_showKey->setChecked(false);
    m_showKey->setTristate(false);

    m_mainLayout->addLayout(m_encTypeLayout);
    m_mainLayout->addLayout(m_keyTypeLayout);
    m_mainLayout->addLayout(m_passwordLayout);
    m_mainLayout->addWidget(m_showKey);

    connect(m_showKey, SIGNAL(stateChanged(int)), this, SLOT(onShowKeyChanged(int)));
}

EncryptionSettingsWidget::~EncryptionSettingsWidget()
{
    delete m_encType;
    delete m_keyType;
    delete m_password;
    delete m_encTypeLabel;
    delete m_keyTypeLabel;
    delete m_passwordLabel;
    delete m_showKey;
    delete m_mainLayout;
    delete m_encTypeLayout;
    delete m_keyTypeLayout;
    delete m_passwordLayout;
}

void EncryptionSettingsWidget::onShowKeyChanged(int state)
{
    if (state == Qt::Unchecked) {
        m_password->setEchoMode(QLineEdit::Password);
    } else {
        m_password->setEchoMode(QLineEdit::Normal);
    }
}

#include "encryptionsettingswidget.moc"
