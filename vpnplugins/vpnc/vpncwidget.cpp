/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

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

#include "vpncwidget.h"
#include "ui_vpncprop.h"

#include <QString>
#include "nm-vpnc-service.h"

#include "connection.h"

class VpncSettingWidgetPrivate
{
public:
    Ui_VpncProp ui;
    Knm::VpnSetting * setting;
    uint dpdTimeout;

    class EnumPasswordStorage
    {
    public:
        enum PasswordStorage {AlwaysAsk = 0, Save, NotRequired};
    };
    class EnumEncryptionMethod
    {
    public:
        enum EncryptionMethod {Secure = 0, Weak, None};
    };
    class EnumNatt
    {
    public:
        enum NatT {NattIfAvail = 0, Force, CiscoUdp, Disabled};
    };
    class EnumDh
    {
    public:
        enum Dh {Dh1 = 0, Dh2, Dh5};
    };
    class EnumPerfectForwardSecrecy
    {
    public:
        enum PFS {NoPfs = 0, Server, Dh1, Dh2, Dh5};
    };
    class EnumVendor
    {
    public:
        enum Vendor {Cisco = 0, Netscreen};
    };
};

VpncSettingWidget::VpncSettingWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(connection, parent), d_ptr(new VpncSettingWidgetPrivate)
{
    Q_D(VpncSettingWidget);
    setValid(false);
    d->dpdTimeout = 0;
    d->ui.setupUi(this);
    d->setting = static_cast<Knm::VpnSetting *>(connection->setting(Knm::Setting::Vpn));
    connect(d->ui.cboNatTraversal, SIGNAL(currentIndexChanged(int)), this, SLOT(natTraversalChanged(int)));
    connect(d->ui.cboUserPassOptions, SIGNAL(currentIndexChanged(int)), this, SLOT(userPasswordTypeChanged(int)));
    connect(d->ui.cboGroupPassOptions, SIGNAL(currentIndexChanged(int)), this, SLOT(groupPasswordTypeChanged(int)));
    connect(d->ui.cbShowPasswords, SIGNAL(toggled(bool)), this, SLOT(showPasswordsChanged(bool)));
    connect(d->ui.leUserName, SIGNAL(textChanged(QString)), SLOT(validate()));
    connect(d->ui.leGroupName, SIGNAL(textChanged(QString)), SLOT(validate()));
}

VpncSettingWidget::~VpncSettingWidget()
{
    delete d_ptr;
}

void VpncSettingWidget::userPasswordTypeChanged(int index)
{
    Q_D(VpncSettingWidget);
    d->ui.leUserPassword->setEnabled(index == VpncSettingWidgetPrivate::EnumPasswordStorage::Save);
    validate();
}

void VpncSettingWidget::groupPasswordTypeChanged(int index)
{
    Q_D(VpncSettingWidget);
    d->ui.leGroupPassword->setEnabled(index == VpncSettingWidgetPrivate::EnumPasswordStorage::Save);
    validate();
}

void VpncSettingWidget::natTraversalChanged(int index)
{
    Q_D(VpncSettingWidget);
    if (index == VpncSettingWidgetPrivate::EnumNatt::CiscoUdp) {
        d->ui.spbCiscoUdpEncPort->setEnabled(true);
    } else {
        d->ui.spbCiscoUdpEncPort->setEnabled(false);
    }
}

