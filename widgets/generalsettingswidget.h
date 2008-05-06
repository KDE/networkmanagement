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

#ifndef GENERALSETTINGSWIDGET_H
#define GENERALSETTINGSWIDGET_H

#include "ifaceitemmodel.h"
#include "wirelesssettingswidget.h"

#include <QListView>
#include <QGridLayout>
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

class GeneralSettingsWidget : public QWidget
{
    Q_OBJECT

    public:
        GeneralSettingsWidget(QWidget *parent=0);
        ~GeneralSettingsWidget();
        
        void setWirelessSettings(WirelessSettingsWidget *wifiSettings);

        QString profileName() const;
        bool wiredProfile() const;

        void saveConfig(KConfigGroup &config);

    private Q_SLOTS:
        void onConnectionTypeChanged(int index);
        void onPriorityListActivated(const QModelIndex &index);
        void onCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
        void onUpButtonClicked();
        void onDownButtonClicked();

    private:
        QStringList m_connectionTypes;
        QStringList m_connectionTypeIcons;

        QGridLayout *m_mainLayout;
        QLabel *m_profileNameLabel, *m_connectionTypeLabel;
        QComboBox *m_profileType, *m_connectionType;
        QLineEdit *m_profileName;
        QListView *m_priorityList;
        QPushButton *m_upButton, *m_downButton;

        IfaceItemModel *m_ifaceModel;
        WirelessSettingsWidget *m_wifiSettings;
};

#endif
