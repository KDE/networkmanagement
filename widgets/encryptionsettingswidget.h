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

#ifndef ENCRYPTIONSETTINGSWIDGET_H
#define ENCRYPTIONSETTINGSWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QLineEdit>
#include <QLabel>
#include <QButtonGroup>
#include <QPushButton>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>

#include <KConfig>
#include <KDialog>
#include <klocalizedstring.h>

class EncryptionSettingsWidget : public QWidget
{
    Q_OBJECT

    public:
        enum EncryptionType {None=0, Wep, Wpa};
        enum KeyType {Ascii=0, Hex, Passphrase};
        
        EncryptionSettingsWidget(QWidget *parent=0);
        ~EncryptionSettingsWidget();

        virtual void saveConfig(KConfigGroup &config) = 0;
        virtual EncryptionType type() const = 0;
        
    Q_SIGNALS:
        void validationChanged(bool);
};

class WepSettingsWidget : public EncryptionSettingsWidget
{
    Q_OBJECT

    public:
        enum WepType {Wep64, Wep128}; //, Ckip64, Ckip128};TODO
        WepSettingsWidget(QWidget *parent=0);
        ~WepSettingsWidget();

        void saveConfig(KConfigGroup &config);
        EncryptionType type() const;

        //validation
        bool isValid() const;

    private Q_SLOTS:
        void onShowKeyChanged(int state);
        //validation
        void onKeyTypeChanged(int index);
        void onWepTypeChanged(int type);
        void onKeyChanged(int key);
        void onDataEntered(const QString &text);

    private:
        QStringList m_authTypes;
        QStringList m_encTypes;
        QStringList m_keyTypes;

        QVBoxLayout *m_mainLayout;
        QHBoxLayout *m_apAuthLayout, *m_dataEncLayout, *m_encKeyTypeLayout;
        QLabel *m_apAuthLabel, *m_dataEncLabel, *m_encKeyTypeLabel;
        QComboBox *m_apAuth, *m_dataEnc, *m_encKeyType;

        //static wep keys
        QGroupBox *m_securityKeyGroup;
        QVBoxLayout *m_securityKeyLayout;
        QHBoxLayout *m_keySelectionLayout, *m_key1Layout, *m_key2Layout, *m_key3Layout, *m_key4Layout;
        QLabel *m_encryptKeyLabel, *m_key1Label, *m_key2Label, *m_key3Label, *m_key4Label;
        QLineEdit *m_key1Edit, *m_key2Edit, *m_key3Edit, *m_key4Edit;
        QComboBox *m_encryptKey;

        //passphrase
        QWidget *m_passphraseWidget;
        QGridLayout *m_passphraseLayout;
        QLabel *m_passphraseLabel;
        QLineEdit *m_passphrase;
        QCheckBox *m_showKey;

        //validation
        bool isStringHex(const QString &str) const;
        bool isStringAscii(const QString &str) const;
        bool validate(const QString &input) const;
        EncryptionSettingsWidget::KeyType m_keyType;
        int m_keyLength, m_keyUsed;
        QString m_hexLetters;
};

#endif
