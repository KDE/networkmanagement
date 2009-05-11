/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#include "hiddenwirelessnetworkitem.h"

#include <QGraphicsLinearLayout>

#include <KLineEdit>
#include <KLocale>
#include <Plasma/IconWidget>
#include <Plasma/LineEdit>

HiddenWirelessNetwork::HiddenWirelessNetwork(QObject * parent) : AbstractWirelessNetwork(parent)
{

}

HiddenWirelessNetwork::~HiddenWirelessNetwork()
{

}


void HiddenWirelessNetwork::setSsid(const QString & ssid)
{
    m_ssid = ssid;
}

QString HiddenWirelessNetwork::ssid() const
{
    return m_ssid;
}

int HiddenWirelessNetwork::strength() const
{
    return -1;
}

Solid::Control::AccessPoint * HiddenWirelessNetwork::referenceAccessPoint() const
{
    return 0;
}

HiddenWirelessNetworkItem::HiddenWirelessNetworkItem(QGraphicsItem * parent): m_connect(0), m_ssidEdit(0), m_layout(0)
{
    m_wirelessNetwork = new HiddenWirelessNetwork(this);
}

HiddenWirelessNetworkItem::~HiddenWirelessNetworkItem()
{
}

void HiddenWirelessNetworkItem::setupItem()
{
    if (!m_layout) {
        m_layout = new QGraphicsLinearLayout(this);
    }
    if (!m_connect) {
        m_connect = new Plasma::IconWidget(this);
        m_connect->setText(i18nc("label for creating a connection to a hidden wireless network", "Connect to hidden network"));
        m_layout->addItem(m_connect);
        connect(m_connect, SIGNAL(activated()), SLOT(connectClicked()));
    }
}

void HiddenWirelessNetworkItem::connectClicked()
{
    delete m_connect;
    m_connect = 0;
    if (!m_ssidEdit) {
        m_ssidEdit = new Plasma::LineEdit(this);
        m_ssidEdit->nativeWidget()->setClickMessage(i18nc("default KLineEdit::clickMessage() for hidden wireless network SSID entry", "Enter hidden SSID and press <enter>"));
        m_layout->addItem(m_ssidEdit);
        connect(m_ssidEdit->nativeWidget(), SIGNAL(returnPressed()), SLOT(ssidEntered()));
    }
}

void HiddenWirelessNetworkItem::ssidEntered()
{
    qobject_cast<HiddenWirelessNetwork*>(m_wirelessNetwork)->setSsid(m_ssidEdit->text());
    emitClicked();
}

void HiddenWirelessNetworkItem::resetSsidEntry()
{
    delete m_ssidEdit;
    m_ssidEdit = 0;
    setupItem();
}

// vim: sw=4 sts=4 et tw=100
