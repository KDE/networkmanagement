/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2009-2010 Sebastian Kügler <sebas@kde.org>

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

// Own
#include "hiddenwirelessnetworkitem.h"

// Qt
#include <QGraphicsLinearLayout>

// KDE
#include <KDebug>
#include <KLineEdit>
#include <KLocale>

// Plasma
#include <Plasma/IconWidget>
#include <Plasma/LineEdit>

HiddenWirelessNetworkItem::HiddenWirelessNetworkItem(QGraphicsWidget *parent) : ActivatableItem(0, parent),
    m_layout(0),
    m_connect(0),
    m_ssidEdit(0)
{
    //kDebug() << "HiddenWirelessNetworkItem";
}

HiddenWirelessNetworkItem::~HiddenWirelessNetworkItem()
{
}

void HiddenWirelessNetworkItem::setSsid(const QString & ssid)
{
    m_ssid = ssid;
}

QString HiddenWirelessNetworkItem::ssid() const
{
    return m_ssid;
}

int HiddenWirelessNetworkItem::strength() const
{
    return -1;
}

Solid::Control::AccessPointNm09 * HiddenWirelessNetworkItem::referenceAccessPoint() const
{
    return 0;
}

void HiddenWirelessNetworkItem::setupItem()
{
    if (!m_layout) {
        m_layout = new QGraphicsLinearLayout(this);
        m_connect = new Plasma::IconWidget(this);
        m_connect->setDrawBackground(false);
        m_connect->setOrientation(Qt::Horizontal);
        m_connect->setIcon("network-wireless");
        m_connect->setText(i18nc("label for creating a connection to a hidden wireless network", "<hidden network>"));
        connect(m_connect, SIGNAL(activated()), SLOT(connectClicked()));

        m_ssidEdit = new Plasma::LineEdit(this);
        m_ssidEdit->nativeWidget()->setClickMessage(i18nc("default KLineEdit::clickMessage() for hidden wireless network SSID entry", "Enter network name and press <enter>"));
        m_ssidEdit->setToolTip(i18nc("@info:tooltip for hidden wireless network SSID entry", "Enter network name and press <enter>"));
        connect(m_ssidEdit->nativeWidget(), SIGNAL(returnPressed()), SLOT(ssidEntered()));
    }
    resetSsidEntry();
}

void HiddenWirelessNetworkItem::connectClicked()
{
    m_connect->hide();
    m_ssidEdit->show();

    // OBS: m_ssidEdit->nativeWidget()->setClickMessage(...) has no effect if nativeWidget() has focus.
    m_ssidEdit->setFocus();

    //workarounds for QGraphicsLayout not being able to layout hidden widgets with a 0 size
    m_layout->removeAt(0);
    m_layout->addItem(m_ssidEdit);
}

void HiddenWirelessNetworkItem::ssidEntered()
{
    setSsid(m_ssidEdit->text());
    //kDebug() << "... ssid is now" << m_ssid;

    if (!m_ssid.isEmpty()) {
        emitClicked();
        emit connectToHiddenNetwork(m_ssid);
    }

    resetSsidEntry();
}

void HiddenWirelessNetworkItem::resetSsidEntry()
{
    m_ssidEdit->nativeWidget()->clearFocus();
    m_ssidEdit->nativeWidget()->clear();
    m_ssidEdit->hide();
    m_connect->show();
    //workarounds for QGraphicsLayout not being able to layout hidden widgets with a 0 size
    m_layout->removeAt(0);
    m_layout->addItem(m_connect);
}

// vim: sw=4 sts=4 et tw=100
