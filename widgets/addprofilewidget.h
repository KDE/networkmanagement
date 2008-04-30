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

#ifndef ADDPROFILEWIDGET_H
#define ADDPROFILEWIDGET_H

#include "ifaceitemmodel.h"
#include "configifacewidget.h"

#include <QWidget>
#include <QListView>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QLineEdit>
#include <QLabel>
#include <QButtonGroup>
#include <QPushButton>
#include <QGroupBox>

#include <KConfig>
#include <KDialog>
#include <klocalizedstring.h>

class AddProfileWidget : public QWidget
{
    Q_OBJECT

    public:
        AddProfileWidget(QWidget *parent=0);
        ~AddProfileWidget();

    private Q_SLOTS:
        void onItemViewClicked(const QModelIndex &index);
        void onConfigClicked();

    private:
        QVBoxLayout *m_mainLayout;
        QGroupBox *m_priorityBox;
        QListView *m_ifaceView;
        IfaceItemModel *m_ifaceModel;
        QLineEdit *m_profileNameEdit;
        QLabel *m_profileName;
        QPushButton *m_configureIface, *m_addPriority, *m_decPriority;

        KDialog *m_configdlg;
        ConfigIfaceWidget *m_configWidget;
};

#endif
