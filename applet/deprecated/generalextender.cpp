/***************************************************************************
 *   Copyright 2009 by Sebastian Kügler <sebas@kde.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

//Qt
#include <QGraphicsLinearLayout>

//KDE
#include <KDebug>
#include <KColorScheme>
#include <KIconLoader>

//#include <solid/control/networkinterface.h>
#include <solid/control/networkmanager.h>

//plasma
#include <Plasma/IconWidget>
#include <Plasma/Extender>


//own
#include "generalextender.h"


GeneralExtender::GeneralExtender(Plasma::Extender *ext)
    : Plasma::ExtenderItem(ext),
      m_widget(0),
      m_layout(0),
      m_rfCheckBox(0),
      m_connectionsButton(0)
{
    setTitle(i18nc("title of general settings extender ", "General Settings"));
    setName("general");
    setIcon("networkmanager");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    (void)graphicsWidget();
}

GeneralExtender::~GeneralExtender()
{
}

QGraphicsWidget* GeneralExtender::graphicsWidget()
{
    // Main widget and layout
    if (m_widget) {
        return m_widget;
        kDebug() << "widget is there, shortcutting";
    }

    m_widget = new QGraphicsWidget(this);

    m_layout = new QGraphicsLinearLayout(m_widget);
    m_layout->setOrientation(Qt::Vertical);
    m_layout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_rfCheckBox = new Plasma::CheckBox(this);
    m_rfCheckBox->setChecked(Solid::Control::NetworkManager::isWirelessEnabled());
    m_rfCheckBox->setEnabled(Solid::Control::NetworkManager::isWirelessHardwareEnabled());
    m_rfCheckBox->setText(i18nc("CheckBox to enable or disable wireless interface (rfkill)", "Enable wireless"));
    m_layout->addItem(m_rfCheckBox);
    connect(m_rfCheckBox, SIGNAL(toggled(bool)),
            SLOT(wirelessEnabledToggled(bool)));
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(wirelessEnabledChanged(bool)),
            this, SLOT(managerWirelessEnabledChanged(bool)));
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(wirelessHardwareEnabledChanged(bool)),
            this, SLOT(managerWirelessHardwareEnabledChanged(bool)));

    m_connectionsButton = new Plasma::IconWidget(m_widget);
    m_connectionsButton->setIcon("networkmanager");
    m_connectionsButton->setOrientation(Qt::Horizontal);
    m_connectionsButton->setText(i18nc("button in general settings extender", "Manage Connections..."));
    m_connectionsButton->setMaximumHeight(KIconLoader::SizeMedium);
    m_connectionsButton->setMinimumHeight(KIconLoader::SizeMedium);
    m_connectionsButton->setDrawBackground(true);
#if KDE_IS_VERSION(4,2,60)
    m_connectionsButton->setTextBackgroundColor(QColor());
#endif
    connect(m_connectionsButton, SIGNAL(activated()), this, SLOT(manageConnections()));
    m_layout->addItem(m_connectionsButton);

    m_widget->setLayout(m_layout);
    setWidget(m_widget);
    kDebug() << "widget is there, shortcutting";
    return m_widget;
}

void GeneralExtender::wirelessEnabledToggled(bool checked)
{
    kDebug() << "Applet wireless enable switch toggled" << checked;
    Solid::Control::NetworkManager::setWirelessEnabled(checked);
}

void GeneralExtender::managerWirelessEnabledChanged(bool enabled)
{
    kDebug() << "NM daemon changed wireless enable state" << enabled;
    // it might have changed because we toggled the switch,
    // but it might have been changed externally, so set it anyway
    m_rfCheckBox->setChecked(enabled);
}

void GeneralExtender::managerWirelessHardwareEnabledChanged(bool enabled)
{
    kDebug() << "Hardware wireless enable switch state changed" << enabled;
    m_rfCheckBox->setChecked(enabled && Solid::Control::NetworkManager::isWirelessEnabled());
    m_rfCheckBox->setEnabled(!enabled);
}

void GeneralExtender::manageConnections()
{
    //kDebug() << "opening connection management dialog";
    QStringList args;
    args << "kcm_networkmanagement";
    KToolInvocation::kdeinitExec("kcmshell4", args);
}


#include "generalextender.moc"
