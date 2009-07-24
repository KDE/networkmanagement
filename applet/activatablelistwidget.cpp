/*
Copyright 2008 Sebastian Kügler <sebas@kde.org>

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

#include "activatablelistwidget.h"

#include <QGraphicsLinearLayout>

#include <Plasma/Label>

#include <KDebug>

ActivatableListWidget::ActivatableListWidget(QGraphicsWidget * parent) : Plasma::ScrollWidget(parent),
    m_layout(0)
{
    m_widget = new QGraphicsWidget(this);
    m_layout = new QGraphicsLinearLayout(m_widget);
    m_layout->setOrientation(Qt::Vertical);
    m_widget->setLayout(m_layout);
    setWidget(m_widget);

    Plasma::Label* l = new Plasma::Label(m_widget);
    l->setText("Connection 1");
    m_layout->addItem(l);

    Plasma::Label* l2 = new Plasma::Label(m_widget);
    l2->setText("Connection 2");
    m_layout->addItem(l2);
}

ActivatableListWidget::~ActivatableListWidget()
{
}
// vim: sw=4 sts=4 et tw=100
