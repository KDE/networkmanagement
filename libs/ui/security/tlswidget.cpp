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

#include <kfiledialog.h>
#include <KUser>

class TlsWidgetPrivate : public EapMethodPrivate
{
public:
    TlsWidgetPrivate(bool isInnerMethod)
        : inner(isInnerMethod)
    {

    }
    bool inner;
    enum Certs {ClientCert = 0, CACert, PrivateKey};
};

TlsWidget::TlsWidget(bool isInnerMethod, Knm::Connection* connection, QWidget * parent)
: EapMethod(*new TlsWidgetPrivate(isInnerMethod), connection, parent)
{
    setupUi(this);
    connect(chkUseSystemCaCerts,SIGNAL(toggled(bool)),this,SLOT(toggleSystemCa(bool)));

    connect(clientCertLoad,SIGNAL(clicked()),this,SLOT(loadCert()));
    connect(caCertLoad,SIGNAL(clicked()),this,SLOT(loadCert()));
    connect(privateKeyLoad,SIGNAL(clicked()),this,SLOT(loadCert()));
    clientCertLoad->setIcon(KIcon("document-open"));
    caCertLoad->setIcon(KIcon("document-open"));
    privateKeyLoad->setIcon(KIcon("document-open"));
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
        caCertLoad->setEnabled(false);
    } else {
        if (!d->setting->cacert().isEmpty()) {
            setText(d->CACert,true);
        } else {
            setText(d->CACert,false);
        }
    }

    if (!d->setting->clientcert().isEmpty()) {
        setText(d->ClientCert,true);
    } else {
        setText(d->ClientCert,false);
    }

    if (!d->setting->privatekey().isEmpty()) {
        setText(d->PrivateKey,true);
    } else {
        setText(d->PrivateKey,false);
    }
}

void TlsWidget::writeConfig()
{
    Q_D(TlsWidget);
    if (!d->inner) {
        // make it TLS
        d->setting->setEapFlags(Knm::Security8021xSetting::tls);
        d->setting->addToCertToDelete(Knm::Security8021xSetting::Phase2CACert);
        d->setting->addToCertToDelete(Knm::Security8021xSetting::Phase2ClientCert);
        d->setting->addToCertToDelete(Knm::Security8021xSetting::Phase2PrivateKey);
        d->setting->setPhase2cacerttoimport("");
        d->setting->setPhase2clientcerttoimport("");
        d->setting->setPhase2privatekeytoimport("");
        d->setting->setPrivatekeypassword(lePrivateKeyPassword->text());
    } else {
        d->setting->addToCertToDelete(Knm::Security8021xSetting::CACert);
        d->setting->addToCertToDelete(Knm::Security8021xSetting::ClientCert);
        d->setting->addToCertToDelete(Knm::Security8021xSetting::PrivateKey);
        d->setting->setCacerttoimport("");
        d->setting->setClientcerttoimport("");
        d->setting->setPrivatekeytoimport("");
        d->setting->setPhase2privatekeypassword(lePrivateKeyPassword->text());
    }

    // TLS specifics
    // nm-applet also writes this if TLS is inner, but it feels dodgy to me
    d->setting->setIdentity(leIdentity->text());

    KUrl url;
    if (chkUseSystemCaCerts->isChecked()) {
        d->setting->setUseSystemCaCerts(true);
        d->setting->addToCertToDelete(Knm::Security8021xSetting::CACert);
        d->setting->addToCertToDelete(Knm::Security8021xSetting::Phase2CACert);
    } else {
        d->setting->setUseSystemCaCerts(false);
    }
}

void TlsWidget::readSecrets()
{
    Q_D(TlsWidget);
    if (d->inner) {
        lePrivateKeyPassword->setText(d->setting->phase2privatekeypassword());
    } else {
        lePrivateKeyPassword->setText(d->setting->privatekeypassword());
    }
}

void TlsWidget::setShowPasswords(bool on)
{
    lePrivateKeyPassword->setPasswordMode(!on);
}

