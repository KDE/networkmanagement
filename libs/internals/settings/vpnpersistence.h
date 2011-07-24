// This file is generated by kconfig_compiler from vpn.kcfg.
// All changes you do to this file will be lost.
#ifndef KNM_VPNPERSISTENCE_H
#define KNM_VPNPERSISTENCE_H

#include <kdebug.h>
#include <kcoreconfigskeleton.h>
#include "settingpersistence.h"
#include "knminternals_export.h"
#include "../types.h"

namespace Knm {

class VpnSetting;

class KNMINTERNALS_EXPORT VpnPersistence : public SettingPersistence
{
  public:
    VpnPersistence( VpnSetting * setting, KSharedConfig::Ptr config, ConnectionPersistence::SecretStorageMode mode = ConnectionPersistence::Secure);
    ~VpnPersistence();
    void load();
    void save();
    QMap<QString,QString> secrets() const;
    void restoreSecrets(QMap<QString,QString>) const;
    static QStringList variantMapToStringList(const QVariantMap &);
    static QVariantMap variantMapFromStringList(const QStringList &);
    static QStringList stringMapToStringList(const QStringMap &);
    static QStringMap stringMapFromStringList(const QStringList &);
    static QVariantMap secretsToSave(const QStringMap &, const QVariantMap &);
};
}

#endif
