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

#include "ui_manageprofilewidget.h"

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

class ManageProfileWidget : public QWidget, public Ui::ManageProfileWidget
{
    Q_OBJECT

    public:
        ManageProfileWidget(QWidget *parent=0);
        ~ManageProfileWidget();

        KConfigGroup config();
        void setConfig(KConfigGroup &config);

    private Q_SLOTS:
        void onAddProfileClicked();
        void onEditProfileClicked();
        void onDeleteProfileClicked();
        void onItemViewClicked(const QModelIndex &index);
        void onProfileAdded();
        void onProfileChanged();
        void onPage1Valid(bool valid);
        void onPage2Valid(bool valid);
        void onPage2Appropriate(bool appropriate);

    private:
        KConfigGroup m_config;
        ProfileItemModel *m_profileModel;
        KAssistantDialog *m_addProfiledlg;
        KConfigDialog *m_editProfiledlg;
        WirelessSettingsWidget *m_wirelessAddWidget, *m_wirelessEditWidget;
        GeneralSettingsWidget *m_generalAddWidget, *m_generalEditWidget;
        KPageWidgetItem *m_page1, *m_page2;
};

#endif

