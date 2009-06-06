// This file is generated by kconfig_compiler from pppoe.kcfg.
// All changes you do to this file will be lost.

#include "pppoepersistence.h"

#include "pppoe.h"

using namespace KnmInternals;

PppoePersistence::PppoePersistence(PppoeSetting * setting, KSharedConfig::Ptr config, ConnectionPersistence::SecretStorageMode mode) : SettingPersistence(setting, config, mode)
{
}

PppoePersistence::~PppoePersistence()
{
}

void PppoePersistence::load()
{
  PppoeSetting * setting = static_cast<PppoeSetting *>(m_setting);
  setting->setService(m_config->readEntry("service", ""));
  setting->setUsername(m_config->readEntry("username", ""));
  // SECRET
  if (m_storageMode != ConnectionPersistence::Secure) {
    setting->setPassword(m_config->readEntry("password", ""));
  }
}

void PppoePersistence::save()
{
  PppoeSetting * setting = static_cast<PppoeSetting *>(m_setting);
  m_config->writeEntry("service", setting->service());
  m_config->writeEntry("username", setting->username());
  // SECRET
  if (m_storageMode != ConnectionPersistence::Secure) {
    m_config->writeEntry("password", setting->password());
  }
}

QMap<QString,QString> PppoePersistence::secrets() const
{
  PppoeSetting * setting = static_cast<PppoeSetting *>(m_setting);
  QMap<QString,QString> map;
  map.insert(QLatin1String("password"), setting->password());
  return map;
}

void PppoePersistence::restoreSecrets(QMap<QString,QString> secrets) const
{
  if (m_storageMode == ConnectionPersistence::Secure) {
  PppoeSetting * setting = static_cast<PppoeSetting *>(m_setting);
    setting->setPassword(secrets.value("password"));
    setting->setSecretsAvailable(true);
  }
}
