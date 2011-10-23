/*
Copyright 2009 Andrey Batyiev <batyiev@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef CONNECTIONINFODIALOG_H
#define CONNECTIONINFODIALOG_H

#include <QtGui/QWidget>
#include <KDialog>

#include <interfaceconnection.h>

#include <solid/control/networkinterface.h>
#include <solid/control/networkipv4confignm09.h>

#include "ui_connectioninfowidget.h"

/**
 * Dialog used to show connection properties. It automatically handles type of InterfaceConnection
 * creating needed tabs if connection is active.
 * It closes automatically if underlying InterfaceConnection destroyed (e.g. device removed).
 */
class ConnectionInfoDialog : public KDialog {
    Q_OBJECT

public:
    ConnectionInfoDialog(Knm::InterfaceConnection *ic, QWidget *parent = 0);
    ~ConnectionInfoDialog();

public slots:
    void interfaceConnectionStateChanged(Knm::InterfaceConnection::ActivationState);
    void updateConnectionState(int new_state, int old_state, int reason);

private:
    void buildGUI();
    void clearGUI();
    Ui_ConnectionInfoWidget m_ui;
    Knm::InterfaceConnection *m_ic;
    Solid::Control::NetworkInterface *m_iface;
    bool m_guiMade;
};

#endif // CONNECTIONINFOWIDGET_H
