/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TRAYSETTINGSWIDGET_H
#define TRAYSETTINGSWIDGET_H

#include <QWidget>

#include "ui_traysettings.h"

class QTreeWidgetItem;

class TraySettingsWidget : public QWidget
{
Q_OBJECT
public:
    TraySettingsWidget(QWidget * parent = 0);
    virtual ~TraySettingsWidget();
protected Q_SLOTS:
    void addIconClicked();
    void removeIconClicked();
private:
    void readIconTypes(uint index, QTreeWidgetItem * item);
    Ui_TraySettings m_ui;
    uint m_iconCount;
};

#endif // TRAYSETTINGSWIDGET_H
