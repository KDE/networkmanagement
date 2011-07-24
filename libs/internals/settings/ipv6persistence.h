// This file is generated by kconfig_compiler from ipv6.kcfg.
// All changes you do to this file will be lost.
#ifndef KNM_IPV6PERSISTENCE_H
#define KNM_IPV6PERSISTENCE_H

#include <kdebug.h>
#include <kcoreconfigskeleton.h>
#include "settingpersistence.h"
#include "knminternals_export.h"
namespace Knm {

class Ipv6Setting;

class KNMINTERNALS_EXPORT Ipv6Persistence : public SettingPersistence
{
  public:
    Ipv6Persistence( Ipv6Setting * setting, KSharedConfig::Ptr config, ConnectionPersistence::SecretStorageMode mode = ConnectionPersistence::Secure);
    ~Ipv6Persistence();
    void load();
    void save();
    QMap<QString,QString> secrets() const;
    void restoreSecrets(QMap<QString,QString>) const;
};
}

#endif
