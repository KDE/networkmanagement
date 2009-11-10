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

#include "connectioninfowiredtab.h"

ConnectionInfoWiredTab::ConnectionInfoWiredTab(Solid::Control::WiredNetworkInterface *iface, QWidget *parent)
    : QWidget(parent), m_iface(iface)
{
    QFormLayout *layout = new QFormLayout(this);
    setLayout(layout);

    m_bitRateLabel = new QLabel(this);
    layout->addRow(i18nc("@label:textbox bit rate of network interface", "Bit rate:"), m_bitRateLabel);

    m_hardwareAddressLabel = new QLabel(this);
    layout->addRow(i18nc("@label:textbox MAC address of network interface", "Hardware address:"), m_hardwareAddressLabel);

    connect(iface, SIGNAL(bitRateChanged(int)), this, SLOT(updateBitRate(int)));

    // populate initial values
    updateBitRate(iface->bitRate());
    m_hardwareAddressLabel->setText(iface->hardwareAddress());
}

void ConnectionInfoWiredTab::updateBitRate(int rate)
{
    m_bitRateLabel->setText(i18nc("@item:intext bit rate value", "%1 Mbit/s", rate / 1000));
}
