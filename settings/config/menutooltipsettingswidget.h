/*
Copyright 2009 Paul Marchouk <paul.marchouk@gmail.com>

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

#ifndef MENUTOOLTIPSETTINGSWIDGET_H
#define MENUTOOLTIPSETTINGSWIDGET_H

#include <QWidget>
#include <QStringList>
#include <QHash>

#include "ui_menutooltipsettingswidget.h"

class MenuToolTipSettingsWidget : public QWidget
{
Q_OBJECT
public:
    MenuToolTipSettingsWidget(QWidget * parent = 0);
    MenuToolTipSettingsWidget(const QStringList & allKeys,
                              const QStringList & selectedKeys, QWidget * parent = 0);

    virtual ~MenuToolTipSettingsWidget();

    void init();
    QStringList toolTipKeys() const;

    enum Button {
        AllButtons = 0xFF,
        IncludeButton = 0x1,
        ExcludeButton = 0x2,
        UpButton = 0x4,
        DownButton = 0x8
    };
    Q_DECLARE_FLAGS(Buttons, Button)
signals:
    void changed();
private slots:
    void includeButtonClicked();
    void excludeButtonClicked();
    void upButtonClicked();
    void downButtonClicked();

    void selectedOptionsCurrentRowChanged(int row);

private:
    void updateButtonsState(Buttons buttons = AllButtons);

    void addItemToKeysWidget(QListWidget * widget, const QString &tooltip);

    void insertItemToKeysWidget(QListWidget * widget, QListWidgetItem *item, int row);

    void moveSelectedItem(int from, int to);

private:
    MenuToolTipSettingsWidget(const MenuToolTipSettingsWidget &);
    MenuToolTipSettingsWidget& operator=(const MenuToolTipSettingsWidget &);

private:
    Ui_MenuToolTipSettings m_ui;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MenuToolTipSettingsWidget::Buttons)

#endif // MENUTOOLTIPSETTINGSWIDGET_H