void VpncSettingWidget::readConfig()
{
    Q_D(VpncSettingWidget);
    // General settings
    QStringMap dataMap = d->setting->data();
    //   gateway
    QString gateway = dataMap[NM_VPNC_KEY_GATEWAY];
    if (!gateway.isEmpty()) {
        d->ui.leGateway->setText(gateway);
    }
    //   username
    QString user = dataMap[NM_VPNC_KEY_XAUTH_USER];
    if (!user.isEmpty()) {
        d->ui.leUserName->setText(user);
    }
    //   group name
    QString group = dataMap[NM_VPNC_KEY_ID];
    if (!group.isEmpty()) {
        d->ui.leGroupName->setText(group);
    }
    // hybrid auth
    if (dataMap[NM_VPNC_KEY_AUTHMODE] == QLatin1String("hybrid")) {
        d->ui.cbUseHybridAuth->setChecked(true);
        d->ui.leCaCertPath->setUrl(KUrl(dataMap[NM_VPNC_KEY_CA_FILE]));
    }
    // password storage type is set in readSecrets

    // Optional settings
    //   domain
    QString domain = dataMap[NM_VPNC_KEY_DOMAIN];
    if (!domain.isEmpty()) {
        d->ui.leDomain->setText(domain);
    }

    //   encryption
    if (dataMap[NM_VPNC_KEY_SINGLE_DES] == QLatin1String("yes")) {
        d->ui.cboEncryptionMethod->setCurrentIndex(VpncSettingWidgetPrivate::EnumEncryptionMethod::Weak);
    } else if (dataMap[NM_VPNC_KEY_NO_ENCRYPTION] == QLatin1String("yes")) {
        d->ui.cboEncryptionMethod->setCurrentIndex(VpncSettingWidgetPrivate::EnumEncryptionMethod::None);
    }

    //   nat traversal
    if (dataMap[NM_VPNC_KEY_NAT_TRAVERSAL_MODE] == NM_VPNC_NATT_MODE_NATT)
        d->ui.cboNatTraversal->setCurrentIndex(VpncSettingWidgetPrivate::EnumNatt::NattIfAvail);
    else if (dataMap[NM_VPNC_KEY_NAT_TRAVERSAL_MODE] == NM_VPNC_NATT_MODE_NATT_ALWAYS)
        d->ui.cboNatTraversal->setCurrentIndex(VpncSettingWidgetPrivate::EnumNatt::Force);
    else if (dataMap[NM_VPNC_KEY_NAT_TRAVERSAL_MODE] == NM_VPNC_NATT_MODE_CISCO)
        d->ui.cboNatTraversal->setCurrentIndex(VpncSettingWidgetPrivate::EnumNatt::CiscoUdp);
    else if (dataMap[NM_VPNC_KEY_NAT_TRAVERSAL_MODE] == NM_VPNC_NATT_MODE_NONE)
        d->ui.cboNatTraversal->setCurrentIndex(VpncSettingWidgetPrivate::EnumNatt::Disabled);

    //   dead peer detection
    if (dataMap.contains(NM_VPNC_KEY_DPD_IDLE_TIMEOUT)) {
        uint dpdTimeout = dataMap.value(NM_VPNC_KEY_DPD_IDLE_TIMEOUT).toUInt();
        if (dpdTimeout == 0) {
            d->ui.chkDeadPeerDetection->setChecked(false);
            d->dpdTimeout = dpdTimeout;
        }
    }
    //   dh group
    if (dataMap.contains(NM_VPNC_KEY_DHGROUP)) {
        QString dhGroup = dataMap.value(NM_VPNC_KEY_DHGROUP);
        if (dhGroup == NM_VPNC_DHGROUP_DH1) {
            // DH Group 1
            d->ui.cboDHGroup->setCurrentIndex(VpncSettingWidgetPrivate::EnumDh::Dh1);
        } else if (dhGroup == NM_VPNC_DHGROUP_DH2) {
            // DH Group 2
            d->ui.cboDHGroup->setCurrentIndex(VpncSettingWidgetPrivate::EnumDh::Dh2);
        } else if (dhGroup == NM_VPNC_DHGROUP_DH5) {
            // DH Group 5
            d->ui.cboDHGroup->setCurrentIndex(VpncSettingWidgetPrivate::EnumDh::Dh5);
        }
    }

    // Perfect Forward Secrecy
    if (dataMap.contains(NM_VPNC_KEY_PERFECT_FORWARD)) {
        QString forwardSecrecy = dataMap.value(NM_VPNC_KEY_PERFECT_FORWARD);
        if (forwardSecrecy == QLatin1String("nopfs")) {
            d->ui.cboPerfectForwardSecrecy->setCurrentIndex(VpncSettingWidgetPrivate::EnumPerfectForwardSecrecy::NoPfs);
        } else if (forwardSecrecy == QLatin1String("server")) {
            d->ui.cboPerfectForwardSecrecy->setCurrentIndex(VpncSettingWidgetPrivate::EnumPerfectForwardSecrecy::Server);
        } else if (forwardSecrecy == QLatin1String("dh1")) {
            d->ui.cboPerfectForwardSecrecy->setCurrentIndex(VpncSettingWidgetPrivate::EnumPerfectForwardSecrecy::Dh1);
        } else if (forwardSecrecy == QLatin1String("dh2")) {
            d->ui.cboPerfectForwardSecrecy->setCurrentIndex(VpncSettingWidgetPrivate::EnumPerfectForwardSecrecy::Dh2);
        } else if (forwardSecrecy == QLatin1String("dh5")) {
            d->ui.cboPerfectForwardSecrecy->setCurrentIndex(VpncSettingWidgetPrivate::EnumPerfectForwardSecrecy::Dh5);
        }
    }

    // Vendor
    if (dataMap.contains(NM_VPNC_KEY_VENDOR)) {
        QString vendor = dataMap.value(NM_VPNC_KEY_VENDOR);
        if (vendor == QLatin1String("cisco")) {
            d->ui.cboVendor->setCurrentIndex(VpncSettingWidgetPrivate::EnumVendor::Cisco);
        } else if (vendor == QLatin1String("netscreen")) {
            d->ui.cboVendor->setCurrentIndex(VpncSettingWidgetPrivate::EnumVendor::Netscreen);
        }
    }

    // Application Version
    if (dataMap.contains(NM_VPNC_KEY_APP_VERSION)) {
        d->ui.leApplicationVersion->setText(dataMap.value(NM_VPNC_KEY_APP_VERSION));
    }

    // Local Port
    if (dataMap.contains(NM_VPNC_KEY_LOCAL_PORT)) {
        d->ui.spbLocalPort->setValue(dataMap.value(NM_VPNC_KEY_LOCAL_PORT).toInt());
    }

    // Cisco UDP Encapsulation Port
    if (dataMap.contains(NM_VPNC_KEY_CISCO_UDP_ENCAPS_PORT)) {
        d->ui.spbCiscoUdpEncPort->setValue(dataMap.value(NM_VPNC_KEY_CISCO_UDP_ENCAPS_PORT).toInt());
    }
}

