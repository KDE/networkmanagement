/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#include "traysettingswidget.h"

#include <QListWidget>

#include <KDebug>
#include <KIcon>
#include <KLocale>

#include <solid/control/networkinterface.h>

#include "knmserviceprefs.h"

static const int IconInterfaceRole = 20118;

class IconListWidgetItem : public QListWidgetItem
{
public:
    IconListWidgetItem(QListWidget * parent, uint visibleNumber)
    : QListWidgetItem(parent)
    {
        setText(i18nc("@item:inlistbox", "Icon %1", QString::number(visibleNumber)));
        setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
        setIcon(KIcon("networkmanager"));
        setData(IconInterfaceRole, Solid::Control::NetworkInterface::UnknownType);
    }
};

TraySettingsWidget::TraySettingsWidget(QWidget * parent)
: QWidget(parent), m_iconCount(0)
{
    m_ui.setupUi(this);

    kDebug() << m_iconCount;
    m_iconCount = KNetworkManagerServicePrefs::self()->iconCount();
    kDebug() << m_iconCount;

    for (uint i = 0; (i < m_iconCount && i < 5); ++i) {

        IconListWidgetItem * item = new IconListWidgetItem(m_ui.iconInterfaceList, i+1);
        if (i == 0) {
            m_firstIconItem = item;
        }

        readIconTypes(i, m_ui.iconInterfaceList);
    }

    m_ui.pbRemoveIcon->setEnabled(m_iconCount > 1);

    connect(m_ui.pbAddIcon, SIGNAL(clicked()), SLOT(addIconClicked()));
    connect(m_ui.pbRemoveIcon, SIGNAL(clicked()), SLOT(removeIconClicked()));
    connect(m_ui.iconInterfaceList->model(), SIGNAL( rowsInserted ( const QModelIndex &, int, int)), SLOT(itemsMoved()));
}

TraySettingsWidget::~TraySettingsWidget()
{

}

QList<uint> TraySettingsWidget::iconInterfaceAllocations() const
{
    // iterate the list and OR together the interface types grouped under each icon, return
    // as a list of uints.
    // Ignore any icons that are empty (have no interface types before the end of the list or the
    // next icon.
    QList<uint> allocations;
    for (int i = 0; i < m_ui.iconInterfaceList->count(); ++i) {
        // is the item an icon or an interface type
        QListWidgetItem * item = m_ui.iconInterfaceList->item(i);
        if (item->data(IconInterfaceRole).toUInt() == Solid::Control::NetworkInterface::UnknownType) {
            // start ORing together the following interface types until we reach the end or
            // another icon
            int allocation = 0;
            QListWidgetItem * iconItem = 0;
            while (++i < m_ui.iconInterfaceList->count()) {
                iconItem = m_ui.iconInterfaceList->item(i);
                if (iconItem->data(IconInterfaceRole).toUInt() == Solid::Control::NetworkInterface::UnknownType) {
                    // we peeked the next icon, but i will be incremented at the end of the outer
                    // loop so decrement it now
                    --i;
                    break; // back to the outer loop
                } else { // interface type, add to this allocation
                    allocation |= iconItem->data(IconInterfaceRole).toUInt();
                }
            }

            // save any allocation we made in the inner loop
            if (allocation != 0) { // don't save empty sets
                allocations.append(allocation);
            }
        }
    }
    kDebug() << allocations;
    return allocations;
}

void TraySettingsWidget::readIconTypes(uint index, QListWidget* list)
{
    Solid::Control::NetworkInterface::Types iconTypes(KNetworkManagerServicePrefs::self()->iconTypes(index));
    QListWidgetItem * childItem;
    if (iconTypes.testFlag(Solid::Control::NetworkInterface::Ieee8023)) {
        childItem = new QListWidgetItem(i18nc("@item:inlistbox", "Wired network interfaces"), list);
        childItem->setData(IconInterfaceRole, Solid::Control::NetworkInterface::Ieee8023);
    }
    if (iconTypes.testFlag(Solid::Control::NetworkInterface::Ieee80211)) {
        childItem = new QListWidgetItem(i18nc("@item:inlistbox", "Wireless network interfaces"), list);
        childItem->setData(IconInterfaceRole, Solid::Control::NetworkInterface::Ieee80211);
    }
    if (iconTypes.testFlag(Solid::Control::NetworkInterface::Serial)) {
        childItem = new QListWidgetItem(i18nc("@item:inlistbox", "DSL network devices"), list);
        childItem->setData(IconInterfaceRole, Solid::Control::NetworkInterface::Serial);
    }
    if (iconTypes.testFlag(Solid::Control::NetworkInterface::Gsm)) {
        childItem = new QListWidgetItem(i18nc("@item:inlistbox", "GSM network devices"), list);
        childItem->setData(IconInterfaceRole, Solid::Control::NetworkInterface::Gsm);
    }
    if (iconTypes.testFlag(Solid::Control::NetworkInterface::Cdma)) {
        childItem = new QListWidgetItem(i18nc("@item:inlistbox", "CDMA network devices"), list);
        childItem->setData(IconInterfaceRole, Solid::Control::NetworkInterface::Cdma);
    }

    if (childItem)
        childItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsDragEnabled);
}


void TraySettingsWidget::addIconClicked()
{
    if (m_iconCount < 5) {
        new IconListWidgetItem(m_ui.iconInterfaceList, ++m_iconCount);
        m_ui.pbRemoveIcon->setEnabled(true);
    } else {
        m_ui.pbAddIcon->setEnabled(false);
    }
    emit changed();
}

void TraySettingsWidget::removeIconClicked()
{
    //iterate the items backwards and remove the first icon item found as long as it is not the
    //first one.
    for (int i = m_ui.iconInterfaceList->count() - 1; i >= 0; --i) {
        QListWidgetItem * item = m_ui.iconInterfaceList->item(i);
        if (item) {
            if (item->data(IconInterfaceRole).toUInt() == Solid::Control::NetworkInterface::UnknownType && item != m_firstIconItem) {
                m_ui.iconInterfaceList->takeItem(i);
                --m_iconCount;
                m_ui.pbAddIcon->setEnabled(true);
                break;
            }
        }
    }
    if (m_iconCount == 1) {
        // removed the penultimate icon item, don't take any more
        m_ui.pbRemoveIcon->setEnabled(false);
    }
    emit changed();
}


void TraySettingsWidget::itemsMoved()
{
    emit changed();
}
// vim: sw=4 sts=4 et tw=100
