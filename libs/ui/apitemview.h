/*
Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>

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

#ifndef APITEMVIEW_H
#define APITEMVIEW_H

#include "networkitemmodel.h"

#include <QWidget>
#include <QPoint>
#include <QAbstractItemDelegate>
#include <QItemSelectionModel>
#include <QPixmap>
#include <QMouseEvent>

class ApItemView : public QWidget
{
    Q_OBJECT

    public:
        ApItemView(QWidget *parent=0);
        ~ApItemView();

        void setModel(QAbstractItemModel *model);
        QAbstractItemModel * model();
        void setItemDelegate(QAbstractItemDelegate *delegate);
        QAbstractItemDelegate* delegate() const;

        void setSelectionModel(QItemSelectionModel *selectionModel);
        QItemSelectionModel* selectionModel() const;

        QModelIndex currentIndex() const;
        void setCurrentIndex(const QModelIndex &index);

        QModelIndex indexAt(const QPoint &p) const;

    private:
        void initStyleOptions();
        void paintEvent(QPaintEvent *event);
        void layoutItems();
        void layoutGroup(QList<QModelIndex> group);
        void paintBackground();

        //mouse events
        void mouseReleaseEvent(QMouseEvent *event);
        void mouseDoubleClickEvent(QMouseEvent *event);

        QAbstractItemModel *m_model;
        QAbstractItemDelegate *m_delegate;
        QItemSelectionModel *m_selectionModel;
        QPixmap *m_background;
        QMap<QModelIndex, QRect> m_apLayout;
        QStyleOptionViewItem m_options;
        int m_itemHeight, m_itemWidth;

        //set size and spacing
        static const int fontHeight = 8;
        static const int itemCharWidth = 10;
        static const int computerHeight = 44;
        static const int padding = 3;

    Q_SIGNALS:
        void doubleClicked(const QModelIndex&);
};

#endif
