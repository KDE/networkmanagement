/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.net>, based on work by Will Stephenson <wstephenson@kde.org> and Paul Marchouk <pmarchouk@gmail.com>

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

#ifndef IPV6ADVANCEDWIDGET_H
#define IPV6ADVANCEDWIDGET_H

#include <QWidget>
#include <QStandardItem>
#include <QStyledItemDelegate>

#include "networkipv6config.h"

class QStandardItem;
class QItemSelection;

class IpV6AdvancedWidget : public QWidget
{
Q_OBJECT
public:
    IpV6AdvancedWidget(QWidget * parent = 0);
    virtual ~IpV6AdvancedWidget();

    void setAdditionalAddresses(const QList<Solid::Control::IPv6Address> &list);
    QList<Solid::Control::IPv6Address> additionalAddresses();

protected slots:
    void addIPAddress();
    void removeIPAddress();
    /**
     * Update remove IP button depending on if there is a selection
     */
    void selectionChanged(const QItemSelection &);
    void tableViewItemChanged(QStandardItem *);

private:
    class Private;
    Private *d;
};

#endif //IPV6ADVANCEDWIDGET_H
