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

#ifndef MARSHALL_ARGUMENTS_H
#define MARSHALL_ARGUMENTS_H

#include <QMap>
#include <QVariant>
#include <QDbus>
#include <QDbusArgument>

QT_DECLARE_METATYPE(QList<uint>)
QT_DECLARE_METATYPE(QList< QList<uint> >)
QT_DECLARE_METATYPE(QList<QDbusObjectPath>)
QT_DECLARE_METATYPE(QMap<QString, QVariant>)
QT_DECLARE_METATYPE(QMap<QString, QMap<QString, QVariant> >)

QDBusArgument &operator<<(QDBusArgument &argument, const QList< QList<uint> > &myarray)
{
    argument.beginArray( qMetaTypeId<QList<uint> >() );
    for ( int i = 0; i < myarray.length; ++i ) {
        argument << myarray.elements[i];
    }
    argument.endArray();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, QList< QList<uint> > &myarray)
{
    argument.beginArray();
    myarray.clear();

    while ( !argument.atEnd() ) {
        QList<uint> element;
        argument >> element;
        myarray.append( element );
    }

    argument.endArray();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const QMap<QString, QMap<QString, QVariant> > &mymap)
{
    argument.beginMap( QVariant::String, qMetaTypeId<QMap<QString, QVariant> >() );
    for ( int i = 0; i < mydict.length; ++i ) {
        argument.beginMapEntry();
        argument << mymap.data[i].key << mymap.data[i].value;
        argument.endMapEntry();
    }
    argument.endMap();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, QMap<QString, QMap<QString, QVariant> > &mymap)
{
    argument.beginMap();
    mymap.clear();

    while ( !argument.atEnd() ) {
        QString key;
        QMap<QString, QVariant> value;
        argument.beginMapEntry();
        argument >> key >> value;
        argument.endMapEntry();
        mymap.append( key, value );
    }

    argument.endMap();
    return argument;
}

#endif