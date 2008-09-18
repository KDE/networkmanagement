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

#ifndef WIRELESSSETTINGSWIDGET_H
#define WIRELESSSETTINGSWIDGET_H

#include "ui_wirelesssettingswidget.h"

#include "scanwidget.h"
#include "encryptionsettingswidget.h"

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
#include <KLocale>

class WirelessSettingsWidget : public QWidget, public Ui::WirelessSettingsWidget
{
    Q_OBJECT

    public:
        WirelessSettingsWidget(QWidget *parent=0);
        ~WirelessSettingsWidget();

        QString wirelessInterface() const;
        void setWirelessInterface(const QString &uni);

        void enableAdhoc(bool enable);

        void saveConfig(KConfigGroup &config);
        void loadConfig(const KConfigGroup &config);

        //validation
        bool isValid() const;

    Q_SIGNALS:
        void validationChanged(bool);

    private Q_SLOTS:
        void onScanClicked();
        void onApChosen();
        void onSecurityTypeChanged(int index);
        void onEncryptClicked();
        void onEncryptionSet();
        void onEssidChanged(const QString &text);

    private:
        void enableSpecificItems(bool enable=true);
        void createEncryptionWidget();

        QStringList m_connectionTypes, m_wirelessModes, m_securityTypes;

        KDialog *m_scandlg, *m_encryptdlg;
        QString m_wirelessInterface;

        //scanview
        ScanWidget *m_scanWidget;

        //encryption
        EncryptionSettingsWidget *m_encryptionWidget;
        EncryptionSettingsWidget::EncryptionType m_savedSecurityType;

        //validation
        bool isInputValid;
};

#endif
