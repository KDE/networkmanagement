/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2009 Paul Marchouk <pmarchouk@gmail.com>

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

#ifndef IPV4ADVANCEDWIDGET_H
#define IPV4ADVANCEDWIDGET_H

#include <QWidget>
#include <QStyledItemDelegate>

#include <solid/control/networkipv4config.h>

class Ipv4Delegate : public QStyledItemDelegate
{
Q_OBJECT
public:
    Ipv4Delegate(QObject * parent = 0);
    virtual ~Ipv4Delegate();

    QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option,
            const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;

    void setModelData(QWidget *editor, QAbstractItemModel *model,
            const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
            const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class QStandardItem;

class IpV4AdvancedWidget : public QWidget
{
Q_OBJECT
public:
    IpV4AdvancedWidget(QWidget * parent = 0);
    virtual ~IpV4AdvancedWidget();

    void setAdditionalAddresses(const QList<Solid::Control::IPv4Address> &list);
    QList<Solid::Control::IPv4Address> additionalAddresses();

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

#endif //IPV4ADVANCEDWIDGET_H
