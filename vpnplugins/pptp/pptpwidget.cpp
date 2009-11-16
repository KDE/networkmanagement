/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2009 Pavel Andreev <apavelm@gmail.com>

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

#include "pptpwidget.h"
#include <KDialog>

#include <nm-setting-vpn.h>

#include "ui_pptpprop.h"
#include "ui_pptpadvanced.h"

#include <QString>
#include "nm-pptp-service.h"

#include "connection.h" 

class PptpSettingWidgetPrivate
{
public:
    Ui_PptpProp ui;
    Ui_PptpAdvanced advUi;
    Knm::VpnSetting * setting;
    KDialog * advancedDlg;
    QWidget * advancedWid;
    bool advancedIsDirty;
};

PptpSettingWidget::PptpSettingWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(connection, parent), d_ptr(new PptpSettingWidgetPrivate)
{
    Q_D(PptpSettingWidget);
    d->advancedIsDirty = false;
    d->ui.setupUi(this);

    d->setting = static_cast<Knm::VpnSetting *>(connection->setting(Knm::Setting::Vpn));

    connect(d->ui.btnAdvanced, SIGNAL(clicked()), this, SLOT(doAdvancedDialog()));

    connect(d->ui.cb_showPassword, SIGNAL(toggled(bool)), this, SLOT(setShowPassword(bool)));
    d->advancedDlg = new KDialog(this);
    d->advancedWid = new QWidget(this);
    d->advUi.setupUi(d->advancedWid);
    d->advancedDlg->setMainWidget(d->advancedWid);
}

PptpSettingWidget::~PptpSettingWidget()
{

}

void PptpSettingWidget::setShowPassword(bool show)
{
    Q_D(PptpSettingWidget);
    d->ui.edt_password->setPasswordMode(!show);
}

void PptpSettingWidget::doAdvancedDialog()
{
    Q_D(PptpSettingWidget);
    if (d->advancedDlg->exec() == QDialog::Accepted) {
        d->advancedIsDirty = true;
    }
}

void PptpSettingWidget::readConfig()
{
    Q_D(PptpSettingWidget);
    // General settings
    QStringMap dataMap = d->setting->data();

    // Authentication
    QString sGateway = dataMap[NM_PPTP_KEY_GATEWAY];
    if (!sGateway.isEmpty())
    {
        d->ui.edt_gateway->setText(sGateway);
    }

    QString sLogin = dataMap[NM_PPTP_KEY_USER];
    if (!sLogin.isEmpty())
    {
        d->ui.edt_login->setText(sLogin);
    }
    // password storage type is set in readSecrets

    QString sDomain = dataMap[NM_PPTP_KEY_DOMAIN];
    if (!sDomain.isEmpty())
    {
        d->ui.edt_ntDomain->setText(sDomain);
    }

    // Options below is belongs to "Advanced" dialog

    // Authentification options
    QString yesString = QLatin1String("yes");
    bool refuse_pap = (dataMap[NM_PPTP_KEY_REFUSE_PAP] == yesString);
    bool refuse_chap = (dataMap[NM_PPTP_KEY_REFUSE_CHAP] == yesString);
    bool refuse_mschap = (dataMap[NM_PPTP_KEY_REFUSE_MSCHAP] == yesString);
    bool refuse_mschapv2 = (dataMap[NM_PPTP_KEY_REFUSE_MSCHAPV2] == yesString);
    bool refuse_eap = (dataMap[NM_PPTP_KEY_REFUSE_EAP] == yesString);

    QListWidgetItem * item = 0;
    item = d->advUi.listWidget->item(0); // PAP
    item->setCheckState(refuse_pap ? Qt::Unchecked : Qt::Checked);
    item = d->advUi.listWidget->item(1); // CHAP
    item->setCheckState(refuse_chap ? Qt::Unchecked : Qt::Checked);
    item = d->advUi.listWidget->item(2); // MSCHAP
    item->setCheckState(refuse_mschap ? Qt::Unchecked : Qt::Checked);
    item = d->advUi.listWidget->item(3); // MSCHAPv2
    item->setCheckState(refuse_mschapv2 ? Qt::Unchecked : Qt::Checked);
    item = d->advUi.listWidget->item(4); // EAP
    item->setCheckState(refuse_eap ? Qt::Unchecked : Qt::Checked);

    // Cryptography and compression
    bool mppe = (dataMap[NM_PPTP_KEY_REQUIRE_MPPE] == yesString);
    bool mppe40 = (dataMap[NM_PPTP_KEY_REQUIRE_MPPE_40] == yesString);
    bool mppe128 = (dataMap[NM_PPTP_KEY_REQUIRE_MPPE_128] == yesString);
    bool mppe_stateful = (dataMap[NM_PPTP_KEY_MPPE_STATEFUL] == yesString);

    if (mppe || mppe40 || mppe128) { // If MPPE is use
        d->advUi.gb_MPPE->setChecked(mppe || mppe40 || mppe128);
        if (mppe128) {
            d->advUi.cb_MPPECrypto->setCurrentIndex(1); // 128 bit
        }
        else if (mppe40) {
            d->advUi.cb_MPPECrypto->setCurrentIndex(2); // 40 bit
        }
        else {
            d->advUi.cb_MPPECrypto->setCurrentIndex(0); // Any
        }
        d->advUi.cb_statefulEncryption->setChecked(mppe_stateful);
    }

    bool nobsd = (dataMap[NM_PPTP_KEY_NOBSDCOMP] == yesString);
    d->advUi.cb_BSD->setChecked(!nobsd);

    bool nodeflate = (dataMap[NM_PPTP_KEY_NODEFLATE] == yesString);
    d->advUi.cb_deflate->setChecked(!nodeflate);

    bool novjcomp = (dataMap[NM_PPTP_KEY_NO_VJ_COMP] == yesString);
    d->advUi.cb_TCPheaders->setChecked(!novjcomp);

    // Echo
    int lcp_echo_interval = QString(dataMap[NM_PPTP_KEY_LCP_ECHO_INTERVAL]).toInt();
    d->advUi.cb_sendEcho->setChecked(lcp_echo_interval > 0);
}

