/*
Copyright 2008 Sebastian Kügler <sebas@kde.org>

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
#include "activatablelistwidget.h"

// Qt
#include <QGraphicsLinearLayout>

// KDE
#include <KDebug>

// Plasma
#include <Plasma/Label>
#include <Plasma/IconWidget>

// networkmanagement lib
#include "remoteactivatable.h"
#include "remoteactivatablelist.h"
#include "remoteinterfaceconnection.h"
#include "remotewirelessnetwork.h"
#include "activatableitem.h"

// networkmanagement applet
#include "interfaceconnectionitem.h"
#include "wirelessnetworkitem.h"

ActivatableListWidget::ActivatableListWidget(RemoteActivatableList* activatables, Solid::Control::NetworkInterface* iface, QGraphicsWidget* parent) : Plasma::ScrollWidget(parent),
    m_activatables(activatables),
    m_iface(iface),
    m_layout(0)
{
    m_widget = new QGraphicsWidget(this);
    m_widget->setMinimumSize(240, 50);
    m_layout = new QGraphicsLinearLayout(m_widget);
    m_layout->setOrientation(Qt::Vertical);
    m_layout->setSpacing(0);
    m_widget->setLayout(m_layout);
    setWidget(m_widget);

    listAppeared();
}

void ActivatableListWidget::init()
{
    connect(m_activatables, SIGNAL(activatableAdded(RemoteActivatable*)),
            SLOT(activatableAdded(RemoteActivatable *)));
    connect(m_activatables, SIGNAL(activatableRemoved(RemoteActivatable*)),
                    SLOT(activatableRemoved(RemoteActivatable *)));

    connect(m_activatables, SIGNAL(appeared()), SLOT(listAppeared()));
    connect(m_activatables, SIGNAL(disappeared()), SLOT(listDisappeared()));
}

ActivatableListWidget::~ActivatableListWidget()
{
}

bool ActivatableListWidget::accept(RemoteActivatable * activatable) const
{
    bool acceptable = false;

    Knm::Activatable::ActivatableType aType = activatable->activatableType();
    kDebug() << activatable << aType;
    if (aType == Knm::Activatable::InterfaceConnection) {
        RemoteInterfaceConnection * ric = static_cast<RemoteInterfaceConnection*>(activatable);
        if (ric->connectionType() == Knm::Connection::Wired && m_iface->type() == Solid::Control::NetworkInterface::Ieee8023) {
            acceptable = true;
        } else if (ric->connectionType() == Knm::Connection::Gsm && m_iface->type() == Solid::Control::NetworkInterface::Gsm) {
            acceptable = true;
        } else if (ric->connectionType() == Knm::Connection::Cdma && m_iface->type() == Solid::Control::NetworkInterface::Cdma) {
            acceptable = true;
        } else if (ric->connectionType() == Knm::Connection::Pppoe && m_iface->type() == Solid::Control::NetworkInterface::Serial) {
            acceptable = true;
        }
    } else if (aType == Knm::Activatable::WirelessInterfaceConnection && m_iface->type() == Solid::Control::NetworkInterface::Ieee80211) {
        kDebug() << "accepting wireless connection";
        acceptable = true;
    } else if (aType == Knm::Activatable::WirelessNetwork && m_iface->type() == Solid::Control::NetworkInterface::Ieee80211) {
        kDebug() << "accepting wireless network item";
        acceptable = true;
    }
    return acceptable;
}


ActivatableItem * ActivatableListWidget::createItem(RemoteActivatable * activatable)
{
    kDebug() << "Creating Item";
    ActivatableItem* ai = 0;
    switch (m_iface->type()) {
        case Solid::Control::NetworkInterface::Ieee80211:
        { // Wireless
            kDebug() << "Wireless thingie";
            WirelessNetworkItem* wni = new WirelessNetworkItem(static_cast<RemoteWirelessNetwork*>(activatable), m_widget);
            ai = wni;
            break;
        }
        default:
        case Solid::Control::NetworkInterface::Ieee8023:
        {
            kDebug() << "default ....";
            ai = new InterfaceConnectionItem(static_cast<RemoteInterfaceConnection*>(activatable), m_widget);
            break;
        }
    }

    Q_ASSERT(ai);
    ai->setupItem();
    m_layout->addItem(ai);
    m_itemIndex[activatable->deviceUni()] = ai;
    m_layout->invalidate();
    return ai;
}

void ActivatableListWidget::listAppeared()
{
    foreach (RemoteActivatable* remote, m_activatables->activatables()) {
        kDebug() << "FOREACH";
        if (accept(remote)) {
            kDebug() << "accept";
            createItem(remote);
        }
    }
#if 0
    foreach (QString bla, QStringList() << "bla" << "foo" <<  "gna" << "snirk"<< "bla3" << "foo3" <<  "gna3" << "snirk3" << "bla" << "foo" <<  "gna" << "snirk"<< "bla3" << "foo3" <<  "gna3" << "snirk3") {
        Plasma::IconWidget* l = new Plasma::IconWidget(m_widget);
        l->setOrientation(Qt::Horizontal);
        l->setMinimumSize(200, 32);
        l->setIcon("network-wired");
        l->setText(QString("dummy network connection %1").arg(bla));
        m_layout->addItem(l);
    }
#endif
}

void ActivatableListWidget::listDisappeared()
{
    foreach (ActivatableItem* item, m_itemIndex) {
        kDebug() << "deleting";
        m_layout->removeItem(item);
    }
    m_itemIndex.clear();
    kDebug() << "Cleared up";
}

/*
bool ActivatableListWidget::registerActivatable(RemoteActivatable * activatable)
{

    bool changed = false;
    / *
    if (!m_connections.contains(activatable)) {
   // let subclass decide
   if (accept(activatable)) {
   kDebug() << "adding activatable";
   ActivatableItem * ci = createItem(activatable);

   m_connections.insert(activatable, ci);
   m_connectionLayout->addItem(ci);
   m_connectionLayout->invalidate();
   m_layout->invalidate();
   changed = true;
   }
   }
   * /
   return changed;
   }
   */
void ActivatableListWidget::activatableAdded(RemoteActivatable * added)
{
    kDebug();
    /*
    if (registerActivatable(added)) {
   emit connectionListUpdated();
}
*/
}

void ActivatableListWidget::activatableRemoved(RemoteActivatable * removed)
{
    kDebug();
    /*
    // look up the ActivatableItem and remove it
    if (m_connections.contains(removed)) {
   ActivatableItem * item = m_connections.value(removed);
   m_connectionLayout->removeItem(item);
   m_connections.remove(removed);
   delete item;
   kDebug();
   emit connectionListUpdated();
}
*/
}

// vim: sw=4 sts=4 et tw=100
