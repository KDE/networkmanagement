/*
Copyright 2012 Lamarque V. Souza <lamarque@kde.org>

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

#include "treewidgetitem.h"

#include <QtCore/QDateTime>

TreeWidgetItem::TreeWidgetItem(QTreeWidget *view, const QStringList &strings, int type): QTreeWidgetItem(view, strings, type)
{
}

bool TreeWidgetItem::operator<(const QTreeWidgetItem &other) const
{
    QTreeWidget * view = treeWidget();
    int column = view ? view->sortColumn() : 0;

    if ((column + Qt::UserRole + 1) != ConnectionLastUsedRole) {
	QString a = data(column, Qt::DisplayRole).toString();
	QString b = other.data(column, Qt::DisplayRole).toString();
	return (QString::localeAwareCompare(a, b) < 0);
    }

    QDateTime a = data(0, ConnectionLastUsedRole).toDateTime();
    QDateTime b = other.data(0, ConnectionLastUsedRole).toDateTime();
    return (a < b);
}
