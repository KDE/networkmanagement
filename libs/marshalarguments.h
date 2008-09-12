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

#ifndef MARSHAL_ARGUMENTS_H
#define MARSHAL_ARGUMENTS_H

#include "types.h"

inline QDBusArgument &operator<<(QDBusArgument &argument, const QList< QList<uint> > &myarray)
{
    argument.beginArray( qMetaTypeId<QList<uint> >() );
    for ( int i = 0; i < myarray.count(); ++i ) {
        argument << myarray[i];
    }
    argument.endArray();
    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, QList< QList<uint> > &myarray)
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

inline QDBusArgument &operator<<(QDBusArgument &argument, const QVariantMapMap &mymap)
{
    argument.beginMap( QVariant::String, QVariant::Map );

    QVariantMapMap::const_iterator i = mymap.constBegin();
    while (i != mymap.constEnd()) {
        argument.beginMapEntry();
        argument << i.key() << i.value();
        argument.endMapEntry();
        ++i;
    }
    argument.endMap();
    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, QVariantMapMap &mymap)
{
    argument.beginMap();
    mymap.clear();

    while ( !argument.atEnd() ) {
        QString key;
        QVariantMap value;
        argument.beginMapEntry();
        argument >> key >> value;
        argument.endMapEntry();
        mymap.insert( key, value );
    }

    argument.endMap();
    return argument;
}

#endif
