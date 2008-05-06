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

#include "manageprofilewidget.h"

#include <KDebug>
#include <KDialog>
#include <KConfigSkeleton>

ManageProfileWidget::ManageProfileWidget(QWidget *parent)
    : QWidget(parent),
      mainLayout(0),
      buttonLayout(0),
      m_profileView(0),
      addProfileButton(0),
      editProfileButton(0),
      m_config(),
      m_profileModel(0),
      m_addProfiledlg(0),
      m_editProfiledlg(0),
      m_wirelessAddWidget(0),
      m_wirelessEditWidget(0),
      m_generalAddWidget(0),
      m_generalEditWidget(0)
{
    addProfileButton = new QPushButton(i18n("Add Profile"));
    editProfileButton = new QPushButton(i18n("Edit Profile"));
    //disable until a profile is selected
    editProfileButton->setEnabled(false);
    buttonLayout = new QVBoxLayout();
    buttonLayout->addWidget(addProfileButton);
    buttonLayout->addWidget(editProfileButton);

    m_profileView = new QListView();
    m_profileModel = new ProfileItemModel();
    m_profileView->setModel(m_profileModel);

    mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(m_profileView);
    mainLayout->addLayout(buttonLayout);

    connect(m_profileView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(onItemViewClicked(const QModelIndex&)));
    connect(addProfileButton, SIGNAL(clicked()), this, SLOT(onAddProfileClicked()));
    connect(editProfileButton, SIGNAL(clicked()), this, SLOT(onEditProfileClicked()));
}
ManageProfileWidget::~ManageProfileWidget()
{
    disconnect(m_profileView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(onItemViewClicked(const QModelIndex&)));
    disconnect(addProfileButton, SIGNAL(clicked()), this, SLOT(onAddProfileClicked()));
    disconnect(editProfileButton, SIGNAL(clicked()), this, SLOT(onEditProfileClicked()));
    
    delete addProfileButton;
    delete editProfileButton;
    delete m_profileModel;
    delete m_profileView;
    delete buttonLayout;
    delete mainLayout;
}

KConfigGroup ManageProfileWidget::config()
{
    return m_config;
}

void ManageProfileWidget::setConfig(KConfigGroup &config)
{
    m_config = config;
    m_profileModel->updateConfig(config);
}

void ManageProfileWidget::onItemViewClicked(const QModelIndex &index)
{
    editProfileButton->setEnabled(index.isValid());
}

void ManageProfileWidget::onAddProfileClicked()
{
    delete m_wirelessAddWidget;
    delete m_generalAddWidget;
    delete m_addProfiledlg;
    
    m_wirelessAddWidget = new WirelessSettingsWidget();
    m_generalAddWidget =  new GeneralSettingsWidget();
    m_generalAddWidget->setWirelessSettings(m_wirelessAddWidget);

    m_addProfiledlg = new KAssistantDialog(this);
    m_addProfiledlg->setCaption("Add Profile");
    m_page1 = m_addProfiledlg->addPage(m_generalAddWidget, i18n("General Settings"));
    m_page2 = m_addProfiledlg->addPage(m_wirelessAddWidget, i18n("Wireless Settings"));
    
    m_addProfiledlg->setCurrentPage(m_page1);
    m_addProfiledlg->show();

    connect(m_addProfiledlg, SIGNAL(user1Clicked()), this, SLOT(onProfileAdded()));
}

void ManageProfileWidget::onEditProfileClicked()
{
    delete m_wirelessEditWidget;
    delete m_generalEditWidget;
    delete m_editProfiledlg;

    m_editProfiledlg = new KConfigDialog(this, "Edit Profile", new KConfigSkeleton(0));
    m_wirelessEditWidget = new WirelessSettingsWidget();
    m_generalEditWidget = new GeneralSettingsWidget();
    m_generalEditWidget->setWirelessSettings(m_wirelessEditWidget);
    m_editProfiledlg->setCaption(i18n("Edit Profile"));
    m_editProfiledlg->addPage(m_generalEditWidget, i18n("General Settings"));
    m_editProfiledlg->addPage(m_wirelessEditWidget, i18n("Wireless Settings"));
    m_editProfiledlg->show();
}

void ManageProfileWidget::onProfileAdded()
{
    kDebug() << "Profile was added.  Saving . . . ";
    KConfigGroup newGroup(&m_config, m_generalAddWidget->profileName());

    //get general settings
    m_generalAddWidget->saveConfig(newGroup);

    //there's no need to get wireless settings if this is a wired network profile
    if (m_generalAddWidget->wiredProfile()) {
        return;
    }
    
    //get wireless settings
    m_wirelessAddWidget->saveConfig(newGroup);

    //update profile item view
    m_profileModel->updateConfig(m_config);
}

void ManageProfileWidget::onProfileChanged()
{
}

#include "manageprofilewidget.moc"
