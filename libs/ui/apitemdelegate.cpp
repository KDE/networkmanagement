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
    KIcon encryptedIcon(index.data(NetworkItemModel::EncryptionRole).value<QIcon>());
    encryptedIcon.paint(painter, QRect(option.rect.topLeft(),option.decorationSize));

    QSize iconSize = encryptedIcon.actualSize(option.decorationSize);
    QStyleOptionViewItem delegateOption = option;
    delegateOption.rect.translate(iconSize.width(),0);
    delegateOption.rect.setWidth(option.rect.width() - iconSize.width());
    delegateOption.decorationSize = QSize(-1, -1);
    QItemDelegate::paint(painter, delegateOption, index);
}

QSize ApItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    KIcon encryptedIcon = KIcon(index.data(NetworkItemModel::EncryptionRole).toString());
    QSize rect = encryptedIcon.actualSize(option.decorationSize);

    return rect;
}

