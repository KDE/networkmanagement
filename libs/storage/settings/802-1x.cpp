// This file is generated by kconfig_compiler from 802-1x.kcfg.
// All changes you do to this file will be lost.

#include "802-1x.h"

Security8021xSetting::Security8021xSetting() : Setting(Setting::Security8021x)
{
}

Security8021xSetting::~Security8021xSetting()
{
}

QString Security8021xSetting::name() const
{
  return QLatin1String("802-1x");
}
bool Security8021xSetting::hasSecrets() const
{
  return true;
}
