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

#include "eapmethodleap.h"

#include <connection.h>
#include <settings/802-1x.h>
#include <settings/802-11-wireless-security.h>

#include "eapmethod_p.h"

EapMethodLeap::EapMethodLeap(Knm::Connection* connection, QWidget * parent)
: EapMethod(connection, parent)
{
    setupUi(this);
}

EapMethodLeap::~EapMethodLeap()
{
}

bool EapMethodLeap::validate() const
{
    return true;
}

void EapMethodLeap::readConfig()
{
    Q_D(EapMethod);

    leUsername->setText(d->setting->identity());
}

void EapMethodLeap::writeConfig()
{
    Q_D(EapMethod);
    // make the Setting LEAP
    d->setting->setEapFlags(Knm::Security8021xSetting::leap);
    // LEAP stuff
    d->setting->setIdentity(leUsername->text());
    d->setting->setPassword(lePassword->text());
    kDebug() << "Lamarque" << lePassword->text();
    d->setting->setUseSystemCaCerts(false);
    d->setting->remove();
}

void EapMethodLeap::readSecrets()
{
    Q_D(EapMethod);
    lePassword->setText(d->setting->password());
}

void EapMethodLeap::setShowPasswords(bool on)
{
    lePassword->setPasswordMode(!on);
}

// vim: sw=4 sts=4 et tw=100
