// This file is generated by kconfig_compiler from 802-11-wireless-security.kcfg.
// All changes you do to this file will be lost.

#include "802-11-wireless-securitydbus.h"

#include <wpasecretidentifier.h>

#include "config-nm09backend.h"
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

      if(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT)) == "none")
      {
          setting->setKeymgmt(Knm::WirelessSecuritySetting::EnumKeymgmt::None);
          setting->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::StaticWep);
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
    {
        setting->setAuthalg(Knm::WirelessSecuritySetting::EnumAuthalg::leap);
        setting->setSecurityType(Knm::WirelessSecuritySetting::EnumSecurityType::Leap);
        if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_USERNAME))) {
          setting->setLeapusername(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_USERNAME)).value<QString>());
        }

        //SECRET
        if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD))) {
          setting->setLeappassword(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD)).value<QString>());
        }

    }
  }

if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PROTO))) {
    setting->setProto(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PROTO)).value<QStringList>());
  }
  if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PAIRWISE))) {
      setting->setPairwise(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PAIRWISE)).value<QStringList>());
  }
  if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_GROUP))) {
      setting->setGroup(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_GROUP)).value<QStringList>());
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
  if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PSK))) {
    setting->setPsk(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PSK)).value<QString>());
  }

  if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY_TYPE))) {
      if (map.value(QLatin1String(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY_TYPE))).value<int>() == 1)
      {
        setting->setWepKeyType(Knm::WirelessSecuritySetting::Hex);
      }
      else
      {
        setting->setWepKeyType(Knm::WirelessSecuritySetting::Passphrase);
      }
  }

  setting->setPskflags((Knm::Setting::secretsTypes)map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PSK_FLAGS)).value<int>());
  setting->setWepkeyflags((Knm::Setting::secretsTypes)map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY_FLAGS)).value<int>());
  setting->setLeappasswordflags((Knm::Setting::secretsTypes)map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD_FLAGS)).value<int>());
}

QVariantMap WirelessSecurityDbus::toMap()
{
  QVariantMap map;
  Knm::WirelessSecuritySetting * setting = static_cast<Knm::WirelessSecuritySetting *>(m_setting);

  if (setting->securityType() != Knm::WirelessSecuritySetting::EnumSecurityType::None) { // don't return anything if there is no security
  switch (setting->keymgmt()) {
    case Knm::WirelessSecuritySetting::EnumKeymgmt::None:
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT), "none");
      qDebug() << "Adding key-mgmt: none";
      break;
    case Knm::WirelessSecuritySetting::EnumKeymgmt::Ieee8021x:
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT), "ieee8021x");
      qDebug() << "Adding key-mgmt: ieee8021x";
      break;
    case Knm::WirelessSecuritySetting::EnumKeymgmt::WPANone:
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT), "wpa-none");
      qDebug() << "Adding key-mgmt: wpa-none";
      break;
    case Knm::WirelessSecuritySetting::EnumKeymgmt::WPAPSK:
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT), "wpa-psk");
      qDebug() << "Adding key-mgmt: wpa-psk";
      break;
    case Knm::WirelessSecuritySetting::EnumKeymgmt::WPAEAP:
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT), "wpa-eap");
      qDebug() << "Adding key-mgmt: wpa-eap";
      break;
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
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PROTO), setting->proto());
  }
  if (!setting->pairwise().isEmpty()) {
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PAIRWISE), setting->pairwise());
  }
  if (!setting->group().isEmpty()) {
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_GROUP), setting->group());
  }
  if (!setting->leapusername().isEmpty()) {
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_USERNAME), setting->leapusername());
  }

  map.unite(toSecretsMap());

  if (!setting->wepkey0().isEmpty() || !setting->wepkey1().isEmpty() ||  !setting->wepkey2().isEmpty() || !setting->wepkey3().isEmpty()) {
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY_FLAGS), (int)setting->wepkeyflags());
  }
  if(!setting->psk().isEmpty()) {
      map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PSK_FLAGS), (int)setting->pskflags());
  }
  map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD_FLAGS), (int)setting->leappasswordflags());

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
    if (setting->securityType() == Knm::WirelessSecuritySetting::EnumSecurityType::StaticWep ||
        setting->securityType() == Knm::WirelessSecuritySetting::EnumSecurityType::DynamicWep) {
        if(setting->wepKeyType() == Knm::WirelessSecuritySetting::Hex)
            map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY_TYPE), 1);
        else if(setting->wepKeyType() == Knm::WirelessSecuritySetting::Passphrase)
            map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY_TYPE), 2);
        else
        kWarning() << "Wep key type is not set!";
        map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_TX_KEYIDX), setting->weptxkeyindex());
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
    } else if (setting->securityType() == Knm::WirelessSecuritySetting::EnumSecurityType::WpaPsk || setting->securityType() == Knm::WirelessSecuritySetting::EnumSecurityType::Wpa2Psk) {
        if (!setting->psk().isEmpty()) {
            map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PSK), setting->psk());
        }
    } else if (setting->securityType() == Knm::WirelessSecuritySetting::EnumSecurityType::Leap) {
        if (!setting->leappassword().isEmpty()) {
            map.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD), setting->leappassword());
        }
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

