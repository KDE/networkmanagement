// This file is generated by kconfig_compiler from vpn.kcfg.
// All changes you do to this file will be lost.

#include "vpnpersistence.h"

#include <KUser>

#include "vpn.h"

using namespace Knm;

VpnPersistence::VpnPersistence(VpnSetting * setting, KSharedConfig::Ptr config, ConnectionPersistence::SecretStorageMode mode) : SettingPersistence(setting, config, mode)
{
}

VpnPersistence::~VpnPersistence()
{
}

void VpnPersistence::load()
{
  VpnSetting * setting = static_cast<VpnSetting *>(m_setting);
  setting->setServiceType(m_config->readEntry("ServiceType", ""));
  setting->setData(stringMapFromStringList(m_config->readEntry("Data", QStringList())));
  setting->setSecretsStorageType(stringMapFromStringList(m_config->readEntry("SecretsStorageType", QStringList())));
  setting->setUserName(m_config->readEntry("UserName", ""));
  // SECRET
  if (m_storageMode == ConnectionPersistence::PlainText) {
      // the map is flattened to a list of key,value,key,value
      setting->setVpnSecrets(variantMapFromStringList(m_config->readEntry("VpnSecrets", QStringList())));
    setting->setSecretsAvailable(true);
  }
  setting->setPluginName(m_config->readEntry("PluginName", ""));
  setting->setInitialized();
}

void VpnPersistence::save()
{
  VpnSetting * setting = static_cast<VpnSetting *>(m_setting);
  m_config->writeEntry("ServiceType", setting->serviceType());
  m_config->writeEntry("Data", stringMapToStringList(setting->data()));
  m_config->writeEntry("SecretsStorageType", stringMapToStringList(setting->secretsStorageType()));
  m_config->writeEntry("UserName", setting->userName());
  // SECRET
  if (m_storageMode == ConnectionPersistence::PlainText) {
    /*
     * Save only secrets with storage type == "save"
     * For compatibility treat empty storage type as "save"
     */
    m_config->writeEntry("VpnSecrets", variantMapToStringList(secretsToSave(setting->secretsStorageType(), setting->vpnSecrets())));
  }
  m_config->writeEntry("PluginName", setting->pluginName());
}

QVariantMap VpnPersistence::variantMapFromStringList(const QStringList & list)
{
    QVariantMap secretsMap;
    if (list.count() % 2 == 0) {
        for ( int i = 0; i < list.count(); i += 2 ) {
            secretsMap.insert( list[i], list[i+1] );
        }
    }
    return secretsMap;
}

QStringList VpnPersistence::variantMapToStringList(const QVariantMap & map)
{
    QStringList rawSecrets;
    QMapIterator<QString,QVariant> i(map);
    while (i.hasNext()) {
        i.next();
	  rawSecrets << i.key() << i.value().toString();
    }
    return rawSecrets;
}

QStringMap VpnPersistence::stringMapFromStringList(const QStringList & list)
{
    QStringMap map;
    if (list.count() % 2 == 0) {
        for ( int i = 0; i < list.count(); i += 2 ) {
            map.insert( list[i], list[i+1] );
        }
    }
    return map;
}

QStringList VpnPersistence::stringMapToStringList(const QStringMap & map)
{
    QStringList rawSecrets;
    QStringMapIterator i(map);
    while (i.hasNext()) {
        i.next();
	  rawSecrets << i.key() << i.value();
    }
    return rawSecrets;
}

QMap<QString,QString> VpnPersistence::secrets() const
{
  VpnSetting * setting = static_cast<VpnSetting *>(m_setting);
  QMap<QString,QString> map;
  map.insert(QLatin1String("VpnSecrets"), variantMapToStringList(secretsToSave(setting->secretsStorageType(), setting->vpnSecrets())).join(QLatin1String("%SEP%")));
  return map;
}

void VpnPersistence::restoreSecrets(QMap<QString,QString> secrets) const
{
  if (m_storageMode == ConnectionPersistence::Secure) {
    VpnSetting * setting = static_cast<VpnSetting *>(m_setting);
    setting->setVpnSecrets(variantMapFromStringList(secrets.value("VpnSecrets").split("%SEP%")));
    setting->setSecretsAvailable(true);
  }
}

QVariantMap VpnPersistence::secretsToSave(const QStringMap & type, const QVariantMap & secrets)
{
  QVariantMap toSave;
  QMapIterator<QString,QVariant> i(secrets);
  while (i.hasNext()) {
      i.next();
      if (type[i.key()].isNull() || type[i.key()] == QLatin1String(NM_VPN_PW_TYPE_SAVE))
	  toSave.insert( i.key(), i.value() );
  }
  return toSave;
}