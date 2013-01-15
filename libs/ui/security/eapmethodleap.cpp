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
    connect(cmbPasswordStorage, SIGNAL(currentIndexChanged(int)), this, SLOT(passwordStorageChanged(int)));
    connect(leUsername, SIGNAL(textChanged(QString)), SLOT(emitValid()));
    connect(lePassword, SIGNAL(textChanged(QString)), SLOT(emitValid()));
}

EapMethodLeap::~EapMethodLeap()
{
}

void EapMethodLeap::emitValid()
{
    emit valid(validate());
}

bool EapMethodLeap::validate() const
{
    return !((lePassword->text().isEmpty() && cmbPasswordStorage->currentIndex() == EapMethodPrivate::Store) || leUsername->text().isEmpty());
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
    switch (cmbPasswordStorage->currentIndex()) {
        case EapMethodPrivate::Store:
            d->setting->setPassword(lePassword->text());
            if (!d->connection->permissions().isEmpty())
                d->setting->setPasswordflags(Knm::Setting::AgentOwned);
            else
                d->setting->setPasswordflags(Knm::Setting::None);
            break;
        case EapMethodPrivate::AlwaysAsk:
            d->setting->setPasswordflags(Knm::Setting::NotSaved);
            break;
        case EapMethodPrivate::NotRequired:
            d->setting->setPasswordflags(Knm::Setting::NotRequired);
            break;
    }
    d->setting->setUseSystemCaCerts(false);
}

void EapMethodLeap::readSecrets()
{
    Q_D(EapMethod);
    if (d->setting->passwordflags().testFlag(Knm::Setting::AgentOwned) || d->setting->passwordflags().testFlag(Knm::Setting::None)) {
        lePassword->setText(d->setting->password());
        cmbPasswordStorage->setCurrentIndex(EapMethodPrivate::Store);
    } else if (d->setting->passwordflags().testFlag(Knm::Setting::NotSaved)) {
        cmbPasswordStorage->setCurrentIndex(EapMethodPrivate::AlwaysAsk);
    } else if (d->setting->passwordflags().testFlag(Knm::Setting::NotRequired)){
        cmbPasswordStorage->setCurrentIndex(EapMethodPrivate::NotRequired);
    }
}

void EapMethodLeap::setShowPasswords(bool on)
{
    lePassword->setPasswordMode(!on);
}

void EapMethodLeap::passwordStorageChanged(int type)
{
    switch (type)
    {
        case EapMethodPrivate::Store:
            lePassword->setEnabled(true);
            break;
        default:
            lePassword->setEnabled(false);
            break;
    }
    emitValid();
}

void EapMethodLeap::syncWidgetData(const QPair<QString, QString> &widgetData)
{
    leUsername->setText(widgetData.first);
    lePassword->setText(widgetData.second);
}

QPair<QString, QString> EapMethodLeap::widgetData()
{
    return QPair<QString, QString>(leUsername->text(), lePassword->text());
}

// vim: sw=4 sts=4 et tw=100
