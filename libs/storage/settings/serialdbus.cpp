// This file is generated by kconfig_compiler from serial.kcfg.
// All changes you do to this file will be lost.

#include "serialdbus.h"

#include "serial.h"

SerialDbus::SerialDbus(SerialSetting * setting) : SettingDbus(setting)
{
}

SerialDbus::~SerialDbus()
{
}

void SerialDbus::fromMap(const QVariantMap & map)
{
  SerialSetting * setting = static_cast<SerialSetting *>(m_setting);
  if (map.contains("baud")) {
    setting->setBaud(map.value("baud").value<uint>());
  }
  if (map.contains("bits")) {
    setting->setBits(map.value("bits").value<uint>());
  }
  if (map.contains("parity")) {
    setting->setParity(map.value("parity").value<QString>());
  }
  if (map.contains("stopbits")) {
    setting->setStopbits(map.value("stopbits").value<uint>());
  }
  if (map.contains(QLatin1String(NM_SETTING_SERIAL_SEND_DELAY))) {
    setting->setSenddelay(map.value(QLatin1String(NM_SETTING_SERIAL_SEND_DELAY)).value<quint64>());
  }
}

QVariantMap SerialDbus::toMap()
{
  QVariantMap map;
  SerialSetting * setting = static_cast<SerialSetting *>(m_setting);
  map.insert("baud", setting->baud());
  map.insert("bits", setting->bits());
  map.insert("parity", setting->parity());
  map.insert("stopbits", setting->stopbits());
  map.insert(QLatin1String(NM_SETTING_SERIAL_SEND_DELAY), setting->senddelay());
  return map;
}

QVariantMap SerialDbus::toSecretsMap()
{
  QVariantMap map;
  SerialSetting * setting = static_cast<SerialSetting *>(m_setting);
  return map;
}

