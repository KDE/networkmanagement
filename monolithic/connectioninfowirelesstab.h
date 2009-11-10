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
#ifndef CONNECTIONINFOWIRELESSTAB_H
#define CONNECTIONINFOWIRELESSTAB_H

#include <QtGui>

#include <solid/control/networkinterface.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirelessaccesspoint.h>

/**
 * Tab for ConnectionInfoDialog, that shows Wi-Fi details
 */
class ConnectionInfoWirelessTab : public QWidget
{
    Q_OBJECT

public:
    ConnectionInfoWirelessTab(Solid::Control::WirelessNetworkInterface *iface, QWidget *parent = 0);
    ~ConnectionInfoWirelessTab() { };

public slots:
    void updateAccessPoint(const QString &uni);
    void updateBitRate(int rate);
    void updateOperationMode(Solid::Control::WirelessNetworkInterface::OperationMode mode);
    void updateSsid(const QString& ssid);
    void updateSignalStrength(int signalStrength);
    void updateMaxBitRate(int bitRate);
    void updateFrequency(uint frequency);
    void updateWpa(Solid::Control::AccessPoint::WpaFlags flags);
    void updateRsn(Solid::Control::AccessPoint::WpaFlags flags);
    void clearFields();
    
private:
    Solid::Control::WirelessNetworkInterface *m_iface;
    Solid::Control::AccessPoint *m_ap;
    QLabel *m_ssidLabel,
           *m_signalStrengthLabel,
           *m_operationModeLabel,
           *m_bitRateLabel,
           *m_accessPointLabel,
           *m_securityWpaLabel,
           *m_securityRsnLabel,
           *m_frequencyLabel,
           *m_hardwareAddressLabel;
    int m_maxBitRate;
 };

#endif // CONNECTIONINFOWIRELESSTAB_H

