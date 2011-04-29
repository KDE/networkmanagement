/*
Copyright 2008,2010 Will Stephenson <wstephenson@kde.org>

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

#include "802_11_wirelesswidget.h"
#include "settingwidget_p.h"

#include <KDebug>
#include <kdeversion.h>

#include <Solid/Device>
#include <solid/control/networkmanager.h>
#include <solid/control/wirelessnetworkinterface.h>

#include <connection.h>
#include <settings/802-11-wireless.h>
#include <uiutils.h>

#include "scanwidget.h"
#include "ui_802-11-wireless.h"


class Wireless80211WidgetPrivate : public SettingWidgetPrivate
{
public:
    Ui_Wireless80211Config ui;
    Knm::WirelessSetting * setting;
    QString originalSsid;
    QString proposedSsid;

    enum BandIndex { AIndex = 0, BGIndex};
    enum ModeIndex { InfrastructureIndex = 0, AdhocIndex};
};

Wireless80211Widget::Wireless80211Widget(Knm::Connection* connection, const QString &ssid, QWidget * parent)
    : SettingWidget(*new Wireless80211WidgetPrivate, connection, parent)
{
    Q_D(Wireless80211Widget);
    d->ui.setupUi(this);
    d->valid = false; //until there is a SSID at least
    // setup validation
    connect(d->ui.ssid, SIGNAL(textChanged(const QString&)), SLOT(validate()));
    d->proposedSsid = ssid;
    d->setting = static_cast<Knm::WirelessSetting *>(connection->setting(Knm::Setting::Wireless));
    d->ui.ssid->setText(d->proposedSsid);

    d->ui.mtu->setSuffix(ki18np(" byte", " bytes"));
    connect(d->ui.btnScan, SIGNAL(clicked()), SLOT(scanClicked()));
    foreach (Solid::Control::NetworkInterface * iface, Solid::Control::NetworkManager::networkInterfaces()) {
        if (iface->type() == Solid::Control::NetworkInterface::Ieee80211) {

            Solid::Control::WirelessNetworkInterface * wiface = static_cast<Solid::Control::WirelessNetworkInterface*>(iface);
            d->ui.cmbMacAddress->addItem(UiUtils::interfaceNameLabel(iface->uni(), KNetworkManagerServicePrefs::SystemNames), wiface->hardwareAddress().toLatin1());
        }
    }
    
    modeChanged(d->ui.cmbMode->currentIndex());
    connect(d->ui.cmbMode,SIGNAL(currentIndexChanged(int)),SLOT(modeChanged(int)));
    connect(d->ui.band,SIGNAL(currentIndexChanged(int)),SLOT(bandChanged(int)));
    connect(d->ui.useCurrentApAsBssid,SIGNAL(clicked()),SLOT(copyToBssid()));
}

Wireless80211Widget::~Wireless80211Widget()
{
}

void Wireless80211Widget::readConfig()
{
    Q_D(Wireless80211Widget);
    //kDebug() << kBacktrace();
    switch(d->setting->mode())
    {
        case Knm::WirelessSetting::EnumMode::adhoc:
            d->ui.cmbMode->setCurrentIndex(d->AdhocIndex);
            break;
        case Knm::WirelessSetting::EnumMode::infrastructure:
        default:
            d->ui.cmbMode->setCurrentIndex(d->InfrastructureIndex);
    }
    switch(d->setting->band())
    {
        case Knm::WirelessSetting::EnumBand::a:
            d->ui.band->setCurrentIndex(d->AIndex);
            break;
        case Knm::WirelessSetting::EnumBand::bg:
        default:
            d->ui.band->setCurrentIndex(d->BGIndex);
            break;
    }
    // need to check that ssids containing international characters are restored correctly
    if (!d->setting->ssid().isEmpty()) {
        d->ui.ssid->setText(QString::fromAscii(d->setting->ssid()));
        d->originalSsid = QString::fromAscii(d->setting->ssid());
    }
    d->ui.bssid->setText(QString::fromAscii(d->setting->bssid()));
    if (!d->setting->macaddress().isEmpty()) {
        int i = d->ui.cmbMacAddress->findData(QVariant(d->setting->macaddressAsString()));
        if (i == -1) {
            d->ui.cmbMacAddress->addItem(i18nc("@item:inlist item for hardware that is currently not attached to the machine with MAC address", "Disconnected interface (%1)", d->setting->macaddressAsString()));
            d->ui.cmbMacAddress->setCurrentIndex(d->ui.cmbMacAddress->count() - 1);
        } else {
            d->ui.cmbMacAddress->setCurrentIndex(i);
        }
    }
    d->ui.mtu->setValue(d->setting->mtu());
    d->ui.channel->setValue(d->ui.channel->posFromChannel(d->setting->channel()));
}

void Wireless80211Widget::writeConfig()
{
    Q_D(Wireless80211Widget);
    kDebug();

    d->setting->setSsid(d->ui.ssid->text().toAscii());
    kDebug() << d->setting->ssid();
    switch ( d->ui.cmbMode->currentIndex()) {
        case Wireless80211WidgetPrivate::InfrastructureIndex:
            d->setting->setMode(Knm::WirelessSetting::EnumMode::infrastructure);
            break;
        case Wireless80211WidgetPrivate::AdhocIndex:
            d->setting->setMode(Knm::WirelessSetting::EnumMode::adhoc);
            switch (d->ui.band->currentIndex())
            {
                case Wireless80211WidgetPrivate::AIndex:
                    d->setting->setBand(Knm::WirelessSetting::EnumBand::a);
                    break;
                case Wireless80211WidgetPrivate::BGIndex:
                    d->setting->setBand(Knm::WirelessSetting::EnumBand::bg);
                    break;
            }
            d->setting->setChannel(d->ui.channel->channelFromPos(d->ui.channel->value()));
            break;
    }

    int i = d->ui.cmbMacAddress->currentIndex();
    if ( i == 0) {
        d->setting->setMacaddress(QByteArray());
    } else {
        d->setting->setMacaddressFromString(d->ui.cmbMacAddress->itemData(i).toString());
    }

    if (d->ui.bssid->text() != QString::fromLatin1(":::::")) {
        d->setting->setBssid(d->ui.bssid->text().toAscii());
    } else {
        d->setting->setBssid(QByteArray());
    }
    d->setting->setMtu(d->ui.mtu->value());
}

void Wireless80211Widget::scanClicked()
{
    Q_D(Wireless80211Widget);
    KDialog scanDialog;
    scanDialog.setCaption(i18nc("@title:window wireless network scan dialog",
                "Available Networks"));
    scanDialog.setButtons( KDialog::Ok | KDialog::Cancel);
    ScanWidget scanWid;
    scanDialog.setMainWidget(&scanWid);
    if (scanDialog.exec() == QDialog::Accepted) {
        d->ui.ssid->setText(scanWid.currentAccessPoint());
        emit ssidSelected(scanWid.currentAccessPoint());
    }
}

QByteArray Wireless80211Widget::selectedInterfaceHardwareAddress() const
{
    Q_D(const Wireless80211Widget);
    QByteArray hwAddr;
    int i = d->ui.cmbMacAddress->currentIndex();
    if ( i != 0) {
        hwAddr = d->ui.cmbMacAddress->itemData(i).toByteArray();
    }
    return hwAddr;
}

QString Wireless80211Widget::enteredSsid() const
{
    Q_D(const Wireless80211Widget);
    return d->ui.ssid->text();
}

bool Wireless80211Widget::enteredSsidIsDirty() const
{
    Q_D(const Wireless80211Widget);
    return (d->ui.ssid->text() != d->originalSsid);
}

void Wireless80211Widget::setEnteredSsidClean()
{
    Q_D(Wireless80211Widget);
    d->originalSsid = d->ui.ssid->text();
}

void Wireless80211Widget::validate()
{
    Q_D(Wireless80211Widget);
    d->valid = (d->ui.ssid->text().length() > 0 && d->ui.ssid->text().length() < 33);
    emit valid(d->valid);
}

void Wireless80211Widget::modeChanged(int index)
{
    Q_D(Wireless80211Widget);
    switch (index) {
        case 1:
            bandChanged(d->ui.band->currentIndex());
            d->ui.band->setVisible(true);
            d->ui.label_3->setVisible(true);
            d->ui.channel->setVisible(true);
            d->ui.label_4->setVisible(true);
            break;
        case 0:
        default:
            d->ui.band->setVisible(false);
            d->ui.label_3->setVisible(false);
            d->ui.channel->setVisible(false);
            d->ui.label_4->setVisible(false);
            break;
    }
}

void Wireless80211Widget::bandChanged(int index)
{
    Q_D(Wireless80211Widget);
    d->ui.channel->setBand(index);
}

void Wireless80211Widget::copyToBssid()
{
    Q_D(Wireless80211Widget);
    foreach (Solid::Control::NetworkInterface * iface, Solid::Control::NetworkManager::networkInterfaces()) {
        if (iface->type() == Solid::Control::NetworkInterface::Ieee80211) {
            Solid::Control::WirelessNetworkInterface * wiface = static_cast<Solid::Control::WirelessNetworkInterface*>(iface);
            QString activeAp = wiface->activeAccessPoint();
            if (!activeAp.isEmpty()) {
                Solid::Control::AccessPoint * ap = wiface->findAccessPoint(wiface->activeAccessPoint());
                if (ap) {
                    d->ui.bssid->setText(ap->hardwareAddress());
                }
            }
            return;
        }
    }
}


Wireless80211WidgetBand::Wireless80211WidgetBand(QWidget * parent)
    :QSpinBox(parent)
{
    selectedBand = 0;
  
    QList<int> channels_a;
    QList<int> channels_b;

    channels_a << 7 << 8 << 9 << 11 << 12 << 16 << 34 << 36 << 38 << 40 << 42 << 44 << 46 << 48 << 52 << 56 << 60 << 64 << 100 << 104 << 108 << 112 << 116 << 120 << 124 << 128 << 132 << 136 << 140 << 149 << 153 << 157 << 161 << 165 << 183 << 184 << 185 << 187 << 188 << 189 << 192 << 196;
    channels_b << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9 << 10 << 11 << 12 << 13;
    
    channels << channels_a << channels_b;
}

QString Wireless80211WidgetBand::textFromValue(int value) const
{
    QString text = QString::number(channels.at(selectedBand).at(value),10);
    return text;
}

uint Wireless80211WidgetBand::channelFromPos(int pos) const
{
    QString text = QString::number(channels.at(selectedBand).at(pos),10);
    bool ok;
    return text.toUInt(&ok,10);
}

uint Wireless80211WidgetBand::posFromChannel(int channel) const
{
    int pos = channels.at(selectedBand).indexOf(channel);
    return (pos < 0) ? 0 : static_cast<uint>(pos);
}

void Wireless80211WidgetBand::setBand(int band)
{
    selectedBand = band;
    setMaximum(channels.at(selectedBand).size()-1);
    setValue(0);
}
// vim: sw=4 sts=4 et tw=100
