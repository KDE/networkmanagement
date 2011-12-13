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

#ifndef EAPMETHODSIMPLE_H
#define EAPMETHODSIMPLE_H

#include "eapmethod.h"
#include "ui_eapmethodsimplebase.h"

class EapMethodSimple : public EapMethod, public Ui_EapMethodSimpleBase
{
Q_OBJECT
Q_DECLARE_PRIVATE(EapMethod)
public:
    enum Type { Pap = 0, MsChap, MsChapV2, MD5, Chap, GTC };
    EapMethodSimple(Type type, Knm::Connection * connection, QWidget * parent = 0);
    virtual ~EapMethodSimple();
    bool validate() const;
    void readConfig();
    void writeConfig();
    void readSecrets();
    void setShowPasswords(bool on = true);
    void syncWidgetData(const QPair<QString, QString> &widgetData);
    QPair<QString, QString> widgetData();

private Q_SLOTS:
    void passwordStorageChanged(int);
private:
    Type m_type;
};

#endif // EAPMETHODSIMPLE_H
