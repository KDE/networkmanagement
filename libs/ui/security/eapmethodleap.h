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

#ifndef EAPMETHODLEAP_H
#define EAPMETHODLEAP_H

#include "eapmethod.h"
#include "ui_eapmethodleapbase.h"

class EapMethodLeap : public EapMethod, public Ui_EapMethodLeapBase
{
Q_OBJECT
Q_DECLARE_PRIVATE(EapMethod)
public:
    EapMethodLeap(Knm::Connection* connection, QWidget * parent = 0 );
    virtual ~EapMethodLeap();

    bool validate() const;
    void readConfig();
    void writeConfig();
    void readSecrets();
    void syncWidgetData(const QPair<QString, QString> &widgetData);
    QPair<QString, QString> widgetData();
public Q_SLOTS:
    void setShowPasswords(bool b = true);
private Q_SLOTS:
    void emitValid();
    void passwordStorageChanged(int);
};

#endif // EAPMETHODLEAP_H