void TlsWidget::loadCert()
{
    Q_D(TlsWidget);
    QString objectname = sender()->objectName();
    if (d->inner) {
        if (objectname == QLatin1String("clientCertLoad")) {
            QString newcert = KFileDialog::getOpenFileName(KUser().homeDir(),"",this,i18nc("File chooser dialog title for certificate loading","Load Certificate"));
            if (!newcert.isEmpty()) {
                d->setting->setPhase2clientcerttoimport(newcert);
                setText(d->ClientCert,true);
            }
        } else if (objectname == QLatin1String("caCertLoad")) {
            QString newcert = KFileDialog::getOpenFileName(KUser().homeDir(),"",this,i18nc("File chooser dialog title for certificate loading","Load Certificate"));
            if (!newcert.isEmpty()) {
                d->setting->setPhase2cacerttoimport(newcert);
                setText(d->CACert,true);
            }
        } else if (objectname == QLatin1String("privateKeyLoad")) {
            QString newcert = KFileDialog::getOpenFileName(KUser().homeDir(),"",this,i18nc("File chooser dialog title for certificate loading","Load Certificate"));
            if (!newcert.isEmpty()) {
                d->setting->setPhase2privatekeytoimport(newcert);
                setText(d->PrivateKey,true);
            }
        }
    } else {
        if (objectname == QLatin1String("clientCertLoad")) {
            QString newcert = KFileDialog::getOpenFileName(KUser().homeDir(),"",this,i18nc("File chooser dialog title for certificate loading","Load Certificate"));
            if (!newcert.isEmpty()) {
                d->setting->setClientcerttoimport(newcert);
                setText(d->ClientCert,true);
            }
        } else if (objectname == QLatin1String("caCertLoad")) {
            QString newcert = KFileDialog::getOpenFileName(KUser().homeDir(),"",this,i18nc("File chooser dialog title for certificate loading","Load Certificate"));
            if (!newcert.isEmpty()) {
                d->setting->setCacerttoimport(newcert);
                setText(d->CACert,true);
            }
        } else if (objectname == QLatin1String("privateKeyLoad")) {
            QString newcert = KFileDialog::getOpenFileName(KUser().homeDir(),"",this,i18nc("File chooser dialog title for certificate loading","Load Certificate"));
            if (!newcert.isEmpty()) {
                d->setting->setPrivatekeytoimport(newcert);
                setText(d->PrivateKey,true);
            }
        }
    }
}

void TlsWidget::toggleSystemCa(bool toggled)
{
    Q_D(TlsWidget);
    if (toggled)
        setText(TlsWidgetPrivate::CACert,false);
    else if (d->inner && !d->setting->phase2cacert().isEmpty())
        setText(TlsWidgetPrivate::CACert,true);
    else if (!d->setting->cacert().isEmpty())
        setText(TlsWidgetPrivate::CACert,true);
}

void TlsWidget::setText(int cert, bool loaded)
{
    KPushButton *button;
    QLabel *label;
    KLed *led;
    switch (cert)
    {
        case TlsWidgetPrivate::ClientCert:
            button = clientCertLoad;
            label = clientCertLoadedLabel;
            led = clientCertLed;
            break;
        case TlsWidgetPrivate::CACert:
            button = caCertLoad;
            label = caCertLoadedLabel;
            led = caCertLed;
            break;
        case TlsWidgetPrivate::PrivateKey:
        default:
            button = privateKeyLoad;
            label = privateKeyLoadedLabel;
            led = privateKeyLed;
            break;
    }
    if (loaded) {
        button->setText(i18nc("Text to display on certificate button a certificate is already loaded","Load new"));
        label->setText(i18nc("Text to display on CA certificate LED label when certificate is already loaded","Loaded"));
        led->setState(KLed::On);
    } else {
        button->setText(i18nc("Text to display on CA certificate button when no certificate is loaded yet","Load"));
        label->setText("");
        led->setState(KLed::Off);
    }
}

// vim: sw=4 sts=4 et tw=100
