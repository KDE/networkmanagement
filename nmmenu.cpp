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

#include "nmmenu.h"

NMMenu::NMMenu(QWidget *parent)
    : KMenu(parent),
      networkConfig(),
      m_addProfile(new QAction(i18n("Manage profiles..."), this)),
      m_wifiNetworks(new QAction(i18n("Connect to wireless networks..."), this)),
      m_sep1(new QAction(this)),
      m_sep2(new QAction(this))
{
    connect(m_addProfile, SIGNAL(triggered()), this, SIGNAL(editProfileRequested()));
    connect(m_wifiNetworks, SIGNAL(triggered()), this, SIGNAL(scanForNetworksRequested()));
    m_sep1->setSeparator(true);
    m_sep2->setSeparator(true);
}

NMMenu::~NMMenu()
{
    foreach (QAction *action, m_menuMap.keys()) {
        disconnect(action, SIGNAL(triggered()), this, SLOT(itemClicked()));
        delete action;
    }
    disconnect(m_addProfile, SIGNAL(triggered()), this, SIGNAL(createProfileRequested()));
    disconnect(m_wifiNetworks, SIGNAL(triggered()), this, SIGNAL(scanForNetworksRequested()));
}

void NMMenu::setConfig(KConfigGroup config)
{
    if (!config.isValid()) {
        return;
    }

    networkConfig = config;

    //build menu template
    this->clear();
    this->addAction(m_addProfile);
    this->addAction(m_sep1);
    this->addAction(m_wifiNetworks);
    this->addAction(m_sep2);

    //load network profiles
    //temporary hack until dynamic menus can be created.
    profileAdded(QString("Home"));
    profileAdded(QString("Work"));
    profileAdded(QString("Cafe"));

    
}

void NMMenu::profileAdded(const QString &profile)
{
    QAction *action = new QAction(profile, this);
    m_menuMap.insert(action, profile);
    connect(action, SIGNAL(triggered()), this, SLOT(itemClicked()));
    this->addAction(action);
}

void NMMenu::profileRemoved(const QString &profile)
{
    QAction *action = m_menuMap.key(profile);
    disconnect(action, SIGNAL(triggered()), this, SLOT(itemClicked()));
    this->removeAction(action);
    delete action;
}

void NMMenu::itemClicked()
{
    emit launchProfileRequested(m_menuMap.value((QAction*)sender()));
}

#include "nmmenu.moc"