void PptpSettingWidget::writeConfig()
{
    Q_D(PptpSettingWidget);

    d->setting->setServiceType(QLatin1String(NM_DBUS_SERVICE_PPTP));

    // save the main dialog's data in the setting
    // if the advanced dialog is dirty, save its data in the vpn setting too
    //
    QStringMap data;
    QVariantMap secretData;

    data.insert(NM_PPTP_KEY_GATEWAY,  d->ui.edt_gateway->text().toUtf8());
    data.insert(NM_PPTP_KEY_USER, d->ui.edt_login->text().toUtf8());
    secretData.insert(QLatin1String(NM_PPTP_KEY_PASSWORD), d->ui.edt_password->text());
    if (!d->ui.edt_ntDomain->text().isEmpty()) {
        data.insert(NM_PPTP_KEY_DOMAIN,  d->ui.edt_ntDomain->text().toUtf8());
    }

    // Advanced dialog settings
    if (d->advancedIsDirty) {
        // Authenfication options
        QListWidgetItem * item = 0;
        item = d->advUi.listWidget->item(0); // PAP
        QString yesString = QLatin1String("yes");
        if (item->checkState() == Qt::Unchecked)
            data.insert(NM_PPTP_KEY_REFUSE_PAP, yesString);
        item = d->advUi.listWidget->item(1); // CHAP
        if (item->checkState() == Qt::Unchecked)
            data.insert(NM_PPTP_KEY_REFUSE_CHAP, yesString);
        item = d->advUi.listWidget->item(2); // MSCHAP
        if (item->checkState() == Qt::Unchecked)
            data.insert(NM_PPTP_KEY_REFUSE_MSCHAP, yesString);
        item = d->advUi.listWidget->item(3); // MSCHAPv2
        if (item->checkState() == Qt::Unchecked)
            data.insert(NM_PPTP_KEY_REFUSE_MSCHAPV2, yesString);
        item = d->advUi.listWidget->item(4); // EAP
        if (item->checkState() == Qt::Unchecked)
            data.insert(NM_PPTP_KEY_REFUSE_EAP, yesString);

        // Cryptography and compression
        if (d->advUi.gb_MPPE->isChecked())
        {
            int index = d->advUi.cb_MPPECrypto->currentIndex();

            switch (index)
            {
                case 0:
                    {
                        // "Any"
                        data.insert(NM_PPTP_KEY_REQUIRE_MPPE, yesString);
                    }
                    break;
                case 1:
                    {
                        // "128 bit"
                        data.insert(NM_PPTP_KEY_REQUIRE_MPPE_128, yesString);
                    }
                    break;
                case 2:
                    {
                        // "40 bit"
                        data.insert(NM_PPTP_KEY_REQUIRE_MPPE_40, yesString);
                    }
                    break;
            }

            if (d->advUi.cb_statefulEncryption->isChecked()) {
                data.insert(NM_PPTP_KEY_MPPE_STATEFUL, yesString);
            }
        }

        if (!d->advUi.cb_BSD->isChecked()) {
            data.insert(NM_PPTP_KEY_NOBSDCOMP, yesString);

        }
        if (!d->advUi.cb_deflate->isChecked()) {
            data.insert(NM_PPTP_KEY_NODEFLATE, yesString);
        }

        if (!d->advUi.cb_TCPheaders->isChecked()) {
            data.insert(NM_PPTP_KEY_NO_VJ_COMP, yesString);
        }
        // Echo
        if (d->advUi.cb_sendEcho->isChecked()) {
            data.insert(NM_PPTP_KEY_LCP_ECHO_FAILURE, "5");
            data.insert(NM_PPTP_KEY_LCP_ECHO_INTERVAL, "30");
        }
        //reset save advanced flag
        d->advancedIsDirty = false;
    }

    // save it all
    d->setting->setData(data);
    d->setting->setVpnSecrets(secretData);
}

void PptpSettingWidget::readSecrets()
{
    Q_D(PptpSettingWidget);
    QVariantMap secrets = d->setting->vpnSecrets();

    d->ui.edt_password->setText(secrets.value(QLatin1String(NM_PPTP_KEY_PASSWORD)).toString());
}

void PptpSettingWidget::validate()
{

}

// vim: sw=4 sts=4 et tw=100
