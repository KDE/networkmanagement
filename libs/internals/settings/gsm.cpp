// This file is generated by kconfig_compiler from gsm.kcfg.
// All changes you do to this file will be lost.

#include "gsm.h"
#include "gsmsecrets.h"

using namespace Knm;

GsmSetting::GsmSetting() : Setting(Setting::Gsm), mNetworktype(-1), mBand(-1), mPinflags(NotSaved)
{
    m_secretsObject = new GsmSecrets(this);
}

GsmSetting::GsmSetting(GsmSetting *setting) : Setting(setting)
{
    m_secretsObject = new GsmSecrets(static_cast<GsmSecrets*>(setting->getSecretsObject()), this);
    setNumber(setting->number());
    setUsername(setting->username());
    setPassword(setting->password());
    setPasswordflags(setting->passwordflags());
    setApn(setting->apn());
    setNetworkid(setting->networkid());
    setNetworktype(setting->networktype());
    setBand(setting->band());
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
bool GsmSetting::hasSecrets() const
{
  return true;
}
void GsmSetting::setSecrets(Setting::secretsTypes types)
{
    if (!mPassword.isEmpty())
        setPasswordflags(types);
}
