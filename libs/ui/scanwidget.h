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
