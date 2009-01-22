// This file is generated by kconfig_compiler from 802-1x.kcfg.
// All changes you do to this file will be lost.

#include "802-1xpersistence.h"

#include "802-1x.h"

using namespace Knm;

Security8021xPersistence::Security8021xPersistence(Security8021xSetting * setting, KSharedConfig::Ptr config, ConnectionPersistence::SecretStorageMode mode) : SettingPersistence(setting, config, mode)
{
}

Security8021xPersistence::~Security8021xPersistence()
{
}

void Security8021xPersistence::load()
{
  Security8021xSetting * setting = static_cast<Security8021xSetting *>(m_setting);
  setting->setEap(m_config->readEntry("eap", QStringList()));
  setting->setIdentity(m_config->readEntry("identity", ""));
  setting->setAnonymousidentity(m_config->readEntry("anonymousidentity", ""));
  setting->setCacert(m_config->readEntry("cacert", QByteArray()));
  setting->setCapath(m_config->readEntry("capath", ""));
  setting->setClientcert(m_config->readEntry("clientcert", QByteArray()));
  setting->setPhase1peapver(m_config->readEntry("phase1peapver", 0));
  setting->setPhase1peaplabel(m_config->readEntry("phase1peaplabel", ""));
  setting->setPhase1fastprovisioning(m_config->readEntry("phase1fastprovisioning", ""));
  setting->setPhase2auth(m_config->readEntry("phase2auth", ""));
  setting->setPhase2autheap(m_config->readEntry("phase2autheap", 0));
  setting->setPhase2cacert(m_config->readEntry("phase2cacert", QByteArray()));
  setting->setPhase2capath(m_config->readEntry("phase2capath", ""));
  setting->setPhase2clientcert(m_config->readEntry("phase2clientcert", QByteArray()));
  // SECRET
  if (m_storageMode != ConnectionPersistence::Secure) {
    setting->setPassword(m_config->readEntry("password", ""));
  }
  setting->setPrivatekey(m_config->readEntry("privatekey", QByteArray()));
  setting->setPhase2privatekey(m_config->readEntry("phase2privatekey", QByteArray()));
  setting->setPin(m_config->readEntry("pin", ""));
  setting->setPsk(m_config->readEntry("psk", ""));
}

void Security8021xPersistence::save()
{
  Security8021xSetting * setting = static_cast<Security8021xSetting *>(m_setting);
  m_config->writeEntry("eap", setting->eap());
  m_config->writeEntry("identity", setting->identity());
  m_config->writeEntry("anonymousidentity", setting->anonymousidentity());
  m_config->writeEntry("cacert", setting->cacert());
  m_config->writeEntry("capath", setting->capath());
  m_config->writeEntry("clientcert", setting->clientcert());
  m_config->writeEntry("phase1peapver", setting->phase1peapver());
  m_config->writeEntry("phase1peaplabel", setting->phase1peaplabel());
  m_config->writeEntry("phase1fastprovisioning", setting->phase1fastprovisioning());
  m_config->writeEntry("phase2auth", setting->phase2auth());
  m_config->writeEntry("phase2autheap", setting->phase2autheap());
  m_config->writeEntry("phase2cacert", setting->phase2cacert());
  m_config->writeEntry("phase2capath", setting->phase2capath());
  m_config->writeEntry("phase2clientcert", setting->phase2clientcert());
  // SECRET
  if (m_storageMode != ConnectionPersistence::Secure) {
    m_config->writeEntry("password", setting->password());
  }
  m_config->writeEntry("privatekey", setting->privatekey());
  m_config->writeEntry("phase2privatekey", setting->phase2privatekey());
  m_config->writeEntry("pin", setting->pin());
  m_config->writeEntry("psk", setting->psk());
}

QMap<QString,QString> Security8021xPersistence::secrets() const
{
  Security8021xSetting * setting = static_cast<Security8021xSetting *>(m_setting);
  QMap<QString,QString> map;
  map.insert(QLatin1String("password"), setting->password());
  return map;
}

void Security8021xPersistence::restoreSecrets(QMap<QString,QString> secrets) const
{
  if (m_storageMode == ConnectionPersistence::Secure) {
  Security8021xSetting * setting = static_cast<Security8021xSetting *>(m_setting);
    setting->setPassword(secrets.value("password"));
    setting->setSecretsAvailable(true);
  }
}
