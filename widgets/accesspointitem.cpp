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

#include "accesspointitem.h"

#include <QSize>

AccessPointItem::AccessPointItem(AcessPoint *ap, QWidget *parent=0)
    : QGraphicsItem(parent),
      m_ap(ap),
      m_icon(),
      m_textLabel()
{
    m_icon(m_ap->type());
}

AccessPointItem::~AccessPointItem()
{
    delete m_ap;
}

void AccessPointItem::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    
    QSize iconSize = m_icon->actualSize(QSize(11,11));
    m_icon.paint(painter, QRect(QPoint(0, 0), iconSize);
    m_textLabel.draw(painter, QPointF(iconSize.width() + 5, 0));
}

#include "accesspointitem.moc"
