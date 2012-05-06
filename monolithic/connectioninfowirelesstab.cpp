/*
Copyright 2009 Andrey Batyiev <batyiev@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <KLocale>
#include "connectioninfowirelesstab.h"

#include <uiutils.h>


ConnectionInfoWirelessTab::ConnectionInfoWirelessTab(Solid::Control::WirelessNetworkInterface *iface, QWidget *parent)
    : QWidget(parent), m_iface(iface), m_ap(0), m_maxBitRate(0)
{
    QFormLayout *layout = new QFormLayout(this);
    setLayout(layout);

    m_ssidLabel = new QLabel(this);
    layout->addRow(i18nc("@label:textbox access point SSID", "SSID:"), m_ssidLabel);

    m_signalStrengthLabel = new QLabel(this);
    layout->addRow(i18nc("@label:textbox signal strength of AP", "Signal strength:"), m_signalStrengthLabel);

    m_operationModeLabel = new QLabel(this);
    layout->addRow(i18nc("@label:textbox Wi-Fi mode, e.g. Ad-Hoc or Managed", "Operation mode:"), m_operationModeLabel);

    m_bitRateLabel = new QLabel(this);
    layout->addRow(i18nc("@label:textbox bit rate of network interface", "Bit rate:"), m_bitRateLabel);

    m_accessPointLabel = new QLabel(this);
    layout->addRow(i18nc("@label:textbox MAC address of access point", "Access point:"), m_accessPointLabel);

    m_securityWpaLabel = new QLabel(this);
    layout->addRow(i18nc("@label:textbox security flags of AP", "Security (WPA):"), m_securityWpaLabel);

    m_securityRsnLabel = new QLabel(this);
    layout->addRow(i18nc("@label:textbox security flags of AP", "Security (RSN/WPA2):"), m_securityRsnLabel);

    m_frequencyLabel = new QLabel(this);
    layout->addRow(i18nc("@label:textbox Wi-Fi frequency", "Frequency:"), m_frequencyLabel);

    m_hardwareAddressLabel = new QLabel(this);
    layout->addRow(i18nc("@label:textbox MAC address of network interface", "Hardware address:"), m_hardwareAddressLabel);

    connect(iface, SIGNAL(activeAccessPointChanged(QString)), this, SLOT(updateAccessPoint(QString)));

    connect(iface, SIGNAL(bitRateChanged(int)), this, SLOT(updateBitRate(int)));
    
    connect(iface, SIGNAL(modeChanged(Solid::Control::WirelessNetworkInterface::OperationMode)), this, SLOT(updateOperationMode(Solid::Control::WirelessNetworkInterface::OperationMode)));
   
    // populate initial values
    updateAccessPoint(iface->activeAccessPoint());
    updateBitRate(iface->bitRate());
    updateOperationMode(iface->mode());

    m_hardwareAddressLabel->setText(iface->hardwareAddress());
}

void ConnectionInfoWirelessTab::updateAccessPoint(const QString &uni)
{
    Solid::Control::AccessPoint *ap = m_iface->findAccessPoint(uni);

    // drop input from previous AP (if any)
    if (m_ap != 0) {
        disconnect(m_ap, SIGNAL(ssidChanged(QString)), this, SLOT(updateSsid(QString)));
        disconnect(m_ap, SIGNAL(signalStrengthChanged(int)), this, SLOT(updateSignalStrength(int)));
        disconnect(m_ap, SIGNAL(bitRateChanged(int)), this, SLOT(updateMaxBitRate(int)));
        disconnect(m_ap, SIGNAL(frequencyChanged(uint)), this, SLOT(updateFrequency(uint)));
        disconnect(m_ap, SIGNAL(wpaFlagsChanged(Solid::Control::AccessPoint::WpaFlags)), this, SLOT(updateWpa(Solid::Control::AccessPoint::WpaFlags)));
        disconnect(m_ap, SIGNAL(rsnFlagsChanged(Solid::Control::AccessPoint::WpaFlags)), this, SLOT(updateRsn(Solid::Control::AccessPoint::WpaFlags)));
    }
    m_ap = ap;
    
    // we have lost AP
    if (ap == 0) {
        clearFields();
        return;
    }
    
    
    // populate initial values
    updateSsid(ap->ssid());
    updateSignalStrength(ap->signalStrength());
    m_accessPointLabel->setText(ap->hardwareAddress());
    updateMaxBitRate(ap->maxBitRate());
    updateFrequency(ap->frequency());
    updateWpa(ap->wpaFlags());
    updateRsn(ap->rsnFlags());

    connect(ap, SIGNAL(ssidChanged(QString)), this, SLOT(updateSsid(QString)));
    connect(ap, SIGNAL(signalStrengthChanged(int)), this, SLOT(updateSignalStrength(int)));
    connect(ap, SIGNAL(bitRateChanged(int)), this, SLOT(updateMaxBitRate(int)));
    connect(ap, SIGNAL(frequencyChanged(uint)), this, SLOT(updateFrequency(uint)));
    connect(ap, SIGNAL(wpaFlagsChanged(Solid::Control::AccessPoint::WpaFlags)), this, SLOT(updateWpa(Solid::Control::AccessPoint::WpaFlags)));
    connect(ap, SIGNAL(rsnFlagsChanged(Solid::Control::AccessPoint::WpaFlags)), this, SLOT(updateRsn(Solid::Control::AccessPoint::WpaFlags)));
}

void ConnectionInfoWirelessTab::updateBitRate(int rate)
{
    m_bitRateLabel->setText(i18nc("@item:intext bit rate value %1 of %2 maximum", "%1 / %2 Mbit/s", rate / 1000, m_maxBitRate / 1000));
}

void ConnectionInfoWirelessTab::updateOperationMode(Solid::Control::WirelessNetworkInterface::OperationMode mode)
{
    m_operationModeLabel->setText(UiUtils::operationModeToString(mode));
}

void ConnectionInfoWirelessTab::updateSsid(const QString& ssid)
{
    m_ssidLabel->setText(ssid);
}

void ConnectionInfoWirelessTab::updateSignalStrength(int signalStrength)
{
    m_signalStrengthLabel->setText(i18nc("@item:intext signal strength %1 in percent", "%1%", signalStrength));
}

void ConnectionInfoWirelessTab::updateMaxBitRate(int bitRate)
{
    m_maxBitRate = bitRate;
    updateBitRate(m_iface->bitRate());
}

void ConnectionInfoWirelessTab::updateFrequency(uint frequency)
{
    m_frequencyLabel->setText(i18nc("@item:intext Wi-Fi frequency", "%1 MHz", frequency));
}

void ConnectionInfoWirelessTab::updateWpa(Solid::Control::AccessPoint::WpaFlags flags)
{
    m_securityWpaLabel->setText(UiUtils::wpaFlagsToStringList(flags).join("\n"));
}

void ConnectionInfoWirelessTab::updateRsn(Solid::Control::AccessPoint::WpaFlags flags)
{
    m_securityRsnLabel->setText(UiUtils::wpaFlagsToStringList(flags).join("\n"));
}

void ConnectionInfoWirelessTab::clearFields()
{
    m_ssidLabel->setText(QString());
    m_signalStrengthLabel->setText(QString());
    m_bitRateLabel->setText(QString());
    m_accessPointLabel->setText(QString());
    m_securityWpaLabel->setText(QString());
    m_securityRsnLabel->setText(QString());
    m_frequencyLabel->setText(QString());
}
