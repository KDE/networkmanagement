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
        EncryptionSettingsWidget(QWidget *parent=0);
        ~EncryptionSettingsWidget();

        QVBoxLayout *m_mainLayout;
        QHBoxLayout *m_encTypeLayout, *m_keyTypeLayout, *m_passwordLayout;
        QComboBox *m_encType, *m_keyType;
        QLineEdit *m_password;
        QLabel *m_encTypeLabel, *m_keyTypeLabel, *m_passwordLabel;
        QCheckBox *m_showKey;

    private Q_SLOTS:
        void onShowKeyChanged(int state);

    private:
        QStringList m_encTypes;
        QStringList m_keyTypes;
};

#endif
