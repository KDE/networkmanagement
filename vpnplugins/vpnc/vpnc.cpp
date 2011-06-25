/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>
Copyright 2011 Rajeesh K Nambiar <rajeeshknambiar@gmail.com>

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

#include "vpnc.h"

#include <KPluginFactory>
#include <KSharedConfig>
#include <KStandardDirs>
#include <KMessageBox>
#include "nm-vpnc-service.h"

#include "vpncwidget.h"
#include "vpncauth.h"
#include "connection.h"
#include "settings/vpnpersistence.h"

#define NM_VPNC_LOCAL_PORT_DEFAULT 500

VpncUiPluginPrivate::VpncUiPluginPrivate()
{
    decryptedPasswd.clear();
    ciscoDecrypt = 0;
}
VpncUiPluginPrivate::~VpncUiPluginPrivate()
{
    if (ciscoDecrypt)
        delete ciscoDecrypt;
}

void VpncUiPluginPrivate::gotciscoDecryptOutput()
{
    QByteArray output = ciscoDecrypt->readAll();
    if (!output.isEmpty()) {
        QList<QByteArray> lines = output.split('\n');
        if (!lines.isEmpty()) {
            decryptedPasswd = QString(lines.first());
        }
    }
}

void VpncUiPluginPrivate::ciscoDecryptFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode || exitStatus != QProcess::NormalExit)
        decryptedPasswd.clear();
    delete ciscoDecrypt;
    ciscoDecrypt = 0;
}

void VpncUiPluginPrivate::ciscoDecryptError(QProcess::ProcessError pError)
{
    if (!pError) {
        kDebug() << "Error in executing cisco-decrypt";
        KMessageBox::error(0, i18n("Error decrypting the obfuscated password"), i18n("Error"), KMessageBox::Notify);
    }
    decryptedPasswd.clear();
}

K_PLUGIN_FACTORY( VpncUiPluginFactory, registerPlugin<VpncUiPlugin>(); )
K_EXPORT_PLUGIN( VpncUiPluginFactory( "networkmanagement_vpncui", "libknetworkmanager" ) )

VpncUiPlugin::VpncUiPlugin(QObject * parent, const QVariantList &) : VpnUiPlugin(parent)
{

}

VpncUiPlugin::~VpncUiPlugin()
{

}

SettingWidget * VpncUiPlugin::widget(Knm::Connection * connection, QWidget * parent)
{
    return new VpncSettingWidget(connection, parent);
}

SettingWidget * VpncUiPlugin::askUser(Knm::Connection * connection, QWidget * parent)
{
    return new VpncAuthWidget(connection, parent);
}

QString VpncUiPlugin::suggestedFileName(Knm::Connection *connection) const
{
    return connection->name() + ".pcf";
}

