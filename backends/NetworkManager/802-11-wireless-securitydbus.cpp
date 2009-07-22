// This file is generated by kconfig_compiler from 802-11-wireless-security.kcfg.
// All changes you do to this file will be lost.

#include "802-11-wireless-securitydbus.h"

#include "802-11-wireless-security.h"
#include "pbkdf2.h"
#include "wephash.h"

WirelessSecurityDbus::WirelessSecurityDbus(Knm::WirelessSecuritySetting * setting, const QString & essid) : SettingDbus(setting),
    m_essid(essid)
{
}

WirelessSecurityDbus::~WirelessSecurityDbus()
{
}

void WirelessSecurityDbus::fromMap(const QVariantMap & map)
{
  Knm::WirelessSecuritySetting * setting = static_cast<Knm::WirelessSecuritySetting *>(m_setting);
  if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT))) {
    setting->setKeymgmt(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT)).value<int>());
  }
  if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_TX_KEYIDX))) {
    setting->setWeptxkeyindex(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_TX_KEYIDX)).value<uint>());
  }
  if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_AUTH_ALG))) {
    setting->setAuthalg(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_AUTH_ALG)).value<int>());
  }
  if (map.contains("proto")) {
    setting->setProto(map.value("proto").value<QStringList>());
  }
  if (map.contains("pairwise")) {
    setting->setPairwise(map.value("pairwise").value<QStringList>());
  }
  if (map.contains("group")) {
    setting->setGroup(map.value("group").value<QStringList>());
  }
  if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_USERNAME))) {
    setting->setLeapusername(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_USERNAME)).value<QString>());
  }
  // SECRET
  if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY0))) {
    setting->setWepkey0(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY0)).value<QString>());
  }
  // SECRET
  if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY1))) {
    setting->setWepkey1(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY1)).value<QString>());
  }
  // SECRET
  if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY2))) {
    setting->setWepkey2(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY2)).value<QString>());
  }
  // SECRET
  if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY3))) {
    setting->setWepkey3(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY3)).value<QString>());
  }
  // SECRET
  kDebug() << "Storing hashed PSK as plaintext!";
  if (map.contains("psk")) {
    setting->setPsk(map.value("psk").value<QString>());
  }
  // SECRET
  if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD))) {
    setting->setLeappassword(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD)).value<QString>());
  }
}

QVariantMap WirelessSecurityDbus::toMap()
{
  QVariantMap map;
  Knm::WirelessSecuritySetting * setting = static_cast<Knm::WirelessSecuritySetting *>(m_setting);
  if (setting->securityType() != Knm::WirelessSecuritySetting::EnumSecurityType::None) { // don't return anything if there is no security
  switch (setting->keymgmt()) {
    case Knm::WirelessSecuritySetting::EnumKeymgmt::None:
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT), "none");
      break;
    case Knm::WirelessSecuritySetting::EnumKeymgmt::Ieee8021x:
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT), "ieee8021x");
      break;
    case Knm::WirelessSecuritySetting::EnumKeymgmt::WPANone:
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT), "wpa-none");
      break;
    case Knm::WirelessSecuritySetting::EnumKeymgmt::WPAPSK:
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT), "wpa-psk");
      break;
    case Knm::WirelessSecuritySetting::EnumKeymgmt::WPAEAP:
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT), "wpa-eap");
      break;
  }
  map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_TX_KEYIDX), setting->weptxkeyindex());
  switch (setting->authalg()) {
    case Knm::WirelessSecuritySetting::EnumAuthalg::open:
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_AUTH_ALG), "open");
      break;
    case Knm::WirelessSecuritySetting::EnumAuthalg::shared:
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_AUTH_ALG), "shared");
      break;
    case Knm::WirelessSecuritySetting::EnumAuthalg::leap:
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_AUTH_ALG), "leap");
      break;
  }
  if (!setting->proto().isEmpty()) {
      map.insert("proto", setting->proto());
  }
  if (!setting->pairwise().isEmpty()) {
      map.insert("pairwise", setting->pairwise());
  }
  if (!setting->group().isEmpty()) {
      map.insert("group", setting->group());
  }
  if (!setting->leapusername().isEmpty()) {
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_USERNAME), setting->leapusername());
  }
  } // end of if not setting->clear()
  return map;
}

QVariantMap WirelessSecurityDbus::toSecretsMap()
{
  QVariantMap map;
  Knm::WirelessSecuritySetting * setting = static_cast<Knm::WirelessSecuritySetting *>(m_setting);
  if (setting->securityType() != Knm::WirelessSecuritySetting::EnumSecurityType::None) { // don't return anything if there is no security
  if (!setting->weppassphrase().isEmpty()) {
      QString key = hashWepPassphrase(setting->weppassphrase());
      switch (setting->weptxkeyindex()) {
          case 0:
              setting->setWepkey0(key);
              break;
          case 1:
              setting->setWepkey1(key);
              break;
          case 2:
              setting->setWepkey2(key);
              break;
          case 3:
              setting->setWepkey3(key);
              break;
      }
  }
  if (!setting->wepkey0().isEmpty()) {
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY0), setting->wepkey0());
  }
  if (!setting->wepkey1().isEmpty()) {
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY1), setting->wepkey1());
  }
  if (!setting->wepkey2().isEmpty()) {
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY2), setting->wepkey2());
  }
  if (!setting->wepkey3().isEmpty()) {
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY3), setting->wepkey3());
  }
  if (!setting->psk().isEmpty()) {
      map.insert("psk", hashWpaPsk(setting->psk()));
  }
  if (!setting->leappassword().isEmpty()) {
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD), setting->leappassword());
  }
  } // end of if not clear
  return map;
}

QString WirelessSecurityDbus::hashWpaPsk(const QString & plainText)
{
#define WPA_PMK_LEN 32
    //kDebug() << "Hashing PSK. essid:" << m_essid << "psk: <omitted>"/* << plainText*/;
    QByteArray buffer(WPA_PMK_LEN * 2, 0);
    pbkdf2_sha1(plainText.toLatin1(), m_essid.toLatin1(), m_essid.size(), 4096, (quint8*)buffer.data(), WPA_PMK_LEN);
    QString hexHash = buffer.toHex().left(WPA_PMK_LEN*2);
    //kDebug() << "  hexadecimal key out:" << hexHash;
    return hexHash;
}

QString WirelessSecurityDbus::hashWepPassphrase(const QString & plainText)
{
    //kDebug() << "Hashing wep passphrase, essid: " << essid << " passphrase: " << passphrase;
    QString hexHash = wep128PassphraseHash(plainText.toAscii());
    //kDebug() << "Hexadecimal key out:" << hexHash;
    //kDebug() << "for wep key: " << wepkey;

    return hexHash;
}

