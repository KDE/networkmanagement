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

#include "editlistdialog.h"
#include "listvalidator.h"

#include <connection.h>
#include <settings/802-1x.h>
#include <knmserviceprefs.h>

#include "../../../libs/internals/paths.h"
#include "eapmethod_p.h"

class TlsWidgetPrivate : public EapMethodPrivate
{
public:
    TlsWidgetPrivate(bool isInnerMethod)
        : inner(isInnerMethod)
    {

    }
    bool inner;
    bool showAdvancedSettings;
    QRegExpValidator *altSubjectValidator;
    QRegExpValidator *serversValidator;
};

TlsWidget::TlsWidget(bool isInnerMethod, Knm::Connection* connection, QWidget * parent)
: EapMethod(*new TlsWidgetPrivate(isInnerMethod), connection, parent)
{
    Q_D(TlsWidget);
    setupUi(this);
    connect(leIdentity, SIGNAL(textChanged(QString)), SLOT(emitValid()));
    d->altSubjectValidator = new QRegExpValidator(QRegExp(QLatin1String("^(DNS:[a-zA-Z0-9_-]+\\.[a-zA-Z0-9_.-]+|EMAIL:[a-zA-Z0-9._-]+@[a-zA-Z0-9_-]+\\.[a-zA-Z0-9_.-]+|URI:[a-zA-Z0-9._-]+:.+|)$")), this);
    d->serversValidator = new QRegExpValidator(QRegExp(QLatin1String("^[a-zA-Z0-9_-]+\\.[a-zA-Z0-9_.-]+$")), this);

    ListValidator *altSubjectValidator = new ListValidator(this);
    altSubjectValidator->setInnerValidator(d->altSubjectValidator);
    leAltSubjectMatches->setValidator(altSubjectValidator);

    ListValidator *serversValidator = new ListValidator(this);
    serversValidator->setInnerValidator(d->serversValidator);
    leConnectToTheseServers->setValidator(d->serversValidator);

    KNetworkManagerServicePrefs::instance(Knm::NETWORKMANAGEMENT_RCFILE);
    KNetworkManagerServicePrefs::self()->readConfig();
    d->showAdvancedSettings = KNetworkManagerServicePrefs::self()->showAdvancedSettings();
    if (d->showAdvancedSettings) {
        lblConnectToTheseServers->hide();
        leConnectToTheseServers->hide();
        connectToTheseServersMoreBtn->hide();
        connect(altSubjectMatchesMoreBtn, SIGNAL(clicked()), this, SLOT(showAltSubjectMatchesEditor()));
    } else {
        lblSubjectMatch->hide();
        leSubjectMatch->hide();
        lblAltSubjectMatches->hide();
        leAltSubjectMatches->hide();
        altSubjectMatchesMoreBtn->hide();
        connect(connectToTheseServersMoreBtn, SIGNAL(clicked()), this, SLOT(showServersEditor()));
    }

    connect(cmbPrivateKeyPasswordStorage, SIGNAL(currentIndexChanged(int)), this, SLOT(privateKeyPasswordStorageChanged(int)));
}

TlsWidget::~TlsWidget()
{
}

void TlsWidget::emitValid()
{
    emit valid(validate());
}

