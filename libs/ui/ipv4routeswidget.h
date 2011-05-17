/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.net>

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

#ifndef IPV4ROUTESWIDGET_H
#define IPV4ROUTESWIDGET_H

#include <QWidget>
#include <QStyledItemDelegate>

#include <solid/control/networkipv4confignm09.h>

class QStandardItem;
class QItemSelection;

class IpV4RoutesWidget : public QWidget
{
Q_OBJECT
public:
    IpV4RoutesWidget(QWidget * parent = 0);
    virtual ~IpV4RoutesWidget();

    void setRoutes(const QList<Solid::Control::IPv4RouteNm09> &list);
    QList<Solid::Control::IPv4RouteNm09> routes();
    void setNeverDefault(bool checked);
    bool neverdefault();
    void setIgnoreAutoRoutes(bool checked);
    bool ignoreautoroutes();

protected slots:
    void addRoute();
    void removeRoute();
    /**
     * Update remove IP button depending on if there is a selection
     */
    void selectionChanged(const QItemSelection &);
    void tableViewItemChanged(QStandardItem *);

private:
    class Private;
    Private *d;
};

#endif //IPV4ADVANCEDWIDGET_H
