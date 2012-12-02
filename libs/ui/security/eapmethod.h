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

#ifndef EAPMETHOD_H
#define EAPMETHOD_H

#include <QWidget>

#include "knm_export.h"

namespace Knm
{
    class Connection;
} // namespace Knm

class EapMethodPrivate;

class KNM_EXPORT EapMethod : public QWidget
{
Q_OBJECT
Q_DECLARE_PRIVATE(EapMethod)
public:
    EapMethod(Knm::Connection * connection, QWidget * parent = 0);
    virtual ~EapMethod();

    virtual bool validate() const = 0;
    virtual void readConfig() = 0;
    virtual void writeConfig() = 0;
    virtual void readSecrets() = 0;
    virtual void setShowPasswords(bool b = true) = 0;
    virtual void syncWidgetData(const QPair<QString, QString> &widgetData) = 0;
    virtual QPair<QString, QString> widgetData() = 0;
Q_SIGNALS:
    void valid(bool);
protected:
    EapMethod(EapMethodPrivate &dd, Knm::Connection * connection, QWidget * parent = 0);
    EapMethodPrivate * d_ptr;
};

#endif // EAPMETHOD_H
