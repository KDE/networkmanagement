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

#include "addprofilewidget.h"

#include <KIcon>

AddProfileWidget::AddProfileWidget(QWidget *parent)
    : QWidget(parent),
      m_mainLayout(0),
      m_priorityBox(0),
      m_ifaceView(0),
      m_profileNameEdit(0),
      m_profileName(0),
      m_configureIface(0),
      m_configdlg(0),
      m_configWidget(0)
      
{
    m_mainLayout = new QVBoxLayout(this);

    QHBoxLayout *profileNameLayout = new QHBoxLayout();
    m_profileName = new QLabel(i18n("Profile Name"));
    m_profileNameEdit = new QLineEdit();
    profileNameLayout->addWidget(m_profileName);
    profileNameLayout->addWidget(m_profileNameEdit);

    m_priorityBox = new QGroupBox(i18n("Interface Priority"));
    m_ifaceView = new QListView();
    m_ifaceModel = new IfaceItemModel();
    m_ifaceView->setModel(m_ifaceModel);
    QHBoxLayout *priorityHLayout = new QHBoxLayout();
    QVBoxLayout *priorityVLayout = new QVBoxLayout();
    m_addPriority = new QPushButton(KIcon("arrow-up"), QString());
    m_decPriority = new QPushButton(KIcon("arrow-down"), QString());
    priorityVLayout->addWidget(m_addPriority);
    priorityVLayout->addWidget(m_decPriority);
    priorityHLayout->addWidget(m_ifaceView);
    priorityHLayout->addLayout(priorityVLayout);
    m_configureIface = new QPushButton(i18n("Configure Interface"));
    priorityVLayout->addWidget(m_configureIface);
    m_priorityBox->setLayout(priorityHLayout);

    m_mainLayout->addLayout(profileNameLayout);
    m_mainLayout->addWidget(m_priorityBox);

    setLayout(m_mainLayout);

    m_configdlg = new KDialog();
    m_configdlg->setButtons( KDialog::Ok | KDialog::Cancel);
    connect(m_configureIface, SIGNAL(clicked()), this, SLOT(onConfigClicked()));
}

AddProfileWidget::~AddProfileWidget()
{
    delete m_mainLayout;
    delete m_priorityBox;
    delete m_ifaceView;
    delete m_profileNameEdit;
    delete m_profileName;
    delete m_configureIface;
}

void AddProfileWidget::onConfigClicked()
{
    if (m_configWidget == 0) {
        m_configdlg->setCaption("Wifi Configuration");
        m_configWidget = new WifiConfigIfaceWidget(m_configdlg);
        m_configdlg->setMainWidget(m_configWidget);
        m_configdlg->show();
    }
}

#include "addprofilewidget.moc"

