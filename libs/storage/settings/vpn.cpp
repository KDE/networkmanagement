// This file is generated by kconfig_compiler from vpn.kcfg.
// All changes you do to this file will be lost.

#include "vpn.h"

using namespace KnmInternals;

VpnSetting::VpnSetting() : Setting(Setting::Vpn)
{
}

VpnSetting::~VpnSetting()
{
}

QString VpnSetting::name() const
{
  return QLatin1String("vpn");
}
bool VpnSetting::hasSecrets() const
{
  return true;
}
