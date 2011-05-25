// This file is generated by kconfig_compiler from 802-11-wireless.kcfg.
// All changes you do to this file will be lost.

#include "802-11-wireless.h"

using namespace Knm;

WirelessSetting::WirelessSetting() : Setting(Setting::Wireless),
    mMode(0), mBand(WirelessSetting::EnumBand::bg), mChannel(0), mRate(0), mTxpower(0), mMtu(0)
{
}

WirelessSetting::WirelessSetting(WirelessSetting *setting) : Setting(setting)
{
    setSsid(setting->ssid());
    setMode(setting->mode());
    setBand(setting->band());
    setChannel(setting->channel());
    setBssid(setting->bssid());
    setRate(setting->rate());
    setTxpower(setting->txpower());
    setMacaddress(setting->macaddress());
    setMtu(setting->mtu());
    setSeenbssids(setting->seenbssids());
    setSecurity(setting->security());
}

WirelessSetting::~WirelessSetting()
{
}

QString WirelessSetting::name() const
{
  return QLatin1String("802-11-wireless");
}
bool WirelessSetting::hasSecrets() const
{
  return false;
}
