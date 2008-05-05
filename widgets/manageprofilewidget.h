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

#ifndef MANAGEPROFILEWIDGET_H
#define MANAGEPROFILEWIDGET_H

#include "addprofilewidget.h"
#include "profileitemmodel.h"
#include "networkprofile.h"
#include "editprofilewidget.h"
#include "generalsettingswidget.h"
#include "wirelesssettingswidget.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QLineEdit>
#include <QLabel>
#include <QButtonGroup>
#include <QPushButton>
#include <QGroupBox>
#include <QListView>

#include <KConfig>
#include <KConfigDialog>
#include <KAssistantDialog>
#include <KConfigGroup>
#include <KPageWidgetItem>
#include <klocalizedstring.h>

class ManageProfileWidget : public QWidget
{
    Q_OBJECT

    public:
        ManageProfileWidget(QWidget *parent=0);
        ~ManageProfileWidget();

        KConfigGroup config();
        void setConfig(const KConfigGroup &config);

        QHBoxLayout *mainLayout;
        QVBoxLayout *buttonLayout;
        QListView *profileView;
        QPushButton *addProfileButton, *editProfileButton;

    private Q_SLOTS:
        void onAddProfileClicked();
        void onEditProfileClicked();
        void onItemViewClicked(const QModelIndex &index);

    private:
        KConfigGroup m_config;
        ProfileItemModel *m_profileModel;
        KAssistantDialog *m_addProfiledlg;
        KConfigDialog *m_editProfiledlg;
        WirelessSettingsWidget *m_wirelessWidget;
        GeneralSettingsWidget *m_generalWidget;
        KPageWidgetItem *m_page1, *m_page2;
};

#endif