void VpncSettingWidget::fillOnePasswordCombo(QComboBox * combo, Knm::Setting::secretsTypes type)
{
    if (type.testFlag(Knm::Setting::AgentOwned) || type.testFlag(Knm::Setting::None)) {
        combo->setCurrentIndex(VpncSettingWidgetPrivate::EnumPasswordStorage::Save);
    } else if (type.testFlag(Knm::Setting::NotRequired)) {
        combo->setCurrentIndex(VpncSettingWidgetPrivate::EnumPasswordStorage::NotRequired);
    } else if (type.testFlag(Knm::Setting::NotSaved)) {
        combo->setCurrentIndex(VpncSettingWidgetPrivate::EnumPasswordStorage::AlwaysAsk);
    }
}

void VpncSettingWidget::writeConfig()
{
    Q_D(VpncSettingWidget);
    kDebug();

    d->setting->setServiceType(QLatin1String(NM_DBUS_SERVICE_VPNC));

    QStringMap data;
    QStringMap secretData;

    // General settings
    //   gateway
    if (!d->ui.leGateway->text().isEmpty()) {
        data.insert(NM_VPNC_KEY_GATEWAY, d->ui.leGateway->text());
    }

    //   group name
    if (!d->ui.leGroupName->text().isEmpty()) {
        data.insert(NM_VPNC_KEY_ID, d->ui.leGroupName->text());
    }

    //   user password
    if (!d->ui.leUserPassword->text().isEmpty() && d->ui.cboUserPassOptions->currentIndex() == VpncSettingWidgetPrivate::EnumPasswordStorage::Save) {
        secretData.insert(NM_VPNC_KEY_XAUTH_PASSWORD, d->ui.leUserPassword->text());
    }
    //   group password
    if (!d->ui.leGroupPassword->text().isEmpty() && d->ui.cboGroupPassOptions->currentIndex() == VpncSettingWidgetPrivate::EnumPasswordStorage::Save) {
        secretData.insert(NM_VPNC_KEY_SECRET, d->ui.leGroupPassword->text());
    }
    handleOnePasswordType(d->ui.cboUserPassOptions, NM_VPNC_KEY_XAUTH_PASSWORD"-flags", data);
    handleOnePasswordType(d->ui.cboGroupPassOptions, NM_VPNC_KEY_SECRET"-flags", data);

    // hybrid auth
    if (d->ui.cbUseHybridAuth->isChecked()) {
        data.insert(NM_VPNC_KEY_AUTHMODE, QLatin1String("hybrid"));
        data.insert(NM_VPNC_KEY_CA_FILE, d->ui.leCaCertPath->url().path());
    }

    // Optional settings
    //   username
    if (!d->ui.leUserName->text().isEmpty()) {
        data.insert(NM_VPNC_KEY_XAUTH_USER, d->ui.leUserName->text());
    }

    //   domain
    if (!d->ui.leDomain->text().isEmpty()) {
        data.insert(NM_VPNC_KEY_DOMAIN, d->ui.leDomain->text());
    }

    //   encryption
    switch (d->ui.cboEncryptionMethod->currentIndex()) {
        case VpncSettingWidgetPrivate::EnumEncryptionMethod::Weak:
            data.insert(NM_VPNC_KEY_SINGLE_DES, QLatin1String("yes"));
            break;
        case VpncSettingWidgetPrivate::EnumEncryptionMethod::None:
            data.insert(NM_VPNC_KEY_NO_ENCRYPTION, QLatin1String("yes"));
            break;
        default:
            break;
    }

    // nat traversal
    switch (d->ui.cboNatTraversal->currentIndex()) {
        case VpncSettingWidgetPrivate::EnumNatt::Force:
        data.insert(NM_VPNC_KEY_NAT_TRAVERSAL_MODE, QLatin1String(NM_VPNC_NATT_MODE_NATT_ALWAYS));
        break;
        case VpncSettingWidgetPrivate::EnumNatt::CiscoUdp:
        data.insert(NM_VPNC_KEY_NAT_TRAVERSAL_MODE, QLatin1String(NM_VPNC_NATT_MODE_CISCO));
        break;
        case VpncSettingWidgetPrivate::EnumNatt::Disabled:
        data.insert(NM_VPNC_KEY_NAT_TRAVERSAL_MODE, QLatin1String(NM_VPNC_NATT_MODE_NONE));
        break;
        case VpncSettingWidgetPrivate::EnumNatt::NattIfAvail:
    default:
        data.insert(NM_VPNC_KEY_NAT_TRAVERSAL_MODE, QLatin1String(NM_VPNC_NATT_MODE_NATT));
        break;
    }

    // dead peer detection
    if (d->ui.chkDeadPeerDetection->isChecked()) {
        if (d->dpdTimeout > 0) {
            data.insert(NM_VPNC_KEY_DPD_IDLE_TIMEOUT, QString::number(d->dpdTimeout));
        }
    } else {
        data.insert(NM_VPNC_KEY_DPD_IDLE_TIMEOUT, QString::number(0));
    }

    // dh group
    switch (d->ui.cboDHGroup->currentIndex()) {
    case VpncSettingWidgetPrivate::EnumDh::Dh1:	// DH Group 1
        data.insert(NM_VPNC_KEY_DHGROUP, NM_VPNC_DHGROUP_DH1);
        break;
    case VpncSettingWidgetPrivate::EnumDh::Dh2:	// DH Group 2
        data.insert(NM_VPNC_KEY_DHGROUP, NM_VPNC_DHGROUP_DH2);
        break;
    case VpncSettingWidgetPrivate::EnumDh::Dh5:	// DH Group 5
        data.insert(NM_VPNC_KEY_DHGROUP, NM_VPNC_DHGROUP_DH5);
        break;
    }

    // Perfect Forward Secrecy
    switch (d->ui.cboPerfectForwardSecrecy->currentIndex()) {
        case VpncSettingWidgetPrivate::EnumPerfectForwardSecrecy::NoPfs:
            data.insert(NM_VPNC_KEY_PERFECT_FORWARD, QLatin1String("nopfs"));
            break;
        case VpncSettingWidgetPrivate::EnumPerfectForwardSecrecy::Server:
            data.insert(NM_VPNC_KEY_PERFECT_FORWARD, QLatin1String("server"));
            break;
        case VpncSettingWidgetPrivate::EnumPerfectForwardSecrecy::Dh1:
            data.insert(NM_VPNC_KEY_PERFECT_FORWARD, QLatin1String("dh1"));
            break;
        case VpncSettingWidgetPrivate::EnumPerfectForwardSecrecy::Dh2:
            data.insert(NM_VPNC_KEY_PERFECT_FORWARD, QLatin1String("dh2"));
            break;
        case VpncSettingWidgetPrivate::EnumPerfectForwardSecrecy::Dh5:
            data.insert(NM_VPNC_KEY_PERFECT_FORWARD, QLatin1String("dh5"));
            break;
    }

    // Vendor
    switch (d->ui.cboVendor->currentIndex()) {
        case VpncSettingWidgetPrivate::EnumVendor::Cisco:
            data.insert(NM_VPNC_KEY_VENDOR, QLatin1String("cisco"));
            break;
        case VpncSettingWidgetPrivate::EnumVendor::Netscreen:
            data.insert(NM_VPNC_KEY_VENDOR, QLatin1String("netscreen"));
            break;
    }

    // Application Version
    if (!d->ui.leApplicationVersion->text().isEmpty()) {
        data.insert(NM_VPNC_KEY_APP_VERSION, d->ui.leApplicationVersion->text());
    }

    // Local Port
    data.insert(NM_VPNC_KEY_LOCAL_PORT, QString::number(d->ui.spbLocalPort->value()));

    // Cisco UDP Encapsulation Port
    data.insert(NM_VPNC_KEY_CISCO_UDP_ENCAPS_PORT, QString::number(d->ui.spbCiscoUdpEncPort->value()));

    d->setting->setData(data);
    d->setting->setVpnSecrets(secretData);
}

