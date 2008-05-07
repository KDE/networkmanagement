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
      m_deleteProfileButton(0),
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
    m_deleteProfileButton = new QPushButton(i18n("Delete Profile"));
    //disable until a profile is selected
    editProfileButton->setEnabled(false);
    m_deleteProfileButton->setEnabled(false);
    
    buttonLayout = new QVBoxLayout();
    buttonLayout->addWidget(addProfileButton);
    buttonLayout->addWidget(editProfileButton);
    buttonLayout->addWidget(m_deleteProfileButton);

    m_profileView = new QListView();
    m_profileModel = new ProfileItemModel();
    m_profileView->setModel(m_profileModel);

    mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(m_profileView);
    mainLayout->addLayout(buttonLayout);

    connect(m_profileView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(onItemViewClicked(const QModelIndex&)));
    connect(addProfileButton, SIGNAL(clicked()), this, SLOT(onAddProfileClicked()));
    connect(editProfileButton, SIGNAL(clicked()), this, SLOT(onEditProfileClicked()));
    connect(m_deleteProfileButton, SIGNAL(clicked()), this, SLOT(onDeleteProfileClicked()));
}
ManageProfileWidget::~ManageProfileWidget()
{
    disconnect(m_profileView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(onItemViewClicked(const QModelIndex&)));
    disconnect(addProfileButton, SIGNAL(clicked()), this, SLOT(onAddProfileClicked()));
    disconnect(editProfileButton, SIGNAL(clicked()), this, SLOT(onEditProfileClicked()));
    disconnect(m_deleteProfileButton, SIGNAL(clicked()), this, SLOT(onDeleteProfileClicked()));
    
    delete addProfileButton;
    delete editProfileButton;
    delete m_deleteProfileButton;
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
    m_deleteProfileButton->setEnabled(index.isValid());
}

void ManageProfileWidget::onAddProfileClicked()
{
    disconnect(m_addProfiledlg, SIGNAL(user1Clicked()), this, SLOT(onProfileAdded()));
    disconnect(m_generalAddWidget, SIGNAL(validationChanged(bool)), this, SLOT(onPage1Valid(bool)));
    disconnect(m_generalAddWidget, SIGNAL(wirelessInAppropriate(bool)), this, SLOT(onPage2Inappropriate(bool)));
    disconnect(m_wirelessAddWidget, SIGNAL(validationChanged(bool)), this, SLOT(onPage2Valid(bool)));
    
    delete m_wirelessAddWidget;
    delete m_generalAddWidget;
    delete m_addProfiledlg;
    
    m_wirelessAddWidget = new WirelessSettingsWidget();
    m_generalAddWidget =  new GeneralSettingsWidget();
    m_generalAddWidget->setExistingProfiles(m_config.groupList());
    m_generalAddWidget->setWirelessSettings(m_wirelessAddWidget);

    m_addProfiledlg = new KAssistantDialog(this);
    m_addProfiledlg->setCaption("Add Profile");
    m_page1 = m_addProfiledlg->addPage(m_generalAddWidget, i18n("General Settings"));
    m_page2 = m_addProfiledlg->addPage(m_wirelessAddWidget, i18n("Wireless Settings"));
    
    m_addProfiledlg->setCurrentPage(m_page1);
    m_addProfiledlg->setValid(m_page1, false);
    m_addProfiledlg->setValid(m_page2, false);
    connect(m_addProfiledlg, SIGNAL(user1Clicked()), this, SLOT(onProfileAdded()));
    connect(m_generalAddWidget, SIGNAL(validationChanged(bool)), this, SLOT(onPage1Valid(bool)));
    connect(m_generalAddWidget, SIGNAL(wirelessAppropriate(bool)), this, SLOT(onPage2Appropriate(bool)));
    connect(m_wirelessAddWidget, SIGNAL(validationChanged(bool)), this, SLOT(onPage2Valid(bool)));
    m_addProfiledlg->show();
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

    //load configuration into the dialog
    QModelIndex index = m_profileView->currentIndex();
    kDebug() << "Loading profile: " << index.data().toString();
    m_generalEditWidget->loadConfig(KConfigGroup(&m_config, index.data().toString()));

    //disable non-editable rows
    m_generalEditWidget->setStaticMode();
    m_editProfiledlg->setCaption(i18n("Edit Profile"));
    m_editProfiledlg->addPage(m_generalEditWidget, i18n("General Settings"));
    m_editProfiledlg->addPage(m_wirelessEditWidget, i18n("Wireless Settings"));
    m_editProfiledlg->show();
}

void ManageProfileWidget::onDeleteProfileClicked()
{
    QModelIndex current = m_profileView->currentIndex();
    if (!current.isValid()) {
        return;
    }

    QString profileName = current.data().toString();
    if (!m_config.groupList().contains(profileName)) {
        return;
    }
    m_config.deleteGroup(profileName);
    m_config.sync();
    m_profileModel->removeRow(current.row());
}

void ManageProfileWidget::onProfileAdded()
{
    kDebug() << "Profile was added.  Saving . . . ";
    KConfigGroup newGroup(&m_config, m_generalAddWidget->profileName());

    //get general settings
    m_generalAddWidget->saveConfig(newGroup);

    //there's no need to get wireless settings if this is a wired network profile
    if (!m_generalAddWidget->wiredProfile()) {
        //get wireless settings
        m_wirelessAddWidget->saveConfig(newGroup);
    }

    //update profile item view
    int newIndex = m_profileModel->rowCount();
    if (m_profileModel->insertRow(newIndex)) {
        m_profileModel->setData(m_profileModel->index(newIndex,0), QVariant(m_generalAddWidget->profileName()), Qt::DisplayRole);
        m_profileModel->setData(m_profileModel->index(newIndex,0), QVariant(newGroup.readEntry("ProfileType", (int)NetworkProfile::Default)), Qt::DecorationRole);
    }
}

void ManageProfileWidget::onProfileChanged()
{
}

void ManageProfileWidget::onPage1Valid(bool valid)
{
    m_addProfiledlg->setValid(m_page1, valid);
    m_addProfiledlg->enableButton(KDialog::User1, valid && !m_addProfiledlg->isAppropriate(m_page2));
}

void ManageProfileWidget::onPage2Valid(bool valid)
{
    m_addProfiledlg->setValid(m_page2, valid);
}

void ManageProfileWidget::onPage2Appropriate(bool appropriate)
{
    m_addProfiledlg->setAppropriate(m_page2, appropriate);
    m_addProfiledlg->enableButton(KDialog::User1, !appropriate && m_addProfiledlg->isValid(m_page1));
    m_addProfiledlg->enableButton(KDialog::User2, appropriate);
}

#include "manageprofilewidget.moc"
