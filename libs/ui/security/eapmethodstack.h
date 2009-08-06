/*
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

#ifndef EAPMETHODSTACK_H
#define EAPMETHODSTACK_H

#include "securitywidget.h"
#include "ui_eapmethodstackbase.h"

namespace Knm
{
    class Connection;
} // namespace Knm

class EapMethod;

class EapMethodStackPrivate;

class EapMethodStack : public SecurityWidget, public Ui_EapMethodStackBase
{
Q_OBJECT
Q_DECLARE_PRIVATE(EapMethodStack)
public:
    EapMethodStack(Knm::Connection* connection, QWidget * parent = 0);
    virtual ~EapMethodStack();

    virtual void registerEapMethod(EapMethod * eapMethod, const QString & label, int & index);
    void setCurrentEapMethod(int index);
    EapMethod * currentEapMethod() const;

    // SecurityWidget methods
    bool validate() const;
    void readConfig();
    void writeConfig();
    void readSecrets();
    void setPasswordMode(bool on);
protected:
    EapMethodStack(EapMethodStackPrivate &dd, Knm::Connection* connection, QWidget * parent = 0);
};
#endif // EAPMETHODSTACK_H
