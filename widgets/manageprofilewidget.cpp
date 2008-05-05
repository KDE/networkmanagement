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
      profileView(0),
      addProfileButton(0),
      editProfileButton(0),
      m_config(),
      m_profileModel(0),
      m_addProfiledlg(0),
      m_editProfiledlg(0),
      m_wirelessWidget(0),
      m_generalWidget(0)
{
    addProfileButton = new QPushButton(i18n("Add Profile"));
    editProfileButton = new QPushButton(i18n("Edit Profile"));
    //disable until a profile is selected
    editProfileButton->setEnabled(false);
    buttonLayout = new QVBoxLayout();
    buttonLayout->addWidget(addProfileButton);
    buttonLayout->addWidget(editProfileButton);

    profileView = new QListView();
    m_profileModel = new ProfileItemModel();
    profileView->setModel(m_profileModel);

    mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(profileView);
    mainLayout->addLayout(buttonLayout);

    m_wirelessWidget = new WirelessSettingsWidget();
    m_generalWidget =  new GeneralSettingsWidget();
    m_generalWidget->setWirelessSettings(m_wirelessWidget);

    m_addProfiledlg = new KAssistantDialog(this);
    m_addProfiledlg->setCaption("Add Profile");
    m_page1 = m_addProfiledlg->addPage(m_generalWidget, i18n("General Settings"));
    m_page2 = m_addProfiledlg->addPage(m_wirelessWidget, i18n("Wireless Settings"));
    
    m_editProfiledlg = new KConfigDialog(this, "Add Profile", new KConfigSkeleton(0));
    m_editProfiledlg->setCaption("Edit Profile");
    m_editProfiledlg->addPage(m_generalWidget, i18n("General Settings"));
    m_editProfiledlg->addPage(m_wirelessWidget, i18n("Wireless Settings"));
    //m_editProfiledlg->resize(390,420);
    
    connect(profileView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(onItemViewClicked(const QModelIndex&)));
    connect(addProfileButton, SIGNAL(clicked()), this, SLOT(onAddProfileClicked()));
    connect(editProfileButton, SIGNAL(clicked()), this, SLOT(onEditProfileClicked()));
}
ManageProfileWidget::~ManageProfileWidget()
{
    disconnect(profileView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(onItemViewClicked(const QModelIndex&)));
    disconnect(addProfileButton, SIGNAL(clicked()), this, SLOT(onAddProfileClicked()));
    disconnect(editProfileButton, SIGNAL(clicked()), this, SLOT(onEditProfileClicked()));
    
    delete addProfileButton;
    delete editProfileButton;
    delete m_profileModel;
    delete profileView;
    delete buttonLayout;
    delete mainLayout;
}

KConfigGroup ManageProfileWidget::config()
{
    return m_config;
}

void ManageProfileWidget::setConfig(const KConfigGroup &config)
{
    m_config = config;
}

void ManageProfileWidget::onItemViewClicked(const QModelIndex &index)
{
    editProfileButton->setEnabled(index.isValid());
}

void ManageProfileWidget::onAddProfileClicked()
{
    m_addProfiledlg->setCurrentPage(m_page1);
    m_addProfiledlg->show();
}

void ManageProfileWidget::onEditProfileClicked()
{
    m_editProfiledlg->show();
}

#include "manageprofilewidget.moc"
