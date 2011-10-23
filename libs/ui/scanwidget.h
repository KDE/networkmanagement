/*
Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>
Copyright 2010 Will Stephenson <wstephenson@kde.org>

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

#ifndef SCANWIDGET_H
#define SCANWIDGET_H

#include <QWidget>
#include "ui_scanwidget.h"

#include <QItemSelectionModel>
#include <QSortFilterProxyModel>

#include <KLocale>
#include <solid/control/wirelessaccesspoint.h>

#include "apitemview.h"
#include "networkitemmodel.h"
#include "apitemdelegate.h"

class QTreeView;

class ScanWidget : public QWidget, public Ui::ScanWidget
{
    Q_OBJECT

    public:
        ScanWidget(QWidget *parent=0);
        ~ScanWidget();

        void setWirelessInterface(const QString &interface);
        QPair<QString,QString> currentAccessPoint() const;
        QPair<Solid::Control::WirelessNetworkInterfaceNm09 *, Solid::Control::AccessPointNm09 *> currentAccessPointUni();

    private Q_SLOTS:
        void onInterfaceChanged(int index);

    private:
        ApItemView * m_scanView;
        NetworkItemModel *m_scanModel;
        QSortFilterProxyModel *m_proxyModel;
        QTreeView * m_detailsView;
        ApItemDelegate *m_scanDelegate;
        QItemSelectionModel *m_scanSelectionModel;
        QItemSelectionModel *m_scanProxySelectionModel;

        enum Views {Map = 0, Details};

    Q_SIGNALS:
        void doubleClicked();
};

#endif
