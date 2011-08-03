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

#include "tlswidget.h"

#include "connection.h"
#include "settings/802-1x.h"

#include "eapmethod_p.h"

class TlsWidgetPrivate : public EapMethodPrivate
{
public:
    TlsWidgetPrivate(bool isInnerMethod)
        : inner(isInnerMethod)
    {

    }
    bool inner;
};

TlsWidget::TlsWidget(bool isInnerMethod, Knm::Connection* connection, QWidget * parent)
: EapMethod(*new TlsWidgetPrivate(isInnerMethod), connection, parent)
{
    setupUi(this);
    connect(cmbPrivateKeyPasswordStorage, SIGNAL(currentIndexChanged(int)), this, SLOT(privateKeyPasswordStorageChanged(int)));
}

TlsWidget::~TlsWidget()
{
}

bool TlsWidget::validate() const
{
    return true;
}

void TlsWidget::readConfig()
{
    Q_D(TlsWidget);
    leIdentity->setText(d->setting->identity());

    QString value;
    if (d->setting->useSystemCaCerts()) {
        chkUseSystemCaCerts->setChecked(true);
        kurCaCert->setEnabled(false);
        kurClientCert->clear();
    } else {

        if (d->inner) {
            value = d->setting->phase2cacertasstring();
        } else {
            value = d->setting->cacertasstring();
        }
        if (!value.isEmpty())
            kurCaCert->setUrl(value);
    }

    if (d->inner) {
        value = d->setting->phase2clientcertasstring();
    } else {
        value = d->setting->clientcertasstring();
    }
    if (!value.isEmpty())
        kurClientCert->setUrl(value);

    if (d->inner) {
        value = d->setting->phase2privatekeyasstring();
    } else {
        value = d->setting->privatekeyasstring();
    }
    if (!value.isEmpty())
        kurPrivateKey->setUrl(value);
}

void TlsWidget::writeConfig()
{
    Q_D(TlsWidget);
    if (!d->inner) {
        // make it TLS
        d->setting->setEapFlags(Knm::Security8021xSetting::tls);
        d->setting->setPrivatekeypassword(lePrivateKeyPassword->text());
    } else {
        d->setting->setPhase2privatekeypassword(lePrivateKeyPassword->text());
    }

    // TLS specifics
    // nm-applet also writes this if TLS is inner, but it feels dodgy to me
    d->setting->setIdentity(leIdentity->text());

    KUrl url;
    if (chkUseSystemCaCerts->isChecked()) {
        d->setting->setUseSystemCaCerts(true);
        d->setting->setPhase2capath(QByteArray());
        d->setting->setCapath(QByteArray());
    } else {
        url = kurCaCert->url();
        if (!url.directory().isEmpty() && !url.fileName().isEmpty()) {
            QString path = url.path();
            if (d->inner) {
                d->setting->setPhase2cacert(path);
            } else {
                d->setting->setCacert(path);
            }
        }
    }

    url = kurClientCert->url();
    if (!url.directory().isEmpty() && !url.fileName().isEmpty()) {
        QString path = url.path();
        if (d->inner) {
            d->setting->setPhase2clientcert(path);
        } else {
            d->setting->setClientcert(path);
        }
    }

    url = kurPrivateKey->url();
    if (!url.directory().isEmpty() && !url.fileName().isEmpty()) {
        QString path = url.path();
        if (d->inner) {
            d->setting->setPhase2privatekey(path);
        } else {
            d->setting->setPrivatekey(path);
        }
    }

    switch (cmbPrivateKeyPasswordStorage->currentIndex()) {
        case EapMethodPrivate::Store:
            if (d->inner) {
                d->setting->setPhase2privatekeypassword(lePrivateKeyPassword->text());
                d->setting->setPhase2privatekeypasswordflags(Knm::Setting::AgentOwned);
            } else {
                d->setting->setPrivatekeypassword(lePrivateKeyPassword->text());
                d->setting->setPrivatekeypasswordflags(Knm::Setting::AgentOwned);
            }
            break;
        case EapMethodPrivate::AlwaysAsk:
            d->inner ? d->setting->setPhase2privatekeypasswordflags(Knm::Setting::NotSaved) : d->setting->setPrivatekeypasswordflags(Knm::Setting::NotSaved);
            break;
        case EapMethodPrivate::NotRequired:
            d->inner ? d->setting->setPhase2privatekeypasswordflags(Knm::Setting::NotRequired) : d->setting->setPrivatekeypasswordflags(Knm::Setting::NotRequired);
            break;
    }
}

void TlsWidget::readSecrets()
{
    Q_D(TlsWidget);
    if (d->inner && (d->setting->phase2privatekeypasswordflags() & Knm::Setting::AgentOwned || d->setting->phase2privatekeypasswordflags() & Knm::Setting::None)) {
        lePrivateKeyPassword->setText(d->setting->phase2privatekeypassword());
        cmbPrivateKeyPasswordStorage->setCurrentIndex(EapMethodPrivate::Store);
    } else if (d->setting->privatekeypasswordflags() & Knm::Setting::AgentOwned || d->setting->privatekeypasswordflags() & Knm::Setting::None) {
        lePrivateKeyPassword->setText(d->setting->privatekeypassword());
        cmbPrivateKeyPasswordStorage->setCurrentIndex(EapMethodPrivate::Store);
    } else if (d->setting->passwordflags() & Knm::Setting::NotSaved) {
        cmbPrivateKeyPasswordStorage->setCurrentIndex(EapMethodPrivate::AlwaysAsk);
    } else if (d->setting->passwordflags() & Knm::Setting::NotRequired){
        cmbPrivateKeyPasswordStorage->setCurrentIndex(EapMethodPrivate::NotRequired);
    }
}

void TlsWidget::setShowPasswords(bool on)
{
    lePrivateKeyPassword->setPasswordMode(!on);
}

void TlsWidget::privateKeyPasswordStorageChanged(int type)
{
    switch (type)
    {
        case EapMethodPrivate::Store:
            lePrivateKeyPassword->setEnabled(true);
            break;
        default:
            lePrivateKeyPassword->setEnabled(false);
            break;
    }
}

// vim: sw=4 sts=4 et tw=100
