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

#include "editprofilewidget.h"

#include <KDebug>

EditProfileWidget::EditProfileWidget(QWidget *parent)
    : QWidget(parent),
      m_tabWidget(0),
      m_gsWidget(0),
      m_wsWidget(0)
{
    m_tabWidget = new QTabWidget(this);
    m_gsWidget = new GeneralSettingsWidget(m_tabWidget);
    m_wsWidget = new WirelessSettingsWidget(m_tabWidget);

    m_tabWidget->addTab(m_gsWidget, i18n("General Settings"));
    m_tabWidget->addTab(m_wsWidget, i18n("Wireless Settings"));
}

EditProfileWidget::~EditProfileWidget()
{
}

#include "editprofilewidget.moc"
