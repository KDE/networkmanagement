/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

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

#ifndef NM07_MANAGE_TRAY_WIDGET_H
#define NM07_MANAGE_TRAY_WIDGET_H

#include <KCModule>

class QMenu;

class TraySettingsWidget;
class MenuToolTipSettingsWidget;

class ManageTrayWidget : public KCModule
{
Q_OBJECT
public:
    ManageTrayWidget(QWidget * parent = 0, const QVariantList & args = QVariantList());
    virtual ~ManageTrayWidget();
    virtual void save();
private slots:
    /**
     * Used by the tray settings widget to indicate when its unmanaged widgets change
     */
    void otherSettingsChanged();
private:
    TraySettingsWidget * mTraySettingsWidget;
    MenuToolTipSettingsWidget * mMenuToolTipSettingsWidget;
};

#endif // NM07_MANAGE_TRAY_WIDGET_H
