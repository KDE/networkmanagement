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

#ifndef SCANWIDGET_H
#define SCANWIDGET_H

#include <QWidget>
#include "ui_scanwidget.h"

#include <QItemSelectionModel>

#include <KLocale>
#include <solid/control/wirelessaccesspoint.h>

#include "apitemview.h"
#include "apitemmodel.h"
#include "apitemdelegate.h"
#include "ifaceitemmodel.h"

class QTableView;

class ScanWidget : public QWidget, public Ui::ScanWidget
{
    Q_OBJECT

    public:
        ScanWidget(QWidget *parent=0);
        ~ScanWidget();

        void setWirelessInterface(const QString &interface);
        QString currentAccessPoint() const;

    private Q_SLOTS:
        void onInterfaceChanged(int index);

    private:
        ApItemView * m_scanView;
        ApItemModel *m_scanModel;
        QTableView * m_detailsView;
        ApItemDelegate *m_scanDelegate;
        QItemSelectionModel *m_scanSelectionModel;
        IfaceItemModel *m_ifaceModel;

        QStringList m_wirelessInterfaces;
};

#endif
