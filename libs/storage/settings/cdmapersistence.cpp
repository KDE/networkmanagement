// This file is generated by kconfig_compiler from cdma.kcfg.
// All changes you do to this file will be lost.

#include "cdmapersistence.h"

#include "cdma.h"

CdmaPersistence::CdmaPersistence(CdmaSetting * setting, KSharedConfig::Ptr config, ConnectionPersistence::SecretStorageMode mode) : SettingPersistence(setting, config, mode)
{
}

CdmaPersistence::~CdmaPersistence()
{
}

void CdmaPersistence::load()
{
  CdmaSetting * setting = static_cast<CdmaSetting *>(m_setting);
  setting->setNumber(m_config->readEntry("number", ""));
  setting->setUsername(m_config->readEntry("username", ""));
  // SECRET
  setting->setPassword(m_config->readEntry("password", ""));
}

void CdmaPersistence::save()
{
  CdmaSetting * setting = static_cast<CdmaSetting *>(m_setting);
  m_config->writeEntry("number", setting->number());
  m_config->writeEntry("username", setting->username());
  // SECRET
  if (m_storageMode != ConnectionPersistence::Secure) {
    m_config->writeEntry("password", setting->password());
  }
}

QMap<QString,QString> CdmaPersistence::secrets() const
{
  CdmaSetting * setting = static_cast<CdmaSetting *>(m_setting);
  QMap<QString,QString> map;
  map.insert(QLatin1String("password"), setting->password());
  return map;
}

void CdmaPersistence::restoreSecrets(QMap<QString,QString> secrets) const
{
  if (m_storageMode == ConnectionPersistence::Secure) {
  CdmaSetting * setting = static_cast<CdmaSetting *>(m_setting);
    setting->setPassword(secrets.value("password"));
    setting->setSecretsAvailable(true);
  }
}