bool TlsWidget::validate() const
{
    // Connection dialog crashes when saving a connection with empty identity.
    // https://bugs.kde.org/show_bug.cgi?id=307496
    return !leIdentity->text().isEmpty();
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

    QStringList altsubjectmatches;
    if (d->inner) {
        leSubjectMatch->setText(d->setting->phase2subjectmatch());
        altsubjectmatches = d->setting->phase2altsubjectmatches();
    }
    else {
        leSubjectMatch->setText(d->setting->subjectmatch());
        altsubjectmatches = d->setting->altsubjectmatches();
    }
    leAltSubjectMatches->setText(altsubjectmatches.join(QLatin1String(", ")));
    if (!d->showAdvancedSettings) {
        QStringList servers;
        foreach (const QString &match, altsubjectmatches) {
            if (match.startsWith(QLatin1String("DNS:")))
                servers.append(match.right(match.length()-4));
        }
        leConnectToTheseServers->setText(servers.join(QLatin1String(", ")));
    }
    emitValid();
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
                if (!d->connection->permissions().isEmpty())
                    d->setting->setPhase2privatekeypasswordflags(Knm::Setting::AgentOwned);
                else
                    d->setting->setPhase2privatekeypasswordflags(Knm::Setting::None);
            } else {
                d->setting->setPrivatekeypassword(lePrivateKeyPassword->text());
                if (!d->connection->permissions().isEmpty())
                    d->setting->setPrivatekeypasswordflags(Knm::Setting::AgentOwned);
                else
                    d->setting->setPrivatekeypasswordflags(Knm::Setting::None);
            }
            break;
        case EapMethodPrivate::AlwaysAsk:
            d->inner ? d->setting->setPhase2privatekeypasswordflags(Knm::Setting::NotSaved) : d->setting->setPrivatekeypasswordflags(Knm::Setting::NotSaved);
            break;
        case EapMethodPrivate::NotRequired:
            d->inner ? d->setting->setPhase2privatekeypasswordflags(Knm::Setting::NotRequired) : d->setting->setPrivatekeypasswordflags(Knm::Setting::NotRequired);
            break;
    }

    QStringList altsubjectmatches = leAltSubjectMatches->text().remove(QLatin1Char(' ')).split(QLatin1Char(','), QString::SkipEmptyParts);
    if (!d->showAdvancedSettings) {
        foreach (const QString &match, leConnectToTheseServers->text().remove(QLatin1Char(' ')).split(QLatin1Char(','), QString::SkipEmptyParts)) {
            QString tempstr = QLatin1String("DNS:") + match;
            if (!altsubjectmatches.contains(tempstr))
                altsubjectmatches.append(tempstr);
        }
    }
    if (d->inner) {
        d->setting->setPhase2subjectmatch(leSubjectMatch->text());
        d->setting->setPhase2altsubjectmatches(altsubjectmatches);
    } else {
        d->setting->setSubjectmatch(leSubjectMatch->text());
        d->setting->setAltsubjectmatches(altsubjectmatches);
    }
}

void TlsWidget::readSecrets()
{
    Q_D(TlsWidget);
    QString password;
    Knm::Setting::secretsTypes flags;
    if (d->inner) {
        password = d->setting->phase2privatekeypassword();
        flags = d->setting->phase2privatekeypasswordflags();
    } else {
        password = d->setting->privatekeypassword();
        flags = d->setting->privatekeypasswordflags();
    }
    if (flags.testFlag(Knm::Setting::AgentOwned) || flags.testFlag(Knm::Setting::None)) {
        lePrivateKeyPassword->setText(password);
        cmbPrivateKeyPasswordStorage->setCurrentIndex(EapMethodPrivate::Store);
    } else if (flags.testFlag(Knm::Setting::NotSaved)) {
        cmbPrivateKeyPasswordStorage->setCurrentIndex(EapMethodPrivate::AlwaysAsk);
    } else if (flags.testFlag(Knm::Setting::NotRequired)) {
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

void TlsWidget::showAltSubjectMatchesEditor()
{
    Q_D(TlsWidget);
    EditListDialog editor;
    editor.setItems(leAltSubjectMatches->text().remove(QLatin1Char(' ')).split(QLatin1Char(','), QString::SkipEmptyParts));
    editor.setCaption(i18n("Alternative Subject Matches"));
    editor.setToolTip(i18n("<qt>This entry must be one of:<ul><li>DNS: &lt;name or ip address&gt;</li><li>EMAIL: &lt;email&gt;</li><li>URI: &lt;uri, e.g. http://www.kde.org&gt;</li></ul></qt>"));
    editor.setValidator(d->altSubjectValidator);
    if (editor.exec() == QDialog::Accepted) {
        leAltSubjectMatches->setText(editor.items().join(QLatin1String(", ")));
    }
}

void TlsWidget::showServersEditor()
{
    Q_D(TlsWidget);
    EditListDialog editor;
    editor.setItems(leConnectToTheseServers->text().remove(QLatin1Char(' ')).split(QLatin1Char(','), QString::SkipEmptyParts));
    editor.setCaption(i18n("Connect to these Servers"));
    editor.setValidator(d->serversValidator);
    if (editor.exec() == QDialog::Accepted) {
        leConnectToTheseServers->setText(editor.items().join(QLatin1String(", ")));
    }
}

void TlsWidget::syncWidgetData(const QPair<QString, QString> &widgetData)
{
    kurPrivateKey->setUrl(widgetData.first);
    lePrivateKeyPassword->setText(widgetData.second);
}

QPair<QString, QString> TlsWidget::widgetData()
{
    return QPair<QString, QString>(kurPrivateKey->text(), lePrivateKeyPassword->text());
}

// vim: sw=4 sts=4 et tw=100
