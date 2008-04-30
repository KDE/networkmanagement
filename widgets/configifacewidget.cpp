/*  This file is part of the KDE project
    Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "configifacewidget.h"

#include <QHBoxLayout>

ConfigIfaceWidget::ConfigIfaceWidget(QWidget *parent)
    : QWidget(parent),
      m_config(0),
      m_ipGroupBox(0),
      m_mainLayout(0),
      m_ipLayout(0),
      m_ipGroup(0),
      m_dynamicButton(0),
      m_staticButton(0),
      m_ipLabel(0),
      m_netmaskLabel(0),
      m_broadcastLabel(0),
      m_ipAddressEdit(0),
      m_netmaskEdit(0),
      m_broadcastEdit(0)
{
}

ConfigIfaceWidget::~ConfigIfaceWidget()
{
    delete m_dynamicButton;
    delete m_staticButton;
    delete m_ipLabel;
    delete m_netmaskLabel;
    delete m_broadcastLabel;
    delete m_ipAddressEdit;
    delete m_netmaskEdit;
    delete m_broadcastEdit;
    delete m_mainLayout;
    delete m_ipLayout;
    delete m_ipGroup;
    delete m_ipGroupBox;
}

void ConfigIfaceWidget::layoutIpWidget()
{
    m_ipGroupBox = new QGroupBox(i18n("IP Settings"));
    m_ipLayout = new QVBoxLayout();
    
    m_ipGroup = new QButtonGroup(this);
    m_dynamicButton = new QRadioButton(i18n("Dynamic IP Configuration"), this);
    m_staticButton = new QRadioButton(i18n("Static IP Configuration"), this);
    m_ipGroup->addButton(m_dynamicButton);
    m_ipGroup->addButton(m_staticButton);
    m_dynamicButton->setChecked(true);

    QHBoxLayout *ipLabelLayout = new QHBoxLayout();
    m_ipAddressEdit = new QLineEdit(this);
    m_ipLabel = new QLabel(i18n("IP:"), this);
    m_ipLabel->setBuddy(m_ipAddressEdit);
    ipLabelLayout->addWidget(m_ipLabel);
    ipLabelLayout->addWidget(m_ipAddressEdit);

    QHBoxLayout *netmaskLabelLayout = new QHBoxLayout();
    m_netmaskEdit = new QLineEdit(this);
    m_netmaskLabel = new QLabel(i18n("Netmask:"), this);
    m_netmaskLabel->setBuddy(m_netmaskLabel);
    netmaskLabelLayout->addWidget(m_netmaskLabel);
    netmaskLabelLayout->addWidget(m_netmaskEdit);

    QHBoxLayout *broadcastLabelLayout = new QHBoxLayout();
    m_broadcastEdit = new QLineEdit(this);
    m_broadcastLabel = new QLabel(i18n("Broadcast"), this);
    m_broadcastLabel->setBuddy(m_broadcastEdit);
    broadcastLabelLayout->addWidget(m_broadcastLabel);
    broadcastLabelLayout->addWidget(m_broadcastEdit);

    //layout items
    m_ipLayout->addWidget(m_dynamicButton);
    m_ipLayout->addWidget(m_staticButton);
    m_ipLayout->addLayout(ipLabelLayout);
    m_ipLayout->addLayout(netmaskLabelLayout);
    m_ipLayout->addLayout(broadcastLabelLayout);

    m_ipGroupBox->setLayout(m_ipLayout);

    //ghost out static IP items
    enableStaticItems(false);

    connect(m_dynamicButton, SIGNAL(clicked()), this, SLOT(onDynamicClicked()));
    connect(m_staticButton, SIGNAL(clicked()), this, SLOT(onStaticClicked()));
}

KConfig* ConfigIfaceWidget::config() const
{
    return m_config;
}

void ConfigIfaceWidget::setConfig(KConfig *config)
{
    m_config = config;
}

void ConfigIfaceWidget::onDynamicClicked()
{
    enableStaticItems(false);
}

void ConfigIfaceWidget::onStaticClicked()
{
    enableStaticItems();
}

void ConfigIfaceWidget::enableStaticItems(bool enable)
{
    //enable or un-enable all the items
    m_ipLabel->setEnabled(enable);
    m_ipAddressEdit->setEnabled(enable);
    m_netmaskLabel->setEnabled(enable);
    m_netmaskEdit->setEnabled(enable);
    m_broadcastLabel->setEnabled(enable);
    m_broadcastEdit->setEnabled(enable);
}

WiredConfigIfaceWidget::WiredConfigIfaceWidget(QWidget *parent)
    : ConfigIfaceWidget(parent)
{
    layoutIpWidget();
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->addWidget(m_ipGroupBox);

    setLayout(m_mainLayout);
}

WiredConfigIfaceWidget::~WiredConfigIfaceWidget()
{
    delete m_mainLayout;
}

ConfigIfaceWidget::Type WiredConfigIfaceWidget::ifaceType() const
{
    return Ieee8023;
}

WifiConfigIfaceWidget::WifiConfigIfaceWidget(QWidget *parent)
    : ConfigIfaceWidget(parent),
      m_wifiGroupBox(0),
      m_wifiLayout(0),
      m_essidLayout(0),
      m_wifiGroup(0),
      m_anyButton(0),
      m_specificButton(0),
      m_essidLabel(0),
      m_essidEdit(0),
      m_scanButton(0),
      m_scandlg(0),
      m_scanView(0),
      m_scanModel(0),
      m_scanDelegate(0),
      m_scanSelectionModel(0)
{
    layoutIpWidget();

    m_wifiGroupBox = new QGroupBox(i18n("Wifi Settings"));
    m_wifiGroup = new QButtonGroup(this);
    m_anyButton = new QRadioButton(i18n("Connect to any open access point."), this);
    m_specificButton = new QRadioButton(i18n("Connect to a specific access point."), this);
    m_wifiGroup->addButton(m_anyButton);
    m_wifiGroup->addButton(m_specificButton);
    m_anyButton->setChecked(true);

    m_essidLayout = new QHBoxLayout();
    m_essidEdit = new QLineEdit(this);
    m_essidLabel = new QLabel(i18n("ESSID:"), this);
    m_scanButton = new QPushButton(i18n("Scan"), this);
    m_essidLabel->setBuddy(m_essidEdit);
    m_essidLayout->addWidget(m_essidLabel);
    m_essidLayout->addWidget(m_essidEdit);
    m_essidLayout->addWidget(m_scanButton);

    m_wifiLayout = new QVBoxLayout();
    m_wifiLayout->addWidget(m_anyButton);
    m_wifiLayout->addWidget(m_specificButton);
    m_wifiLayout->addLayout(m_essidLayout);

    m_wifiGroupBox->setLayout(m_wifiLayout);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->addWidget(m_wifiGroupBox);
    m_mainLayout->addWidget(m_ipGroupBox);

    setLayout(m_mainLayout);

    enableScanningItems(false);
    connect(m_anyButton, SIGNAL(clicked()), this, SLOT(onAnyButtonClicked()));
    connect(m_specificButton, SIGNAL(clicked()), this, SLOT(onSpecificButtonClicked()));
    connect(m_scanButton, SIGNAL(clicked()), this, SLOT(onScanClicked()));
}

WifiConfigIfaceWidget::~WifiConfigIfaceWidget()
{
    delete m_anyButton;
    delete m_specificButton;
    delete m_essidLabel;
    delete m_essidEdit;
    delete m_scanButton;
    delete m_essidLayout;
    delete m_wifiLayout;
    delete m_wifiGroup;
    delete m_wifiGroupBox;
}

ConfigIfaceWidget::Type WifiConfigIfaceWidget::ifaceType() const
{
    return Ieee80211;
}

void WifiConfigIfaceWidget::onAnyButtonClicked()
{
    enableScanningItems(false);
}

void WifiConfigIfaceWidget::onSpecificButtonClicked()
{
    enableScanningItems();
}

void WifiConfigIfaceWidget::enableScanningItems(bool enable)
{
    m_essidEdit->setEnabled(enable);
    m_essidLabel->setEnabled(enable);
    m_scanButton->setEnabled(enable);
}

void WifiConfigIfaceWidget::onScanClicked()
{
    if (m_scandlg == 0) {
        //setup scanview if it doesn't already exist
        m_scanView = new ApItemView();
        m_scanModel = new ApItemModel("eth0");
        m_scanDelegate = new ApItemDelegate(m_scanView);
        m_scanSelectionModel = new QItemSelectionModel(m_scanModel);
        m_scanModel->init();
        m_scanView->setModel(m_scanModel);
        m_scanView->setItemDelegate(m_scanDelegate);
        m_scanView->setSelectionModel(m_scanSelectionModel);
        
        m_scandlg = new KDialog();
        m_scandlg->setButtons( KDialog::Ok | KDialog::Cancel);
        m_scandlg->setCaption(i18n("Available Access Points"));
        m_scandlg->setMainWidget(m_scanView);
    }
    m_scandlg->show();
}

#include "configifacewidget.moc"
