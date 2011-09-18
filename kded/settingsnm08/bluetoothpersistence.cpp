// This file is generated by kconfig_compiler from bluetooth.kcfg.
// All changes you do to this file will be lost.

#include <nm-setting-bluetooth.h>

#include "bluetoothpersistence.h"

using namespace Knm;

BluetoothPersistence::BluetoothPersistence(BluetoothSetting * setting, KSharedConfig::Ptr config, SettingPersistence::SecretStorageMode mode) : SettingPersistence(setting, config, mode)
{
}

BluetoothPersistence::~BluetoothPersistence()
{
}

void BluetoothPersistence::load()
{
  BluetoothSetting * setting = static_cast<BluetoothSetting *>(m_setting);
  setting->setNetworkname(m_config->readEntry("name", ""));
  setting->setBdaddr(macaddressFromString(m_config->readEntry("bdaddr", QString())));
  setting->setNetworktype(m_config->readEntry("type", NM_SETTING_BLUETOOTH_TYPE_DUN));
}