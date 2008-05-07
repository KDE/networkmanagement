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

#ifndef NMMENU_H
#define NMMENU_H

#include <QHash>
#include <QAction>

#include <KMenu>
#include <KConfigGroup>

class NMMenu : public KMenu
{
    Q_OBJECT

    public:
        NMMenu(QWidget *parent=0);
        ~NMMenu();

        void setConfig(KConfigGroup config);
        
    public Q_SLOTS:
        void profileAdded(const QString &profile);
        void profileRemoved(const QString &profile);
        void itemClicked();
        void reloadProfiles();

    Q_SIGNALS:
        void manageProfilesRequested();
        void scanForNetworksRequested();
        void launchProfileRequested(const QString &profile);

    private:
        void deleteAllProfiles();
        KConfigGroup networkConfig;
        QHash<QAction*,QString> m_menuMap;
        QAction *m_addProfile, *m_wifiNetworks, *m_sep1, *m_sep2;
};

#endif //NMMENU_H
