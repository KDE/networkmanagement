/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

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

#ifndef NM07_CONNECTION_EDITOR_H
#define NM07_CONNECTION_EDITOR_H

#include <KCModule>

#include "connectioneditor.h"

#include "ui_connectioneditor.h"

class ConnectionPreferences;

class ConnectionEditor : public KCModule
{
Q_OBJECT
public:
    ConnectionEditor(QWidget * parent = 0, const QVariantList & args = QVariantList());
    virtual ~ConnectionEditor();
    virtual void save();
    virtual void load();
private slots:
    void addClicked();
    void editClicked();
    void deleteClicked();
private:
    ConnectionPreferences * editorForCurrentIndex(QWidget * parent, const QVariantList & args) const;
    QTreeWidgetItem * selectedItem() const;
    void restoreConnections();
    Ui_ConnectionEditor mConnEditUi;
    QTreeWidget * mWiredList;
};

#endif
