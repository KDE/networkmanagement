// This file is generated by kconfig_compiler from gsm.kcfg.
// All changes you do to this file will be lost.

#include "gsm.h"

using namespace Knm;

GsmSetting::GsmSetting() : Setting(Setting::Gsm), mNetworktype(-1), mBand(-1), mHomeonly(false), mPasswordflags(Setting::AgentOwned), mPinflags(NotSaved)
{
}

GsmSetting::GsmSetting(GsmSetting *setting) : Setting(setting)
{
    setNumber(setting->number());
    setUsername(setting->username());
    setPassword(setting->password());
    setPasswordflags(setting->passwordflags());
    setApn(setting->apn());
    setNetworkid(setting->networkid());
    setNetworktype(setting->networktype());
    setBand(setting->band());
    setHomeonly(setting->homeonly());
    setPin(setting->pin());
    setPinflags(setting->pinflags());
}

GsmSetting::~GsmSetting()
{
}

QString GsmSetting::name() const
{
  return QLatin1String("gsm");
}

QMap<QString,QString> GsmSetting::secretsToMap() const
{
    QMap<QString,QString> map;
    if (passwordflags().testFlag(Setting::AgentOwned)) {
        map.insert(QLatin1String("password"), password());
    }
    if (pinflags().testFlag(Setting::AgentOwned)) {
        map.insert(QLatin1String("pin"), pin());
    }
    return map;
}

void GsmSetting::secretsFromMap(QMap<QString,QString> secrets)
{
    setPassword(secrets.value("password"));
    setPin(secrets.value("pin"));
}

QStringList GsmSetting::needSecrets(const bool requestNew) const
{
    QStringList list;
    if ((password().isEmpty() || requestNew) && !passwordflags().testFlag(Setting::NotRequired))
        list.append("password");
    if ((pin().isEmpty() || requestNew) && !pinflags().testFlag(Setting::NotRequired))
        list.append("pin");
    return list;
}

bool GsmSetting::hasPersistentSecrets() const
{
    if (passwordflags().testFlag(Setting::None) || passwordflags().testFlag(Setting::AgentOwned))
        return true;
    if (pinflags().testFlag(Setting::None) || pinflags().testFlag(Setting::AgentOwned))
        return true;
    return false;
}
