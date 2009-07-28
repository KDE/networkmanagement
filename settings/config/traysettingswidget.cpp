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

class InterfaceTypeListWidgetItem : public QListWidgetItem
{
public:
    InterfaceTypeListWidgetItem(Solid::Control::NetworkInterface::Type type, QListWidget * parent = 0)
        : QListWidgetItem(parent)
    {
        switch (type) {
            case Solid::Control::NetworkInterface::Ieee8023:
                setText(i18nc("@item:inlistbox", "Wired network interfaces"));
                setData(IconInterfaceRole, Solid::Control::NetworkInterface::Ieee8023);
                break;
            case Solid::Control::NetworkInterface::Ieee80211:
                setText(i18nc("@item:inlistbox", "Wireless network interfaces"));
                setData(IconInterfaceRole, Solid::Control::NetworkInterface::Ieee80211);
                break;
            case Solid::Control::NetworkInterface::Serial:
                setText(i18nc("@item:inlistbox", "DSL network devices"));
                setData(IconInterfaceRole, Solid::Control::NetworkInterface::Serial);
                break;
            case Solid::Control::NetworkInterface::Gsm:
                setText(i18nc("@item:inlistbox", "GSM network devices"));
                setData(IconInterfaceRole, Solid::Control::NetworkInterface::Gsm);
                break;
            case Solid::Control::NetworkInterface::Cdma:
                setText(i18nc("@item:inlistbox", "CDMA network devices"));
                setData(IconInterfaceRole, Solid::Control::NetworkInterface::Cdma);
                break;
        }
        setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsDragEnabled);
    }
};

class TraySettingsWidgetPrivate
{
public:
    TraySettingsWidgetPrivate()
        : iconCount(0), displayedTypes(0)
    { }
    Ui_TraySettings ui;
    uint iconCount;
    QListWidgetItem * firstIconItem;
    Solid::Control::NetworkInterface::Types displayedTypes;
};

TraySettingsWidget::TraySettingsWidget(QWidget * parent)
: QWidget(parent), d_ptr(new TraySettingsWidgetPrivate)
{
    Q_D(TraySettingsWidget);
    d->ui.setupUi(this);
kDebug() << d->iconCount;
    d->iconCount = KNetworkManagerServicePrefs::self()->iconCount();
    kDebug() << d->iconCount;

    for (uint i = 0; (i < d->iconCount && i < 5); ++i) {

        IconListWidgetItem * item = new IconListWidgetItem(d->ui.iconInterfaceList, i+1);
        if (i == 0) {
            d->firstIconItem = item;
        }

        readIconTypes(i);
    }
    // finally add any interface types that user has configured to not be shown
    QList<Solid::Control::NetworkInterface::Type> allTypes;
    allTypes << Solid::Control::NetworkInterface::Ieee8023 << Solid::Control::NetworkInterface::Ieee80211 << Solid::Control::NetworkInterface::Serial << Solid::Control::NetworkInterface::Gsm << Solid::Control::NetworkInterface::Cdma;
    foreach (Solid::Control::NetworkInterface::Type type, allTypes) {
        if (!d->displayedTypes.testFlag(type)) {
            InterfaceTypeListWidgetItem * item =  new InterfaceTypeListWidgetItem(type);
            d->ui.iconInterfaceList->insertItem(0, item);
        }
    }

    d->ui.pbRemoveIcon->setEnabled(d->iconCount > 1);

    connect(d->ui.pbAddIcon, SIGNAL(clicked()), SLOT(addIconClicked()));
    connect(d->ui.pbRemoveIcon, SIGNAL(clicked()), SLOT(removeIconClicked()));
    connect(d->ui.iconInterfaceList->model(), SIGNAL( rowsInserted ( const QModelIndex &, int, int)), SLOT(itemsMoved()));
}

TraySettingsWidget::~TraySettingsWidget()
{

}

QList<uint> TraySettingsWidget::iconInterfaceAllocations() const
{
    Q_D(const TraySettingsWidget);
    // iterate the list and OR together the interface types grouped under each icon, return
    // as a list of uints.
    // Ignore any icons that are empty (have no interface types before the end of the list or the
    // next icon.
    QList<uint> allocations;
    for (int i = 0; i < d->ui.iconInterfaceList->count(); ++i) {
        // is the item an icon or an interface type
        QListWidgetItem * item = d->ui.iconInterfaceList->item(i);
        if (item->data(IconInterfaceRole).toUInt() == Solid::Control::NetworkInterface::UnknownType) {
            // start ORing together the following interface types until we reach the end or
            // another icon
            int allocation = 0;
            QListWidgetItem * iconItem = 0;
            while (++i < d->ui.iconInterfaceList->count()) {
                iconItem = d->ui.iconInterfaceList->item(i);
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

void TraySettingsWidget::readIconTypes(uint index)
{
    Q_D(TraySettingsWidget);
    Solid::Control::NetworkInterface::Types iconTypes(KNetworkManagerServicePrefs::self()->iconTypes(index));

    QList<Solid::Control::NetworkInterface::Type> allTypes;

    allTypes << Solid::Control::NetworkInterface::Ieee8023 << Solid::Control::NetworkInterface::Ieee80211 << Solid::Control::NetworkInterface::Serial << Solid::Control::NetworkInterface::Gsm << Solid::Control::NetworkInterface::Cdma;

    foreach (Solid::Control::NetworkInterface::Type type, allTypes) {
        if (iconTypes.testFlag(type)) {
            new InterfaceTypeListWidgetItem(type, d->ui.iconInterfaceList);
        }
    }
}


void TraySettingsWidget::addIconClicked()
{
    Q_D(TraySettingsWidget);
    if (d->iconCount < 5) {
        new IconListWidgetItem(d->ui.iconInterfaceList, ++d->iconCount);
        d->ui.pbRemoveIcon->setEnabled(true);
    } else {
        d->ui.pbAddIcon->setEnabled(false);
    }
    emit changed();
}

void TraySettingsWidget::removeIconClicked()
{
    Q_D(TraySettingsWidget);
    //iterate the items backwards and remove the first icon item found as long as it is not the
    //first one.
    for (int i = d->ui.iconInterfaceList->count() - 1; i >= 0; --i) {
        QListWidgetItem * item = d->ui.iconInterfaceList->item(i);
        if (item) {
            if (item->data(IconInterfaceRole).toUInt() == Solid::Control::NetworkInterface::UnknownType && item != d->firstIconItem) {
                d->ui.iconInterfaceList->takeItem(i);
                --d->iconCount;
                d->ui.pbAddIcon->setEnabled(true);
                break;
            }
        }
    }
    if (d->iconCount == 1) {
        // removed the penultimate icon item, don't take any more
        d->ui.pbRemoveIcon->setEnabled(false);
    }
    emit changed();
}


void TraySettingsWidget::itemsMoved()
{
    emit changed();
}
// vim: sw=4 sts=4 et tw=100
