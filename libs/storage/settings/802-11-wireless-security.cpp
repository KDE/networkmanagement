// This file is generated by kconfig_compiler from 802-11-wireless-security.kcfg.
// All changes you do to this file will be lost.

#include "802-11-wireless-security.h"

using namespace Knm;

WirelessSecuritySetting::WirelessSecuritySetting() : Setting(Setting::WirelessSecurity)
                                                     , mSecurityType(WirelessSecuritySetting::EnumSecurityType::None), mKeymgmt(0), mWeptxkeyindex(0), mAuthalg(0)
{
}

WirelessSecuritySetting::~WirelessSecuritySetting()
{
}

QString WirelessSecuritySetting::name() const
{
  return QLatin1String("802-11-wireless-security");
}
bool WirelessSecuritySetting::hasSecrets() const
{
  if (mSecurityType == EnumSecurityType::None || mSecurityType == EnumSecurityType::WPAEAP)
    return false;
  return true;
}
