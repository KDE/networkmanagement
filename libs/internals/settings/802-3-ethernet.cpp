// This file is generated by kconfig_compiler from 802-3-ethernet.kcfg.
// All changes you do to this file will be lost.

#include "802-3-ethernet.h"

using namespace Knm;

WiredSetting::WiredSetting() : Setting(Setting::Wired),
    mPort(WiredSetting::EnumPort::mii),
    mSpeed(0),
    mDuplex(WiredSetting::EnumDuplex::full),
    mAutonegotiate(true),
    mMtu(0)
{
}

WiredSetting::WiredSetting(WiredSetting *setting) : Setting(setting)
{
    setPort(setting->port());
    setSpeed(setting->speed());
    setDuplex(setting->duplex());
    setAutonegotiate(setting->autonegotiate());
    setMacaddress(setting->macaddress());
    setMtu(setting->mtu());
}

WiredSetting::~WiredSetting()
{
}

QString WiredSetting::name() const
{
  return QLatin1String("802-3-ethernet");
}
bool WiredSetting::hasSecrets() const
{
  return false;
}
