// This file is generated by kconfig_compiler from ipv6.kcfg.
// All changes you do to this file will be lost.

#include "ipv6.h"

using namespace Knm;

Ipv6Setting::Ipv6Setting() : Setting(Setting::Ipv6),
    mMethod(Ipv6Setting::EnumMethod::Ignore),
    mIgnoredhcpdns(false),
    mIgnoreautoroute(false),
    mNeverdefault(false),
    mMayfail(true)
{
}

Ipv6Setting::Ipv6Setting(Ipv6Setting *setting) : Setting(setting)
{
    setMethod(setting->method());
    setDns(setting->dns());
    setDnssearch(setting->dnssearch());
    setAddresses(setting->addresses());
    setIgnoredhcpdns(setting->ignoredhcpdns());
    setIgnoreautoroute(setting->ignoreautoroute());
    setNeverdefault(setting->neverdefault());
    setRoutes(setting->routes());
    setMayfail(setting->mayfail());
}

Ipv6Setting::~Ipv6Setting()
{
}

QString Ipv6Setting::name() const
{
  return QLatin1String("ipv6");
}
bool Ipv6Setting::hasSecrets() const
{
  return false;
}
