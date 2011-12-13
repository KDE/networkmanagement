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

    /**
     * Add an EAP method to the stack
     * @param key uniquely identifying key - duplicates will be replaced!
     */
    virtual void registerEapMethod(int key, EapMethod * eapMethod, const QString & label);
    void setCurrentEapMethod(int key);
    EapMethod * currentEapMethod() const;
    
    // SecurityWidget methods
    virtual bool validate() const;
    virtual void readConfig();
    virtual void writeConfig();
    virtual void readSecrets();
    void setShowPasswords(bool on);
    void syncWidgetData(const QPair<QString, QString> &widgetData);
    QPair<QString, QString> widgetData();
private Q_SLOTS:
    void setCurrentEapMethodInternal(int);
protected:
    EapMethodStack(EapMethodStackPrivate &dd, Knm::Connection* connection, QWidget * parent = 0);
};
#endif // EAPMETHODSTACK_H
