// This file is generated by kconfig_compiler from 802-1x.kcfg.
// All changes you do to this file will be lost.

#include "802-1xdbus.h"

#include "802-1x.h"

using namespace Knm;

Security8021xDbus::Security8021xDbus(Security8021xSetting * setting) : SettingDbus(setting)
{
}

Security8021xDbus::~Security8021xDbus()
{
}

void Security8021xDbus::fromMap(const QVariantMap & map)
{
  Security8021xSetting * setting = static_cast<Security8021xSetting *>(m_setting);
  if (map.contains("eap")) {
    setting->setEap(map.value("eap").value<QStringList>());
  }
  if (map.contains("identity")) {
    setting->setIdentity(map.value("identity").value<QString>());
  }
  if (map.contains(QLatin1String(NM_SETTING_802_1X_ANONYMOUS_IDENTITY))) {
    setting->setAnonymousidentity(map.value(QLatin1String(NM_SETTING_802_1X_ANONYMOUS_IDENTITY)).value<QString>());
  }
  if (map.contains(QLatin1String(NM_SETTING_802_1X_CA_CERT))) {
    setting->setCacert(map.value(QLatin1String(NM_SETTING_802_1X_CA_CERT)).value<QByteArray>());
  }
  if (map.contains(QLatin1String(NM_SETTING_802_1X_CA_PATH))) {
    setting->setCapath(map.value(QLatin1String(NM_SETTING_802_1X_CA_PATH)).value<QString>());
  }
  if (map.contains(QLatin1String(NM_SETTING_802_1X_CLIENT_CERT))) {
    setting->setClientcert(map.value(QLatin1String(NM_SETTING_802_1X_CLIENT_CERT)).value<QByteArray>());
  }
  if (map.contains(QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPVER))) {
    setting->setPhase1peapver(map.value(QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPVER)).value<int>());
  }
  if (map.contains(QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPLABEL))) {
    setting->setPhase1peaplabel(map.value(QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPLABEL)).value<QString>());
  }
  if (map.contains(QLatin1String(NM_SETTING_802_1X_PHASE1_FAST_PROVISIONING))) {
    setting->setPhase1fastprovisioning(map.value(QLatin1String(NM_SETTING_802_1X_PHASE1_FAST_PROVISIONING)).value<QString>());
  }
  if (map.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTH))) {
    setting->setPhase2auth(map.value(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTH)).value<QString>());
  }
  if (map.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTHEAP))) {
    setting->setPhase2autheap(map.value(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTHEAP)).value<int>());
  }
  if (map.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_CA_CERT))) {
    setting->setPhase2cacert(map.value(QLatin1String(NM_SETTING_802_1X_PHASE2_CA_CERT)).value<QByteArray>());
  }
  if (map.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_CA_PATH))) {
    setting->setPhase2capath(map.value(QLatin1String(NM_SETTING_802_1X_PHASE2_CA_PATH)).value<QString>());
  }
  if (map.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_CLIENT_CERT))) {
    setting->setPhase2clientcert(map.value(QLatin1String(NM_SETTING_802_1X_PHASE2_CLIENT_CERT)).value<QByteArray>());
  }
  // SECRET
  if (map.contains("password")) {
    setting->setPassword(map.value("password").value<QString>());
  }
  if (map.contains(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY))) {
    setting->setPrivatekey(map.value(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY)).value<QByteArray>());
  }
  if (map.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY))) {
    setting->setPhase2privatekey(map.value(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY)).value<QByteArray>());
  }
  if (map.contains("pin")) {
    setting->setPin(map.value("pin").value<QString>());
  }
  if (map.contains("psk")) {
    setting->setPsk(map.value("psk").value<QString>());
  }
}

QVariantMap Security8021xDbus::toMap()
{
  QVariantMap map;
  Security8021xSetting * setting = static_cast<Security8021xSetting *>(m_setting);
  map.insert("eap", setting->eap());
  map.insert("identity", setting->identity());
  map.insert(QLatin1String(NM_SETTING_802_1X_ANONYMOUS_IDENTITY), setting->anonymousidentity());
  map.insert(QLatin1String(NM_SETTING_802_1X_CA_CERT), setting->cacert());
  map.insert(QLatin1String(NM_SETTING_802_1X_CA_PATH), setting->capath());
  map.insert(QLatin1String(NM_SETTING_802_1X_CLIENT_CERT), setting->clientcert());
  map.insert(QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPVER), setting->phase1peapver());
  map.insert(QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPLABEL), setting->phase1peaplabel());
  map.insert(QLatin1String(NM_SETTING_802_1X_PHASE1_FAST_PROVISIONING), setting->phase1fastprovisioning());
  map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTH), setting->phase2auth());
  map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTHEAP), setting->phase2autheap());
  map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_CA_CERT), setting->phase2cacert());
  map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_CA_PATH), setting->phase2capath());
  map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_CLIENT_CERT), setting->phase2clientcert());
  map.insert(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY), setting->privatekey());
  map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY), setting->phase2privatekey());
  map.insert("pin", setting->pin());
  map.insert("psk", setting->psk());
  return map;
}

QVariantMap Security8021xDbus::toSecretsMap()
{
  QVariantMap map;
  Security8021xSetting * setting = static_cast<Security8021xSetting *>(m_setting);
  map.insert("name", setting->name());
  map.insert("password", setting->password());
  return map;
}

