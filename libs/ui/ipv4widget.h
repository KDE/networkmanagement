/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

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

#ifndef IPV4WIDGET_H
#define IPV4WIDGET_H

#include "settingwidget.h"

#include <QItemDelegate>
#include <QItemSelection>

#include "knm_export.h"

namespace Knm
{
    class Connection;
} // namespace Knm

class Ipv4Delegate : public QItemDelegate
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

class NetmaskPrefixDelegate : public QItemDelegate
{
Q_OBJECT
public:
    NetmaskPrefixDelegate(QObject * parent = 0);
    virtual ~NetmaskPrefixDelegate();

    QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option,
            const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;

    void setModelData(QWidget *editor, QAbstractItemModel *model,
            const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
            const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class KNM_EXPORT IpV4Widget : public SettingWidget
{
Q_OBJECT
public:
    IpV4Widget(Knm::Connection * connection, QWidget * parent = 0 );
    virtual ~IpV4Widget();
    void readConfig();
    void writeConfig();
protected Q_SLOTS:
    /**
     * Update state of widgets dependent on IP setting method
     */
    void methodChanged(int);
    void addIpClicked();
    void removeIpClicked();
    /**
     * Update remove IP button depending on if there is a selection
     */
    void selectionChanged(const QItemSelection &);
private:
    class Private;
    Private * d;
};

#endif
