// This file is generated by kconfig_compiler from 802-11-wireless-security.kcfg.
// All changes you do to this file will be lost.

#include "802-11-wireless-securitydbus.h"

#include <wpasecretidentifier.h>

#include "config-nm07backend.h"
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
  bool enabled = true;

  if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT))) {
      kDebug() << "Wireless security type in DBus map is " << map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT));

      if(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT)) == "none")
      {
          setting->setKeymgmt(Knm::WirelessSecuritySetting::EnumKeymgmt::None);
          setting->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::StaticWep);
          enabled = false; // this is enabled only if at least one of the four possible secret keys is also available.
      }
      else if (map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT)) == "ieee8021x")
      {
          setting->setKeymgmt(Knm::WirelessSecuritySetting::EnumKeymgmt::Ieee8021x);
          setting->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::DynamicWep);
      }
      else if (map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT)) == "wpa-none")
      {
          setting->setKeymgmt(Knm::WirelessSecuritySetting::EnumKeymgmt::WPANone);
      }
      else if (map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT)) == "wpa-psk")
      {
          setting->setKeymgmt(Knm::WirelessSecuritySetting::EnumKeymgmt::WPAPSK);
          setting->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::WpaPsk);
      }
      else if (map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT)) == "wpa-eap")
      {
          setting->setKeymgmt(Knm::WirelessSecuritySetting::EnumKeymgmt::WPAEAP);
          setting->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::WpaEap);
      }
    setting->setSecretsAvailable(true);
  }

  if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_TX_KEYIDX))) {
    setting->setWeptxkeyindex(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_TX_KEYIDX)).value<uint>());
  }

  if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_AUTH_ALG))) {

    if (map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_AUTH_ALG)) == "open" )
        setting->setAuthalg(Knm::WirelessSecuritySetting::EnumAuthalg::open);
    else if (map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_AUTH_ALG)) == "shared")
        setting->setAuthalg(Knm::WirelessSecuritySetting::EnumAuthalg::shared);
    else if (map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_AUTH_ALG)) == "leap")
        setting->setAuthalg(Knm::WirelessSecuritySetting::EnumAuthalg::leap);
  }

  if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_USERNAME))) {
      setting->setLeapusername(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_USERNAME)).value<QString>());
      setting->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::Leap);
  }

    //SECRET
  if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD))) {
      setting->setLeappassword(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD)).value<QString>());
      setting->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::Leap);
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

  // SECRET
  if (map.contains("psk")) {
    setting->setPsk(map.value("psk").value<QString>());
  }

  if (map.contains(NM_SETTING_WIRELESS_SECURITY_WEP_KEY_TYPE)) {
      if (map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY_TYPE)).value<int>() == (int) NM_WEP_KEY_TYPE_KEY)
      {
          setting->setWepKeyType(Knm::WirelessSecuritySetting::Hex);

          // SECRET
          if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY0))) {
              setting->setWepkey0(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY0)).value<QString>());
              enabled = true;
          }
          // SECRET
          if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY1))) {
              setting->setWepkey1(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY1)).value<QString>());
              enabled = true;
          }
          // SECRET
          if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY2))) {
              setting->setWepkey2(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY2)).value<QString>());
              enabled = true;
          }
          // SECRET
          if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY3))) {
              setting->setWepkey3(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY3)).value<QString>());
              enabled = true;
          }
      } else {
        setting->setWepKeyType(Knm::WirelessSecuritySetting::Passphrase);

        // SECRET
        if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY0))) {
          setting->setWeppassphrase(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY0)).value<QString>());
          enabled = true;
        }
      }
  }

  setting->setEnabled(enabled);
  setting->setInitialized();
}

