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

#ifndef NETWORKPROFILE_H
#define NETWORKPROFILE_H

#include <QString>

class NetworkProfile
{
    public:
        enum Type {Default=0, Wireless, Home, Work, Airport, Cafe, Hotel};
        
        NetworkProfile();
        NetworkProfile(const QString &name, Type profileType);

        QString name() const;
        void setName(const QString &name);

        Type type() const;
        void setType(Type profileType);

        QString icon() const;

        static QString typeAsString(Type profileType);
        static Type stringAsType(const QString &profileString);
        static QStringList types();
        static QString icon(Type type);
        static QString icon(const QString &type);

    private:
        QString m_name;
        Type m_type;
};

#endif
