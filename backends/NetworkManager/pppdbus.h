// This file is generated by kconfig_compiler from ppp.kcfg.
// All changes you do to this file will be lost.
#ifndef PPPDBUS_H
#define PPPDBUS_H

#include <nm-setting-ppp.h>

#include <kdebug.h>
#include <kcoreconfigskeleton.h>
#include "settingdbus.h"
#include "knm_export.h"
namespace Knm{
    class PppSetting;
}

class KNM_EXPORT PppDbus : public SettingDbus
{
  public:
    PppDbus(Knm::PppSetting * setting);
    ~PppDbus();
    void fromMap(const QVariantMap&);
    QVariantMap toMap();
    QVariantMap toSecretsMap();
};
#endif

