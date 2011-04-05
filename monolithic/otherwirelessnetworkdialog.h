/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#ifndef OTHERWIRELESSNETWORKWIDGET_H
#define OTHERWIRELESSNETWORKWIDGET_H

#include <KDialog>

#include <activatableobserver.h>

#include "ui_otherwirelessnetworkwidget.h"
#include "interfaceconnection.h"

namespace Knm
{
    class Activatable;
} // namespace Knm

class ActivatableList;
class QTreeWidgetItem;

class OtherWirelessNetworkDialog : public KDialog, public ActivatableObserver
{
Q_OBJECT
public:
    OtherWirelessNetworkDialog(ActivatableList * activatables, QWidget * parent = 0);
    ~OtherWirelessNetworkDialog();
protected:
    void handleAdd(Knm::Activatable*);
    void handleUpdate(Knm::Activatable*);
    void handleRemove(Knm::Activatable*);
    void activateInternal(QTreeWidgetItem *);
protected Q_SLOTS:
    void itemDoubleClicked(QTreeWidgetItem *);
    void itemSelectionChanged();
    void slotButtonClicked(int button);
    void setActivationState(Knm::InterfaceConnection::ActivationState);
private:
    ActivatableList * m_activatables;
    QTreeWidgetItem * m_newAdhocItem;
    int m_networkItemCount;
    Ui_OtherWirelessNetworkWidget m_ui;
};

#endif
