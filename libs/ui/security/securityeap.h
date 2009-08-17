/*
Copyright 2008 Helmut Schaa <helmut.schaa@googlemail.com>
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#ifndef SECURITYEAPWIDGET_H
#define SECURITYEAPWIDGET_H

#include "eapmethodstack.h"

class SecurityEapPrivate;

class SecurityEap : public EapMethodStack
{
Q_OBJECT
Q_DECLARE_PRIVATE(SecurityEap)
public:
    SecurityEap(Knm::Connection* connection, bool wireless = true, QWidget * parent = 0);
    virtual ~SecurityEap();
    virtual bool validate() const;
    virtual void readConfig();
    virtual void writeConfig();
    virtual void readSecrets();

    void registerEapMethod(int key, EapMethod * eapMethod, const QString & label);
};

#endif // SECURITYEAPWIDGET_H
