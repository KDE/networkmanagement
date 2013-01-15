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

#include "ttlswidget.h"

#include <connection.h>
#include <settings/802-1x.h>
#include <knmserviceprefs.h>

#include "eapmethodstack.h"
#include "eapmethodsimple.h"
#include "eapmethodinnerauth_p.h"

#include "editlistdialog.h"
#include "listvalidator.h"

class TtlsWidgetPrivate : public EapMethodInnerAuthPrivate
{
public:
    bool showAdvancedSettings;
    QRegExpValidator *altSubjectValidator;
    QRegExpValidator *serversValidator;
};

TtlsWidget::TtlsWidget(Knm::Connection* connection, QWidget * parent)
: EapMethodInnerAuth(connection, *new TtlsWidgetPrivate(), parent)
{
    Q_D(TtlsWidget);
    setupUi(this);

    d->altSubjectValidator = new QRegExpValidator(QRegExp(QLatin1String("^(DNS:[a-zA-Z0-9_-]+\\.[a-zA-Z0-9_.-]+|EMAIL:[a-zA-Z0-9._-]+@[a-zA-Z0-9_-]+\\.[a-zA-Z0-9_.-]+|URI:[a-zA-Z0-9._-]+:.+|)$")), this);
    d->serversValidator = new QRegExpValidator(QRegExp(QLatin1String("^[a-zA-Z0-9_-]+\\.[a-zA-Z0-9_.-]+$")), this);

    ListValidator *altSubjectValidator = new ListValidator(this);
    altSubjectValidator->setInnerValidator(d->altSubjectValidator);
    leAltSubjectMatches->setValidator(altSubjectValidator);

    ListValidator *serversValidator = new ListValidator(this);
    serversValidator->setInnerValidator(d->serversValidator);
    leConnectToTheseServers->setValidator(d->serversValidator);

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

    d->innerAuth->registerEapMethod(Knm::Security8021xSetting::EnumPhase2auth::pap,
            new EapMethodSimple(EapMethodSimple::Pap, connection, d->innerAuth),
            i18nc("PAP inner auth method", "PAP"));
    d->innerAuth->registerEapMethod(Knm::Security8021xSetting::EnumPhase2auth::mschap,
            new EapMethodSimple(EapMethodSimple::MsChap, connection, d->innerAuth),
            i18nc("MSCHAP inner auth method", "MSCHAP"));
    d->innerAuth->registerEapMethod(Knm::Security8021xSetting::EnumPhase2auth::mschapv2,
            new EapMethodSimple(EapMethodSimple::MsChapV2, connection, d->innerAuth),
            i18nc("MSCHAPv2 inner auth method", "MSCHAPv2"));
    d->innerAuth->registerEapMethod(Knm::Security8021xSetting::EnumPhase2auth::chap,
            new EapMethodSimple(EapMethodSimple::Chap, connection, d->innerAuth),
            i18nc("CHAP inner auth method", "CHAP"));
    d->innerAuth->registerEapMethod(Knm::Security8021xSetting::EnumPhase2auth::md5,
            new EapMethodSimple(EapMethodSimple::MD5, connection, d->innerAuth),
            i18nc("MD5 inner auth method", "MD5"));
    d->innerAuth->registerEapMethod(Knm::Security8021xSetting::EnumPhase2auth::gtc,
            new EapMethodSimple(EapMethodSimple::GTC, connection, d->innerAuth),
            i18nc("GTC inner auth method", "GTC"));
    formLayout->addRow(d->innerAuth);
}

TtlsWidget::~TtlsWidget()
{
}

void TtlsWidget::emitValid()
{
    emit valid(validate());
}

bool TtlsWidget::validate() const
{
    return true;
}

