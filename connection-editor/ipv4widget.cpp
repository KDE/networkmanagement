/*
    Copyright (c) 2013 Lukas Tinkl <ltinkl@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ipv4widget.h"
#include "ui_ipv4.h"


IPv4Widget::IPv4Widget(NetworkManager::Settings::Setting* setting, QWidget* parent, Qt::WindowFlags f):
    SettingWidget(setting, parent, f),
    m_ui(new Ui::IPv4Widget), m_ipv4Setting(0)
{
    m_ui->setupUi(this);

    if (setting) {
        m_ipv4Setting = static_cast<NetworkManager::Settings::Ipv4Setting *>(setting);
        loadConfig(m_ipv4Setting);
    }

    connect(m_ui->method, SIGNAL(currentIndexChanged(int)),
            SLOT(slotModeComboChanged(int)));
    slotModeComboChanged(m_ui->method->currentIndex());
}

IPv4Widget::~IPv4Widget()
{
}

void IPv4Widget::loadConfig(NetworkManager::Settings::Setting * setting)
{
    Q_UNUSED(setting)

    m_ui->method->setCurrentIndex(static_cast<int>(m_ipv4Setting->method()));

    QStringList tmp;
    foreach (const QHostAddress & addr, m_ipv4Setting->dns()) {
        tmp.append(addr.toString());
    }
    m_ui->dns->setText(tmp.join(","));
    m_ui->dnsSearch->setText(m_ipv4Setting->dnsSearch().join(","));

    m_ui->dhcpClientId->setText(m_ipv4Setting->dhcpClientId());

    // TODO addresses

    // TODO routes

    m_ui->ipv4RequiredCB->setChecked(!m_ipv4Setting->mayFail());
}

QVariantMap IPv4Widget::setting() const
{
    NetworkManager::Settings::Ipv4Setting ipv4Setting;

    ipv4Setting.setMethod(static_cast<NetworkManager::Settings::Ipv4Setting::ConfigMethod>(m_ui->method->currentIndex()));

    if (!m_ui->dns->text().isEmpty()) {
        QStringList tmp = m_ui->dns->text().split(',');
        QList<QHostAddress> tmpAddrList;
        foreach (const QString & str, tmp) {
            QHostAddress addr(str);
            if (!addr.isNull())
                tmpAddrList.append(addr);
        }
        ipv4Setting.setDns(tmpAddrList);
    }
    if (!m_ui->dnsSearch->text().isEmpty()) {
        ipv4Setting.setDnsSearch(m_ui->dnsSearch->text().split(','));
    }

    if (!m_ui->dhcpClientId->text().isEmpty()) {
        ipv4Setting.setDhcpClientId(m_ui->dhcpClientId->text());
    }

    // TODO addresses

    // TODO routes

    ipv4Setting.setMayFail(!m_ui->ipv4RequiredCB->isChecked());

    return ipv4Setting.toMap();
}

void IPv4Widget::slotModeComboChanged(int index)
{
    if (index == 0) {  // Automatic
        m_ui->dns->setEnabled(true);
        m_ui->dnsMorePushButton->setEnabled(true);
        m_ui->dnsSearch->setEnabled(true);
        m_ui->dnsSearchMorePushButton->setEnabled(true);
        m_ui->dhcpClientId->setEnabled(true);
        m_ui->ipv4RequiredCB->setEnabled(true);
        m_ui->btnRoutes->setEnabled(true);

        m_ui->labelAddresses->setVisible(false);
        m_ui->IPTable->setVisible(false);
        m_ui->btnAdd->setVisible(false);
        m_ui->btnDelete->setVisible(false);
    } else if (index == 1) {  // Manual
        m_ui->dns->setEnabled(true);
        m_ui->dnsMorePushButton->setEnabled(true);
        m_ui->dnsSearch->setEnabled(true);
        m_ui->dnsSearchMorePushButton->setEnabled(true);
        m_ui->dhcpClientId->setEnabled(false);
        m_ui->ipv4RequiredCB->setEnabled(true);
        m_ui->btnRoutes->setEnabled(true);

        m_ui->labelAddresses->setVisible(true);
        m_ui->IPTable->setVisible(true);
        m_ui->btnAdd->setVisible(true);
        m_ui->btnDelete->setVisible(true);
    } else if (index == 2 || index == 3) {  // Link-local or Shared
        m_ui->dns->setEnabled(false);
        m_ui->dnsMorePushButton->setEnabled(false);
        m_ui->dnsSearch->setEnabled(false);
        m_ui->dnsSearchMorePushButton->setEnabled(false);
        m_ui->dhcpClientId->setEnabled(false);
        m_ui->ipv4RequiredCB->setEnabled(true);
        m_ui->btnRoutes->setEnabled(false);

        m_ui->labelAddresses->setVisible(false);
        m_ui->IPTable->setVisible(false);
        m_ui->btnAdd->setVisible(false);
        m_ui->btnDelete->setVisible(false);
    } else if (index == 4) {  // Disabled
        m_ui->dns->setEnabled(false);
        m_ui->dnsMorePushButton->setEnabled(false);
        m_ui->dnsSearch->setEnabled(false);
        m_ui->dnsSearchMorePushButton->setEnabled(false);
        m_ui->dhcpClientId->setEnabled(false);
        m_ui->ipv4RequiredCB->setEnabled(false);
        m_ui->btnRoutes->setEnabled(false);

        m_ui->labelAddresses->setVisible(false);
        m_ui->IPTable->setVisible(false);
        m_ui->btnAdd->setVisible(false);
        m_ui->btnDelete->setVisible(false);
    }
}

