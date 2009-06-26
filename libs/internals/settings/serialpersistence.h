// This file is generated by kconfig_compiler from serial.kcfg.
// All changes you do to this file will be lost.
#ifndef KNM_SERIALPERSISTENCE_H
#define KNM_SERIALPERSISTENCE_H

#include <kdebug.h>
#include <kcoreconfigskeleton.h>
#include "settingpersistence.h"
#include "knminternals_export.h"
namespace Knm {

class SerialSetting;

class KNMINTERNALS_EXPORT SerialPersistence : public SettingPersistence
{
  public:
    SerialPersistence( SerialSetting * setting, KSharedConfig::Ptr config, ConnectionPersistence::SecretStorageMode mode = ConnectionPersistence::Secure);
    ~SerialPersistence();
    void load();
    void save();
    QMap<QString,QString> secrets() const;
    void restoreSecrets(QMap<QString,QString>) const;
};
}

#endif