QVariantMap WirelessSecurityDbus::toMap()
{
  QVariantMap map;
  Knm::WirelessSecuritySetting * setting = static_cast<Knm::WirelessSecuritySetting *>(m_setting);

  if (setting->securityType() != Knm::WirelessSecuritySetting::EnumSecurityType::None) { // don't return anything if there is no security
  switch (setting->keymgmt()) {
    case Knm::WirelessSecuritySetting::EnumKeymgmt::None:
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT), "none");
      kDebug() << "Adding key-mgmt: none";
      break;
    case Knm::WirelessSecuritySetting::EnumKeymgmt::Ieee8021x:
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT), "ieee8021x");
      kDebug() << "Adding key-mgmt: ieee8021x";
      break;
    case Knm::WirelessSecuritySetting::EnumKeymgmt::WPANone:
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT), "wpa-none");
      kDebug() << "Adding key-mgmt: wpa-none";
      break;
    case Knm::WirelessSecuritySetting::EnumKeymgmt::WPAPSK:
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT), "wpa-psk");
      kDebug() << "Adding key-mgmt: wpa-psk";
      break;
    case Knm::WirelessSecuritySetting::EnumKeymgmt::WPAEAP:
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT), "wpa-eap");
      kDebug() << "Adding key-mgmt: wpa-eap";
      break;
  }
  // only insert WEP key index if we are using WEP
  if (setting->securityType() == Knm::WirelessSecuritySetting::EnumSecurityType::StaticWep
          || setting->securityType() == Knm::WirelessSecuritySetting::EnumSecurityType::DynamicWep) {
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_TX_KEYIDX), setting->weptxkeyindex());
  }
  switch (setting->authalg()) {
      case Knm::WirelessSecuritySetting::EnumAuthalg::none:
          // the none auth alg is internal
          break;
      case Knm::WirelessSecuritySetting::EnumAuthalg::open:
          // the default
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

  if (!setting->leappassword().isEmpty()) {
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD), setting->leappassword());
  }

  // SECRET
  if(!setting->psk().isEmpty())
      map.insert("psk",  setting->psk());

  if (setting->securityType() == Knm::WirelessSecuritySetting::EnumSecurityType::StaticWep ||
      setting->securityType() == Knm::WirelessSecuritySetting::EnumSecurityType::DynamicWep) {
    if(setting->wepKeyType() == Knm::WirelessSecuritySetting::Hex)
    {
        map.insert(NM_SETTING_WIRELESS_SECURITY_WEP_KEY_TYPE, NM_WEP_KEY_TYPE_KEY);
  
        // SECRET
        if(!setting->wepkey0().isEmpty())
            map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY0),  setting->wepkey0());
  
        // SECRET
        if(!setting->wepkey1().isEmpty())
            map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY1),  setting->wepkey1());
  
        // SECRET
        if(!setting->wepkey2().isEmpty())
            map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY2),  setting->wepkey2());
  
        // SECRET
        if(!setting->wepkey3().isEmpty())
            map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY3),  setting->wepkey3());
  
    }
    else if(setting->wepKeyType() == Knm::WirelessSecuritySetting::Passphrase)
    {
        map.insert(NM_SETTING_WIRELESS_SECURITY_WEP_KEY_TYPE, NM_WEP_KEY_TYPE_PASSPHRASE);

        if (!setting->weppassphrase().isEmpty())
            map.insert(NM_SETTING_WIRELESS_SECURITY_WEP_KEY0, setting->weppassphrase());
    }
    else
        kWarning() << "Wep key type is not set!";
  }
  } // end of if not setting->clear()
  return map;
}

/**
 * Turns a key that may be ascii into the equivalent hex string
 */
static QString toHexKey(const QString & key)
{
    if (key.length() == 5 || key.length() == 13) {
        return key.toAscii().toHex();
    } else {
        return key;
    }
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
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY0), toHexKey(setting->wepkey0()));
  }
  if (!setting->wepkey1().isEmpty()) {
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY1), toHexKey(setting->wepkey1()));
  }
  if (!setting->wepkey2().isEmpty()) {
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY2), toHexKey(setting->wepkey2()));
  }
  if (!setting->wepkey3().isEmpty()) {
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY3), toHexKey(setting->wepkey3()));
  }
  if (!setting->psk().isEmpty()) {
      WpaSecretIdentifier::WpaSecretType secretType = WpaSecretIdentifier::identify(setting->psk());
      if (secretType == WpaSecretIdentifier::Passphrase) {
          map.insert("psk", hashWpaPsk(setting->psk()));
      } else if (secretType == WpaSecretIdentifier::PreSharedKey) {
          map.insert("psk", setting->psk());
      }
  }
  if (!setting->leappassword().isEmpty()) {
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD), setting->leappassword());
  }
  } // end of if not clear
  return map;
}

QString WirelessSecurityDbus::hashWpaPsk(const QString & plainText)
{
    QString result;
//#ifdef NM_0_7_1
#if 0
    kDebug() << "Built for NetworkManager that can hash WPA keys itself; passing through plaintext";
    result = plainText.toLocal8Bit();
    kDebug() << "  plaintext out:" << result;
#else
#define WPA_PMK_LEN 32
    //kDebug() << "Hashing PSK. essid:" << m_essid << "psk:" << plainText;
    QByteArray buffer(WPA_PMK_LEN * 2, 0);
    pbkdf2_sha1(plainText.toLatin1(), m_essid.toLatin1(), m_essid.size(), 4096, (quint8*)buffer.data(), WPA_PMK_LEN);
    result = buffer.toHex().left(WPA_PMK_LEN*2);
    //kDebug() << "  hexadecimal key out:" << result;
#endif
    return result;

}

QString WirelessSecurityDbus::hashWepPassphrase(const QString & plainText)
{
    //kDebug() << "Hashing wep passphrase, essid: " << essid << " passphrase: " << passphrase;
    QString hexHash = wep128PassphraseHash(plainText.toAscii());
    //kDebug() << "Hexadecimal key out:" << hexHash;
    //kDebug() << "for wep key: " << wepkey;

    return hexHash;
}

