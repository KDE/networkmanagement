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

#include "apitemview.h"
#include "apitemmodel.h"
#include "apitemdelegate.h"
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
#include <klocalizedstring.h>

class WirelessSettingsWidget : public QWidget
{
    Q_OBJECT

    public:
        WirelessSettingsWidget(QWidget *parent=0);
        ~WirelessSettingsWidget();

        QString wirelessInterface() const;
        void setWirelessInterface(const QString &uni);

        void enableAdhoc(bool enable);

        void saveConfig(KConfigGroup &config);

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

        QStringList m_connectionTypes, m_wirelessModes, m_securityTypes;

        QGridLayout *m_mainLayout;
        QLabel *m_wirelessConfigLabel, *m_essidLabel, *m_connectionTypeLabel, *m_wirelessModeLabel, *m_securityTypeLabel;
        QLineEdit *m_essid;
        QComboBox *m_connectionType, *m_wirelessMode, *m_securityType;
        QPushButton *m_scanButton, *m_securitySettingsButton;

        KDialog *m_scandlg, *m_encryptdlg;
        QString m_wirelessInterface;

        //scanview
        ApItemView *m_scanView;
        ApItemModel *m_scanModel;
        ApItemDelegate *m_scanDelegate;
        QItemSelectionModel *m_scanSelectionModel;

        //encryption
        EncryptionSettingsWidget *m_encryptionWidget;
        EncryptionSettingsWidget::EncryptionType m_savedSecurityType;

        //validation
        bool isInputValid;
};

#endif
