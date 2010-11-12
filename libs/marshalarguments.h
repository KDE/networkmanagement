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
