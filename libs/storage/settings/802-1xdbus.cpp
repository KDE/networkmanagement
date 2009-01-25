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
  if (setting->enabled()) {
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
}

QVariantMap Security8021xDbus::toMap()
{
  QVariantMap map;
  Security8021xSetting * setting = static_cast<Security8021xSetting *>(m_setting);
  if (setting->enabled()) {
  if (!setting->eap().isEmpty()) {
    map.insert("eap", setting->eap());
  }
  if (!setting->identity().isEmpty()) {
    map.insert("identity", setting->identity());
  }
  if (!setting->anonymousidentity().isEmpty()) {
    map.insert(QLatin1String(NM_SETTING_802_1X_ANONYMOUS_IDENTITY), setting->anonymousidentity());
  }
  if (!setting->cacert().isEmpty()) {
    map.insert(QLatin1String(NM_SETTING_802_1X_CA_CERT), setting->cacert());
  }
  if (!setting->capath().isEmpty()) {
    map.insert(QLatin1String(NM_SETTING_802_1X_CA_PATH), setting->capath());
    }
  if (!setting->clientcert().isEmpty()) {
    map.insert(QLatin1String(NM_SETTING_802_1X_CLIENT_CERT), setting->clientcert());
  }
  if (setting->eap().contains(QLatin1String("peap"))) {  // just a guess
    switch (setting->phase1peapver()) {
      case Security8021xSetting::EnumPhase1peapver::zero:
        map.insert(QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPVER), 0);
        break;
      case Security8021xSetting::EnumPhase1peapver::one:
        map.insert(QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPVER), 1);
        break;
    }
    map.insert(QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPLABEL), setting->phase1peaplabel());
  }
  if (!setting->phase1fastprovisioning().isEmpty()) {
    map.insert(QLatin1String(NM_SETTING_802_1X_PHASE1_FAST_PROVISIONING), setting->phase1fastprovisioning());
  }
  if (!setting->phase2auth().isEmpty()) {
    map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTH), setting->phase2auth());
    switch (setting->phase2autheap()) {
    case Security8021xSetting::EnumPhase2autheap::pap:
      map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTHEAP), "pap");
      break;
    case Security8021xSetting::EnumPhase2autheap::mschap:
      map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTHEAP), "mschap");
      break;
    case Security8021xSetting::EnumPhase2autheap::mschapv2:
      map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTHEAP), "mschapv2");
      break;
    case Security8021xSetting::EnumPhase2autheap::chap:
      map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTHEAP), "chap");
      break;
    }
    map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_CA_CERT), setting->phase2cacert());
    map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_CA_PATH), setting->phase2capath());
    map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_CLIENT_CERT), setting->phase2clientcert());
    map.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY), setting->phase2privatekey());
  }
  if (!setting->privatekey().isEmpty()) {
      map.insert(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY), setting->privatekey());
  }
  if (!setting->pin().isEmpty()) {
      map.insert("pin", setting->pin());
  }
  if (!setting->psk().isEmpty()) {
      map.insert("psk", setting->psk());
  }
  }
  return map;
}

QVariantMap Security8021xDbus::toSecretsMap()
{
  QVariantMap map;
  Security8021xSetting * setting = static_cast<Security8021xSetting *>(m_setting);
  if (setting->enabled()) {
  map.insert("name", setting->name());
  map.insert("password", setting->password());
  }
  return map;
}