void TtlsWidget::readConfig()
{
    Q_D(TtlsWidget);
    leAnonIdentity->setText(d->setting->anonymousidentity());

    if (d->setting->useSystemCaCerts()) {
        chkUseSystemCaCerts->setChecked(true);
        kurCaCert->setEnabled(false);
        kurCaCert->clear();
    } else {
        chkUseSystemCaCerts->setChecked(false);
        QString capath = d->setting->cacertasstring();
        if (!capath.isEmpty())
            kurCaCert->setUrl(capath);
    }

    if (d->setting->phase2autheap() != Knm::Security8021xSetting::EnumPhase2autheap::none) {
        d->innerAuth->setCurrentEapMethod(d->setting->phase2autheap());
    } else {
        d->innerAuth->setCurrentEapMethod(d->setting->phase2auth());
    }

    leSubjectMatch->setText(d->setting->subjectmatch());
    QStringList altsubjectmatches = d->setting->altsubjectmatches();
    leAltSubjectMatches->setText(altsubjectmatches.join(QLatin1String(", ")));
    if (!d->showAdvancedSettings) {
        QStringList servers;
        foreach (const QString &match, altsubjectmatches) {
            if (match.startsWith(QLatin1String("DNS:")))
                servers.append(match.right(match.length()-4));
        }
        leConnectToTheseServers->setText(servers.join(QLatin1String(", ")));
    }

    d->innerAuth->readConfig();
    emitValid();
}

void TtlsWidget::writeConfig()
{
    Q_D(TtlsWidget);
    // make the Setting TTLS
    d->setting->setEapFlags(Knm::Security8021xSetting::ttls);
    // TTLS specific config
    d->setting->setAnonymousidentity(leAnonIdentity->text());

    KUrl url;

    if (chkUseSystemCaCerts->isChecked()) {
        d->setting->setUseSystemCaCerts(true);
        d->setting->setCapath(QByteArray());
    } else {
        d->setting->setUseSystemCaCerts(false);
        url = kurCaCert->url();
        if (!url.directory().isEmpty() && !url.fileName().isEmpty())
            d->setting->setCacert(url.path());
    }

    QStringList altsubjectmatches = leAltSubjectMatches->text().remove(QLatin1Char(' ')).split(QLatin1Char(','), QString::SkipEmptyParts);
    if (!d->showAdvancedSettings) {
        foreach (const QString &match, leConnectToTheseServers->text().remove(QLatin1Char(' ')).split(QLatin1Char(','), QString::SkipEmptyParts)) {
            QString tempstr = QLatin1String("DNS:") + match;
            if (!altsubjectmatches.contains(tempstr))
                altsubjectmatches.append(tempstr);
        }
    }
    d->setting->setSubjectmatch(leSubjectMatch->text());
    d->setting->setAltsubjectmatches(altsubjectmatches);

    d->innerAuth->writeConfig();
}

void TtlsWidget::readSecrets()
{
    Q_D(TtlsWidget);
    d->innerAuth->readSecrets();
}

void TtlsWidget::showAltSubjectMatchesEditor()
{
    Q_D(TtlsWidget);
    EditListDialog editor;
    editor.setItems(leAltSubjectMatches->text().remove(QLatin1Char(' ')).split(QLatin1Char(','), QString::SkipEmptyParts));
    editor.setCaption(i18n("Alternative Subject Matches"));
    editor.setToolTip(i18n("<qt>This entry must be one of:<ul><li>DNS: &lt;name or ip address&gt;</li><li>EMAIL: &lt;email&gt;</li><li>URI: &lt;uri, e.g. http://www.kde.org&gt;</li></ul></qt>"));
    editor.setValidator(d->altSubjectValidator);
    if (editor.exec() == QDialog::Accepted) {
        leAltSubjectMatches->setText(editor.items().join(QLatin1String(", ")));
    }
}

void TtlsWidget::showServersEditor()
{
    Q_D(TtlsWidget);
    EditListDialog editor;
    editor.setItems(leConnectToTheseServers->text().remove(QLatin1Char(' ')).split(QLatin1Char(','), QString::SkipEmptyParts));
    editor.setCaption(i18n("Connect to these Servers"));
    editor.setValidator(d->serversValidator);
    if (editor.exec() == QDialog::Accepted) {
        leConnectToTheseServers->setText(editor.items().join(QLatin1String(", ")));
    }
}

void TtlsWidget::syncWidgetData(const QPair<QString, QString> &widgetData)
{
    Q_D(TtlsWidget);
    d->innerAuth->syncWidgetData(widgetData);
}

QPair<QString, QString> TtlsWidget::widgetData()
{
    Q_D(TtlsWidget);
    return d->innerAuth->widgetData();
}

// vim: sw=4 sts=4 et tw=100
