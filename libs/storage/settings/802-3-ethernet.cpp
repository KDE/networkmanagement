// This file is generated by kconfig_compiler from 802-3-ethernet.kcfg.
// All changes you do to this file will be lost.

#include "802-3-ethernet.h"

using namespace Knm;

WiredSetting::WiredSetting() : Setting(Setting::Wired)
{
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
