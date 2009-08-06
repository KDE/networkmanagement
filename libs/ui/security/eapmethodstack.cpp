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

#include "eapmethodstack.h"
#include "eapmethodstack_p.h"


#include <connection.h>
#include "eapmethod.h"

EapMethodStack::EapMethodStack(EapMethodStackPrivate &dd, Knm::Connection* connection, QWidget * parent)
: SecurityWidget(dd, connection, parent )
{
    setupUi(this);
    QWidget * wid = eapMethods->currentWidget();
    eapMethods->removeWidget(wid);
    delete wid;
    connect(cboEapMethod, SIGNAL(currentIndexChanged(int)), eapMethods, SLOT(setCurrentIndex(int)));
}

EapMethodStack::EapMethodStack(Knm::Connection* connection, QWidget * parent)
: SecurityWidget(*new EapMethodStackPrivate, connection, parent )
{
    setupUi(this);
    QWidget * wid = eapMethods->currentWidget();
    eapMethods->removeWidget(wid);
    delete wid;
    connect(cboEapMethod, SIGNAL(currentIndexChanged(int)), eapMethods, SLOT(setCurrentIndex(int)));
}

EapMethodStack::~EapMethodStack()
{
}

void EapMethodStack::registerEapMethod(EapMethod * eapMethod, const QString & theLabel, int & index)
{
    cboEapMethod->addItem(theLabel);
    index = eapMethods->addWidget(eapMethod);
}

void EapMethodStack::setCurrentEapMethod(int index)
{
    if (index >= 0 && index < eapMethods->count()) {
        cboEapMethod->setCurrentIndex(index);
    }
}

EapMethod * EapMethodStack::currentEapMethod() const
{
    return static_cast<EapMethod *>(eapMethods->currentWidget());
}

bool EapMethodStack::validate() const
{
    if (eapMethods->count()) {
        return qobject_cast<EapMethod *>( eapMethods->currentWidget())->validate();
    }
    return true;
}

void EapMethodStack::readConfig()
{
    if (eapMethods->count()) {
        qobject_cast<EapMethod *>( eapMethods->currentWidget())->readConfig();
    }
}

void EapMethodStack::writeConfig()
{
    if (eapMethods->count()) {
        qobject_cast<EapMethod *>( eapMethods->currentWidget())->writeConfig();
    }
}

void EapMethodStack::readSecrets()
{
    if (eapMethods->count()) {
        qobject_cast<EapMethod *>( eapMethods->currentWidget())->readSecrets();
    }
}

void EapMethodStack::setPasswordMode(bool on)
{
    if (eapMethods->count()) {
        qobject_cast<EapMethod *>( eapMethods->currentWidget())->setPasswordMode(on);
    }
}

// vim: sw=4 sts=4 et tw=100
