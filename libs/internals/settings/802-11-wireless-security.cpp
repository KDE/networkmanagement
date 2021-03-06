// This file is generated by kconfig_compiler from 802-11-wireless-security.kcfg.
// All changes you do to this file will be lost.

#include "802-11-wireless-security.h"

using namespace Knm;

WirelessSecuritySetting::WirelessSecuritySetting()
    : Setting(Setting::WirelessSecurity), mSecurityType(WirelessSecuritySetting::EnumSecurityType::None), mKeymgmt(0),
    mWeptxkeyindex(0), mAuthalg(0), mWepKeyType(None), mWepkeyflags(Setting::AgentOwned), mPskflags(Setting::AgentOwned), mLeappasswordflags(Setting::AgentOwned)
{
}

WirelessSecuritySetting::WirelessSecuritySetting(WirelessSecuritySetting *setting) : Setting(setting)
{
    setSecurityType(setting->securityType());
    setKeymgmt(setting->keymgmt());
    setWeptxkeyindex(setting->weptxkeyindex());
    setAuthalg(setting->authalg());
    setProto(setting->proto());
    setPairwise(setting->pairwise());
    setGroup(setting->group());
    setLeapusername(setting->leapusername());
    setWepkey0(setting->wepkey0());
    setWepkey1(setting->wepkey1());
    setWepkey2(setting->wepkey2());
    setWepkey3(setting->wepkey3());
    setWepkeyflags(setting->wepkeyflags());
    setPsk(setting->psk());
    setPskflags(setting->pskflags());
    setLeappassword(setting->leappassword());
    setLeappasswordflags(setting->leappasswordflags());
    setWepKeyType(setting->wepKeyType());
}

WirelessSecuritySetting::~WirelessSecuritySetting()
{
}

QString WirelessSecuritySetting::name() const
{
  return QLatin1String("802-11-wireless-security");
}

void WirelessSecuritySetting::reset()
{
    m_initialized = false;
    mSecurityType = EnumSecurityType::None;
    mKeymgmt = EnumKeymgmt::None;
    mWeptxkeyindex = 0;
    mAuthalg = EnumAuthalg::none;
    mProto = QStringList();
    mPairwise = QStringList();
    mGroup = QStringList();
    mLeapusername = QString();
    mWepkey0 = QString();
    mWepkey1 = QString();
    mWepkey2 = QString();
    mWepkey3 = QString();
    mPsk = QString();
    mLeappassword = QString();
    mWeppassphrase = QString();
    mWepKeyType = None;
}

QMap<QString,QString> WirelessSecuritySetting::secretsToMap() const
{
    QMap<QString,QString> map;
    if (wepkeyflags().testFlag(Setting::AgentOwned)) {
        map.insert(QLatin1String("wepkey0"), wepkey0());
        map.insert(QLatin1String("wepkey1"), wepkey1());
        map.insert(QLatin1String("wepkey2"), wepkey2());
        map.insert(QLatin1String("wepkey3"), wepkey3());
    }
    if (pskflags().testFlag(Setting::AgentOwned)) {
        map.insert(QLatin1String("psk"), psk());
    }
    if (leappasswordflags().testFlag(Setting::AgentOwned)) {
        map.insert(QLatin1String("leappassword"), leappassword());
    }
    return map;
}

void WirelessSecuritySetting::secretsFromMap(QMap<QString,QString> secrets)
{
    bool secretsAvailable = false;
    if (secrets.contains("wepkey0")) {
        setWepkey0(secrets.value("wepkey0"));
        secretsAvailable = true;
    }
    if (secrets.contains("wepkey1")) {
        setWepkey1(secrets.value("wepkey1"));
        secretsAvailable = true;
    }
    if (secrets.contains("wepkey2")) {
        setWepkey2(secrets.value("wepkey2"));
        secretsAvailable = true;
    }
    if (secrets.contains("wepkey3")) {
        setWepkey3(secrets.value("wepkey3"));
        secretsAvailable = true;
    }
    if (secrets.contains("psk")) {
        setPsk(secrets.value("psk"));
        secretsAvailable = true;
    }
    if (secrets.contains("leappassword")) {
        setLeappassword(secrets.value("leappassword"));
        secretsAvailable = true;
    }
    setSecretsAvailable(secretsAvailable);
}

QStringList WirelessSecuritySetting::needSecrets(const bool requestNew) const
{
    QStringList list;
    switch (securityType())
    {
        case WirelessSecuritySetting::EnumSecurityType::None:
        case WirelessSecuritySetting::EnumSecurityType::DynamicWep:
            break;
        case WirelessSecuritySetting::EnumSecurityType::StaticWep:
            if (!wepkeyflags().testFlag(Setting::NotRequired)) {
                switch (weptxkeyindex())
                {
                    case 0:
                        if ((wepkey0().isEmpty() || requestNew))
                            list.append("wepkey0");
                        break;
                    case 1:
                        if ((wepkey1().isEmpty() || requestNew))
                            list.append("wepkey1");
                        break;
                    case 2:
                        if ((wepkey2().isEmpty() || requestNew))
                            list.append("wepkey2");
                        break;
                    case 3:
                        if ((wepkey3().isEmpty() || requestNew))
                            list.append("wepkey3");
                        break;
                }
            }

            break;
        case WirelessSecuritySetting::EnumSecurityType::WpaPsk:
        case WirelessSecuritySetting::EnumSecurityType::Wpa2Psk:
            if ((psk().isEmpty() || requestNew) && !pskflags().testFlag(Setting::NotRequired))
                list.append("psk");
            break;
        case WirelessSecuritySetting::EnumSecurityType::Leap:
            if ((leappassword().isEmpty() || requestNew) && !leappasswordflags().testFlag(Setting::NotRequired))
                list.append("leappassword");
            break;
    }
    return list;
}

bool WirelessSecuritySetting::hasPersistentSecrets() const
{
    bool hasSecrets = false;
    switch (securityType())
    {
        case WirelessSecuritySetting::EnumSecurityType::None:
        case WirelessSecuritySetting::EnumSecurityType::DynamicWep:
            break;
        case WirelessSecuritySetting::EnumSecurityType::StaticWep:
            if (wepkeyflags().testFlag(Setting::None) || wepkeyflags().testFlag(Setting::AgentOwned))
                hasSecrets = true;
            break;
        case WirelessSecuritySetting::EnumSecurityType::WpaPsk:
        case WirelessSecuritySetting::EnumSecurityType::Wpa2Psk:
            if (pskflags().testFlag(Setting::None) || pskflags().testFlag(Setting::AgentOwned))
                hasSecrets = true;
            break;
        case WirelessSecuritySetting::EnumSecurityType::Leap:
            if (leappasswordflags().testFlag(Setting::None) || leappasswordflags().testFlag(Setting::AgentOwned))
                hasSecrets = true;
            break;
    }
    return hasSecrets;
}