uint VpncSettingWidget::handleOnePasswordType(const QComboBox * combo, const QString & key, QStringMap & data)
{
    uint type = combo->currentIndex();
    switch (type) {
        case VpncSettingWidgetPrivate::EnumPasswordStorage::AlwaysAsk:
            data.insert(key, QString::number(Knm::Setting::NotSaved));
            break;
        case VpncSettingWidgetPrivate::EnumPasswordStorage::Save:
            data.insert(key, QString::number(Knm::Setting::AgentOwned));
            break;
        case VpncSettingWidgetPrivate::EnumPasswordStorage::NotRequired:
            data.insert(key, QString::number(Knm::Setting::NotRequired));
            break;
    }
    return type;
}

void VpncSettingWidget::readSecrets()
{
    Q_D(VpncSettingWidget);
    QStringMap secrets = d->setting->vpnSecrets();
    Knm::Setting::secretsTypes userType;
    Knm::Setting::secretsTypes groupType;

    userType = (Knm::Setting::secretsTypes)d->setting->data().value(NM_VPNC_KEY_XAUTH_PASSWORD"-flags").toInt();

    if (userType & Knm::Setting::AgentOwned || userType & Knm::Setting::None || !userType) {
        d->ui.leUserPassword->setText(secrets.value(QLatin1String(NM_VPNC_KEY_XAUTH_PASSWORD)));
    }
    fillOnePasswordCombo(d->ui.cboUserPassOptions, userType);

    groupType = (Knm::Setting::secretsTypes)d->setting->data().value(NM_VPNC_KEY_SECRET"-flags").toInt();

    if (groupType & Knm::Setting::AgentOwned || groupType & Knm::Setting::None || !groupType) {
        d->ui.leGroupPassword->setText(secrets.value(QLatin1String(NM_VPNC_KEY_SECRET)));
    }
    fillOnePasswordCombo(d->ui.cboGroupPassOptions, groupType);
}

void VpncSettingWidget::validate()
{
    Q_D(VpncSettingWidget);
    // vpnc-0.5.3 refuses to connect without username (Xauth) and group (IPSec) options.
    bool v = !d->ui.leUserName->text().isEmpty() &&
             (d->ui.cboUserPassOptions->currentIndex() != VpncSettingWidgetPrivate::EnumPasswordStorage::NotRequired) &&
             !d->ui.leGroupName->text().isEmpty() &&
             (d->ui.cboGroupPassOptions->currentIndex() != VpncSettingWidgetPrivate::EnumPasswordStorage::NotRequired);
    setValid(v);
    emit valid(v);
}

void VpncSettingWidget::showPasswordsChanged(bool show)
{
    Q_D(VpncSettingWidget);
    d->ui.leUserPassword->setPasswordMode(!show);
    d->ui.leGroupPassword->setPasswordMode(!show);
}

// vim: sw=4 sts=4 et tw=100
