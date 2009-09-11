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

#ifndef TRAYSETTINGSWIDGET_H
#define TRAYSETTINGSWIDGET_H

#include <QWidget>

#include <solid/control/networkinterface.h>

#include "ui_traysettings.h"

class QListWidgetItem;

class TraySettingsWidgetPrivate;

class TraySettingsWidget : public QWidget
{
Q_OBJECT
Q_DECLARE_PRIVATE(TraySettingsWidget)
public:
    TraySettingsWidget(QWidget * parent = 0);
    virtual ~TraySettingsWidget();
    QList<uint> iconInterfaceAllocations() const;
signals:
    void changed();
protected Q_SLOTS:
    void addIconClicked();
    void removeIconClicked();
    void itemsMoved();
private:
    void readIconTypes(uint index);
    TraySettingsWidgetPrivate * d_ptr;
};

#endif // TRAYSETTINGSWIDGET_H
