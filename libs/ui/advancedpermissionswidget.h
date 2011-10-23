/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.net>

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

#ifndef ADVANCEDPERMISSIONSWIDGET_H
#define ADVANCEDPERMISSIONSWIDGET_H

#include <QWidget>
#include <QHash>

class QTreeWidgetItem;
class AdvancedPermissionsWidgetPrivate;
class KUser;

class AdvancedPermissionsWidget : public QWidget
{
Q_OBJECT
Q_DECLARE_PRIVATE(AdvancedPermissionsWidget)
public:
    AdvancedPermissionsWidget(QWidget *parent=0);
    AdvancedPermissionsWidget(const QHash<QString,QString>&, QWidget *parent=0);
    ~AdvancedPermissionsWidget();

    QHash<QString,QString> currentUsers();

protected:
    AdvancedPermissionsWidgetPrivate *d_ptr;

private:
    enum Columns {FullName = 0, LoginName = 1};
    void setupCommon();
    QTreeWidgetItem * constructItem(const KUser &user, const QString &itemData = QString());

private Q_SLOTS:
    void leftArrowClicked();
    void rightArrowClicked();
};

#endif // ADVANCEDPERMISSIONSWIDGET_H