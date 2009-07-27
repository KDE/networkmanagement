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

#include <QTreeWidget>

#include <KDebug>
#include <KLocale>

#include <solid/control/networkinterface.h>

#include "knmserviceprefs.h"

class IconTreeWidgetItem : public QTreeWidgetItem
{
public:
    IconTreeWidgetItem(QTreeWidget * parent, uint visibleNumber)
    : QTreeWidgetItem(parent)
    {
        setText(0, i18nc("@item:inlistbox", "Icon %1", QString::number(visibleNumber)));
        setExpanded(true);
        setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
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

        IconTreeWidgetItem * item = new IconTreeWidgetItem(m_ui.iconInterfaceTree, i+1);

        readIconTypes(i, item);
    }

    connect(m_ui.pbAddIcon, SIGNAL(clicked()), SLOT(addIconClicked()));
}

TraySettingsWidget::~TraySettingsWidget()
{

}

void TraySettingsWidget::readIconTypes(uint index, QTreeWidgetItem * item)
{
    Solid::Control::NetworkInterface::Types iconTypes(KNetworkManagerServicePrefs::self()->iconTypes(index));
    QTreeWidgetItem * childItem;
    if (iconTypes.testFlag(Solid::Control::NetworkInterface::Ieee8023)) {
        childItem = new QTreeWidgetItem(item, QStringList(i18nc("@item:inlistbox", "Wired network interfaces")));
    }
    if (iconTypes.testFlag(Solid::Control::NetworkInterface::Ieee80211)) {
        childItem = new QTreeWidgetItem(item, QStringList(i18nc("@item:inlistbox", "Wireless network interfaces")));
    }
    if (iconTypes.testFlag(Solid::Control::NetworkInterface::Serial)) {
        childItem = new QTreeWidgetItem(item, QStringList(i18nc("@item:inlistbox", "DSL network devices")));
    }
    if (iconTypes.testFlag(Solid::Control::NetworkInterface::Gsm)) {
        childItem = new QTreeWidgetItem(item, QStringList(i18nc("@item:inlistbox", "GSM network devices")));
    }
    if (iconTypes.testFlag(Solid::Control::NetworkInterface::Cdma)) {
        childItem = new QTreeWidgetItem(item, QStringList(i18nc("@item:inlistbox", "CDMA network devices")));
    }

    if (childItem)
        childItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsDragEnabled);
}


void TraySettingsWidget::addIconClicked()
{
    if (m_iconCount < 4) {
        new IconTreeWidgetItem(m_ui.iconInterfaceTree, ++m_iconCount);
    } else {
        m_ui.pbAddIcon->setEnabled(false);
    }
}

void TraySettingsWidget::removeIconClicked()
{

}

// vim: sw=4 sts=4 et tw=100