QVariantList VpncUiPlugin::importConnectionSettings(const QString &fileName)
{
    kDebug() << "Importing Cisco VPN connection from " << fileName;

    VpncUiPluginPrivate * decrPlugin = 0;
    QVariantList conSetting;

    // NOTE: Cisco VPN pcf files follow ini style matching KConfig files
    // http://www.cisco.com/en/US/docs/security/vpn_client/cisco_vpn_client/vpn_client46/administration/guide/vcAch2.html#wp1155033
    KSharedConfig::Ptr config = KSharedConfig::openConfig(fileName);
    if (!config)
        return conSetting;

    KConfigGroup cg(config, "main");   // Keys&Values are stored under [main]
    if (cg.exists()) {
        // Setup cisco-decrypt binary to decrypt the passwords
        QStringList decrArgs;
        QString ciscoDecryptBinary = KStandardDirs::findExe("cisco-decrypt");

        decrPlugin = new VpncUiPluginPrivate();
        decrPlugin->ciscoDecrypt = new KProcess(this);
        decrPlugin->ciscoDecrypt->setOutputChannelMode(KProcess::OnlyStdoutChannel);
        decrPlugin->ciscoDecrypt->setReadChannel(QProcess::StandardOutput);
        connect(decrPlugin->ciscoDecrypt, SIGNAL(error(QProcess::ProcessError)), decrPlugin, SLOT(ciscoDecryptError(QProcess::ProcessError)));
        connect(decrPlugin->ciscoDecrypt, SIGNAL(finished(int,QProcess::ExitStatus)), decrPlugin, SLOT(ciscoDecryptFinished(int,QProcess::ExitStatus)));
        connect(decrPlugin->ciscoDecrypt, SIGNAL(readyReadStandardOutput()), decrPlugin, SLOT(gotciscoDecryptOutput()));

        QStringMap data;
        QVariantMap secretData;
        QStringMap secretsType;

        // gateway
        data.insert(NM_VPNC_KEY_GATEWAY, cg.readEntry("Host"));
        // group name
        data.insert(NM_VPNC_KEY_ID, cg.readEntry("GroupName"));
        // user password
        if (!cg.readEntry("UserPassword").isEmpty()) {
            secretData.insert(NM_VPNC_KEY_XAUTH_PASSWORD, cg.readEntry("UserPassword"));
        }
        else if (!cg.readEntry("enc_UserPassword").isEmpty() && !ciscoDecryptBinary.isEmpty()) {
            // Decrypt the password and insert into map
            decrArgs.clear();
            decrArgs << cg.readEntry("enc_UserPassword");
            decrPlugin->ciscoDecrypt->setProgram(ciscoDecryptBinary, decrArgs);
            decrPlugin->ciscoDecrypt->start();
            if (decrPlugin->ciscoDecrypt->waitForStarted() && decrPlugin->ciscoDecrypt->waitForFinished()) {
                secretData.insert(NM_VPNC_KEY_XAUTH_PASSWORD, decrPlugin->decryptedPasswd);
            }
        }
        // Save user password
        switch (cg.readEntry("SaveUserPassword").toInt())
        {
            case 0:
                secretsType.insert(NM_VPNC_KEY_XAUTH_PASSWORD, NM_VPN_PW_TYPE_ASK);
                break;
            case 1:
                secretsType.insert(NM_VPNC_KEY_XAUTH_PASSWORD, NM_VPN_PW_TYPE_SAVE);
                break;
            case 2:
                secretsType.insert(NM_VPNC_KEY_XAUTH_PASSWORD, NM_VPN_PW_TYPE_UNUSED);
                break;
        }

        // group password
        if (!cg.readEntry("GroupPwd").isEmpty()) {
            secretData.insert(NM_VPNC_KEY_SECRET, cg.readEntry("GroupPwd"));
            secretsType.insert(NM_VPNC_KEY_SECRET, NM_VPN_PW_TYPE_SAVE);
        }
        else if (!cg.readEntry("enc_GroupPwd").isEmpty() && !ciscoDecryptBinary.isEmpty()) {
            //Decrypt the password and insert into map
            decrArgs.clear();
            decrArgs << cg.readEntry("enc_GroupPwd");
            decrPlugin->ciscoDecrypt->setProgram(ciscoDecryptBinary, decrArgs);
            decrPlugin->ciscoDecrypt->start();
            if (decrPlugin->ciscoDecrypt->waitForStarted() && decrPlugin->ciscoDecrypt->waitForFinished()) {
                secretData.insert(NM_VPNC_KEY_SECRET, decrPlugin->decryptedPasswd);
                secretsType.insert(NM_VPNC_KEY_SECRET, NM_VPN_PW_TYPE_SAVE);
            }
        }
        delete decrPlugin;

        // Optional settings
        // username
        if (!cg.readEntry("Username").isEmpty()) {
            data.insert(NM_VPNC_KEY_XAUTH_USER, cg.readEntry("Username"));
        }
        // domain
        if (!cg.readEntry("NTDomain").isEmpty()) {
            data.insert(NM_VPNC_KEY_DOMAIN, cg.readEntry("NTDomain"));
        }
        // encryption
        if (!cg.readEntry("SingleDES").isEmpty() && cg.readEntry("SingleDES").toInt() != 0) {
            data.insert(NM_VPNC_KEY_SINGLE_DES, QLatin1String("yes"));
        }
        /* Disable all NAT Traversal if explicit EnableNat=0 exists, otherwise
         * default to NAT-T which is newer and standardized.  If EnableNat=1, then
         * use Cisco-UDP like always; but if the key "X-NM-Use-NAT-T" is set, then
         * use NAT-T.  If the key "X-NM-Force-NAT-T" is set then force NAT-T always
         * on.  See vpnc documentation for more information on what the different
         * NAT modes are.
         */
        // enable NAT
        if (cg.readEntry("EnableNat").toInt() == 1) {
            data.insert(NM_VPNC_KEY_NAT_TRAVERSAL_MODE, QLatin1String(NM_VPNC_NATT_MODE_CISCO));
            // NAT traversal
            if (!cg.readEntry("X-NM-Use-NAT-T").isEmpty()) {
                if (cg.readEntry("X-NM-Use-NAT-T").toInt() == 1) {
                    data.insert(NM_VPNC_KEY_NAT_TRAVERSAL_MODE, QLatin1String(NM_VPNC_NATT_MODE_NATT));
                }
                if (cg.readEntry("X-NM-Force-NAT-T").toInt() == 1) {
                    data.insert(NM_VPNC_KEY_NAT_TRAVERSAL_MODE, QLatin1String(NM_VPNC_NATT_MODE_NATT_ALWAYS));
                }
            }
        }
        else {
            data.insert(NM_VPNC_KEY_NAT_TRAVERSAL_MODE, QLatin1String(NM_VPNC_NATT_MODE_NONE));
        }
        // dead peer detection
        data.insert(NM_VPNC_KEY_DPD_IDLE_TIMEOUT, cg.readEntry("PeerTimeout"));
        // UseLegacyIKEPort=0 uses dynamic source IKE port instead of 500.
        if (cg.readEntry("UseLegacyIKEPort").isEmpty() || cg.readEntry("UseLegacyIKEPort").toInt() != 0) {
            data.insert(NM_VPNC_KEY_LOCAL_PORT, QString(NM_VPNC_LOCAL_PORT_DEFAULT));
        }
        // DH Group
        data.insert(NM_VPNC_KEY_DHGROUP, cg.readEntry("DHGroup"));
        // Tunneling Mode - not supported by vpnc
        if (cg.readEntry("TunnelingMode").toInt() == 1) {
            KMessageBox::error(0, i18n("The VPN settings file '%1' specifies that VPN traffic should be tunneled through TCP which is currently not supported in the vpnc software.\n\nThe connection can still be created, with TCP tunneling disabled, however it may not work as expected.").arg(fileName), i18n("Not supported"), KMessageBox::Notify);
        }
        // TODO : EnableLocalLAN and X-NM-Routes are to be added to IPv4Setting

        conSetting << Knm::VpnPersistence::variantMapFromStringList(Knm::VpnPersistence::stringMapToStringList(data));
        conSetting << secretData;
        conSetting << Knm::VpnPersistence::variantMapFromStringList(Knm::VpnPersistence::stringMapToStringList(secretsType));
        conSetting << cg.readEntry("Description");
    }

    return conSetting;
}

