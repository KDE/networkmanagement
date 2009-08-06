/*
Copyright 2008 Helmut Schaa <helmut.schaa@googlemail.com>
Copyright 2009 Will Stephenson <wstephenson@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "eapmethod.h"

#include <connection.h>
#include <settings/802-1x.h>

#include "eapmethod_p.h"

EapMethod::EapMethod(EapMethodPrivate &dd, Knm::Connection * connection, QWidget * parent)
: QWidget(parent), d_ptr(&dd)
{
    Q_D(EapMethod);
    d->connection = connection;
    d->setting = static_cast<Knm::Security8021xSetting *>(connection->setting(Knm::Setting::Security8021x));
}

EapMethod::EapMethod(Knm::Connection * connection, QWidget * parent)
: QWidget(parent), d_ptr(new EapMethodPrivate)
{
    Q_D(EapMethod);
    d->connection = connection;
    d->setting = static_cast<Knm::Security8021xSetting *>(connection->setting(Knm::Setting::Security8021x));
}

EapMethod::~EapMethod()
{
    delete d_ptr;
}

// vim: sw=4 sts=4 et tw=100
