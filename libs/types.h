/*  This file is part of the KDE project
    Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>
    Copyright (C) 2008 Will Stephenson <wstephenson@kde.org>

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

#ifndef KNM07_TYPES_H
#define KNM07_TYPES_H

#include <QMap>
#include <QVariant>
#include <QtDBus/QtDBus>
#include <QtDBus/QDBusArgument>

Q_DECLARE_METATYPE(QList<uint>)
Q_DECLARE_METATYPE(QList< QList<uint> >)
typedef QList< QList<uint> > UintListList;
Q_DECLARE_METATYPE(QVariantMap)
typedef QMap<QString, QMap<QString, QVariant> > QVariantMapMap;
Q_DECLARE_METATYPE(QVariantMapMap)
typedef QMap<QString, QString> QStringMap;
typedef QMapIterator<QString, QString> QStringMapIterator;
Q_DECLARE_METATYPE(QStringMap)

#endif
