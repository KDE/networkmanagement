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

#include "apitemdelegate.h"

#include <QSize>
#include <KDebug>

ApItemDelegate::ApItemDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

ApItemDelegate::~ApItemDelegate()
{
}

void ApItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    KIcon encryptedIcon = KIcon(index.data(ApItemModel::EncryptionRole).toString());
    encryptedIcon.paint(painter, QRect(option.rect.topLeft(),option.decorationSize));

    QSize iconSize = encryptedIcon.actualSize(option.decorationSize);
    QStyleOptionViewItem delegateOption = option;
    delegateOption.rect.translate(iconSize.width(),0);
    delegateOption.rect.setWidth(option.rect.width() - iconSize.width());
    QItemDelegate::paint(painter, delegateOption, index);
}

QSize ApItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    KIcon encryptedIcon = KIcon(index.data(ApItemModel::EncryptionRole).toString());
    QSize rect = encryptedIcon.actualSize(option.decorationSize);

    return rect;
}

#include "apitemdelegate.moc"
