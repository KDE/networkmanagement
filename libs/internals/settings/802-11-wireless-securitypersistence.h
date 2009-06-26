// This file is generated by kconfig_compiler from 802-11-wireless-security.kcfg.
// All changes you do to this file will be lost.
#ifndef KNM_WIRELESSSECURITYPERSISTENCE_H
#define KNM_WIRELESSSECURITYPERSISTENCE_H

#include <kdebug.h>
#include <kcoreconfigskeleton.h>
#include "settingpersistence.h"
#include "knminternals_export.h"
namespace Knm {

class WirelessSecuritySetting;

class KNMINTERNALS_EXPORT WirelessSecurityPersistence : public SettingPersistence
{
  public:
    WirelessSecurityPersistence( WirelessSecuritySetting * setting, KSharedConfig::Ptr config, ConnectionPersistence::SecretStorageMode mode = ConnectionPersistence::Secure);
    ~WirelessSecurityPersistence();
    void load();
    void save();
    QMap<QString,QString> secrets() const;
    void restoreSecrets(QMap<QString,QString>) const;
};
}

#endif

