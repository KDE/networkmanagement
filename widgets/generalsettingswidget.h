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

        QGridLayout *m_mainLayout;
        QLabel *m_profileNameLabel, *m_connectionTypeLabel;
        QComboBox *m_profileType, *m_connectionType;
        QLineEdit *m_profileName;
        QListView *m_priorityList;
        QPushButton *m_upButton, *m_downButton;

    private Q_SLOTS:
        void onConnectionTypeChanged(int index);

    private:
        QStringList m_connectionTypes;
        QStringList m_connectionTypeIcons;

        IfaceItemModel *m_ifaceModel;
};

#endif
