// This file is generated by kconfig_compiler from 802-3-ethernet.kcfg.
// All changes you do to this file will be lost.
#ifndef KNM_WIREDPERSISTENCE_H
#define KNM_WIREDPERSISTENCE_H

#include <kdebug.h>
#include <kcoreconfigskeleton.h>
#include "settingpersistence.h"
#include "knm_export.h"
namespace Knm {

class WiredSetting;

class KNM_EXPORT WiredPersistence : public SettingPersistence
{
  public:
    WiredPersistence( WiredSetting * setting, KSharedConfig::Ptr config, ConnectionPersistence::SecretStorageMode mode = ConnectionPersistence::Secure);
    ~WiredPersistence();
    void load();
    void save();
    QMap<QString,QString> secrets() const;
    void restoreSecrets(QMap<QString,QString>) const;
};
}

#endif