void VpncUiPlugin::exportConnectionSettings(Knm::Connection * connection, const QString &fileName)
{
    QStringMap data;
    QVariantMap secretData;
    QStringMap secretsType;
    KSharedConfig::Ptr config = KSharedConfig::openConfig(fileName);
    KConfigGroup cg(config,"main");

    Knm::VpnSetting * vpnSetting = static_cast<Knm::VpnSetting*>(connection->setting(Knm::Setting::Vpn));
    data = vpnSetting->data();
    secretsType = vpnSetting->secretsStorageType();
    secretData = vpnSetting->vpnSecrets();

    cg.writeEntry("Description", connection->name());
    cg.writeEntry("Host", data[NM_VPNC_KEY_GATEWAY]);
    cg.writeEntry("AuthType", "1");
    cg.writeEntry("GroupName", data[NM_VPNC_KEY_ID]);
    //cg.writeEntry("GroupPwd", secretData[NM_VPNC_KEY_SECRET]);
    //cg.writeEntry("UserPassword", secretData[NM_VPNC_KEY_XAUTH_PASSWORD]);
    cg.writeEntry("GroupPwd", "");
    cg.writeEntry("UserPassword", "");
    cg.writeEntry("enc_GroupPwd", "");
    cg.writeEntry("enc_UserPassword", "");
    if (secretsType[NM_VPNC_KEY_XAUTH_PASSWORD] == NM_VPN_PW_TYPE_ASK) {
        cg.writeEntry("SaveUserPassword", "0");
    }
    if (secretsType[NM_VPNC_KEY_XAUTH_PASSWORD] == NM_VPN_PW_TYPE_SAVE) {
        cg.writeEntry("SaveUserPassword", "1");
    }
    if (secretsType[NM_VPNC_KEY_XAUTH_PASSWORD] == NM_VPN_PW_TYPE_UNUSED) {
        cg.writeEntry("SaveUserPassword", "2");
    }
    cg.writeEntry("Username", data[NM_VPNC_KEY_XAUTH_USER]);
    cg.writeEntry("EnableISPConnect", "0");
    cg.writeEntry("ISPConnectType", "0");
    cg.writeEntry("ISPConnect", "");
    cg.writeEntry("ISPCommand", "");
    cg.writeEntry("EnableBackup", "0");
    cg.writeEntry("BackupServer", "");
    cg.writeEntry("CertStore", "0");
    cg.writeEntry("CertName", "");
    cg.writeEntry("CertPath", "");
    cg.writeEntry("CertSubjectName", "");
    cg.writeEntry("CertSerialHash", "");
    cg.writeEntry("DHGroup", data[NM_VPNC_KEY_DHGROUP]);
    cg.writeEntry("ForceKeepAlives", "0");
    cg.writeEntry("NTDomain", data[NM_VPNC_KEY_DOMAIN]);
    cg.writeEntry("EnableMSLogon", "0");
    cg.writeEntry("MSLogonType", "0");
    cg.writeEntry("TunnelingMode", "0");
    cg.writeEntry("TcpTunnelingPort", "10000");
    cg.writeEntry("PeerTimeout", data[NM_VPNC_KEY_DPD_IDLE_TIMEOUT]);
    cg.writeEntry("EnableLocalLAN", "1");
    cg.writeEntry("SendCertChain", "0");
    cg.writeEntry("VerifyCertDN", "");
    cg.writeEntry("EnableSplitDNS", "1");
    cg.writeEntry("SPPhonebook", "");
    if (data[NM_VPNC_KEY_SINGLE_DES] == "yes") {
        cg.writeEntry("SingleDES", "1");
    }
    if (data[NM_VPNC_KEY_NAT_TRAVERSAL_MODE] == NM_VPNC_NATT_MODE_CISCO) {
        cg.writeEntry("EnableNat", "1");
    }
    if (data[NM_VPNC_KEY_NAT_TRAVERSAL_MODE] == NM_VPNC_NATT_MODE_NATT) {
        cg.writeEntry("EnableNat", "1");
        cg.writeEntry("X-NM-Use-NAT-T", "1");
    }

    if (data[NM_VPNC_KEY_NAT_TRAVERSAL_MODE] == NM_VPNC_NATT_MODE_NATT_ALWAYS) {
        cg.writeEntry("EnableNat", "1");
        cg.writeEntry("X-NM-Force-NAT-T", "1");
    }
    // TODO : export X-NM-Routes

    return;
}

// vim: sw=4 sts=4 et tw=100
