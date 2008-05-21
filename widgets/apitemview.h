/*  This file is part of the KDE project
    Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef APITEMVIEW_H
#define APITEMVIEW_H

#include "apitemmodel.h"

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

        void setModel(ApItemModel *model);
        ApItemModel* model();
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

        ApItemModel *m_model;
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
};

#endif
