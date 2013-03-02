/*
Copyright 2008,2010 Will Stephenson <wstephenson@kde.org>
Copyright 2010-2013 Lamarque V. Souza <lamarque@kde.org>

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

    enum BandIndex { AutoIndex = 0, AIndex, BGIndex};
    enum ModeIndex { InfrastructureIndex = 0, AdhocIndex, ApModeIndex};
};

Wireless80211Widget::Wireless80211Widget(Knm::Connection* connection, const QString &ssid, bool shared, QWidget * parent)
    : SettingWidget(*new Wireless80211WidgetPrivate, connection, parent)
{
    Q_D(Wireless80211Widget);
    d->ui.setupUi(this);
    d->valid = false; //until there is a SSID at least
    // setup validation
    connect(d->ui.ssid, SIGNAL(textChanged(QString)), SLOT(validate()));
    d->proposedSsid = ssid;
    d->setting = static_cast<Knm::WirelessSetting *>(connection->setting(Knm::Setting::Wireless));
    d->ui.ssid->setText(d->proposedSsid);

    d->ui.mtu->setSuffix(ki18np(" byte", " bytes"));
    connect(d->ui.btnScan, SIGNAL(clicked()), SLOT(scanClicked()));
    foreach (Solid::Control::NetworkInterfaceNm09 * iface, Solid::Control::NetworkManagerNm09::networkInterfaces()) {
        if (iface->type() == Solid::Control::NetworkInterfaceNm09::Wifi) {
            Solid::Control::WirelessNetworkInterfaceNm09 * wiface = static_cast<Solid::Control::WirelessNetworkInterfaceNm09*>(iface);
            d->ui.cmbMacAddress->addItem(UiUtils::interfaceNameLabel(iface->uni(), KNetworkManagerServicePrefs::SystemNames), UiUtils::macAddressFromString(wiface->permanentHardwareAddress()));
        }
    }

    connect(d->ui.band,SIGNAL(currentIndexChanged(int)),d->ui.channel,SLOT(setBand(int)));
    connect(d->ui.cmbMode,SIGNAL(currentIndexChanged(int)),SLOT(modeChanged(int)));
    connect(d->ui.btnSelectBssid,SIGNAL(clicked()),SLOT(copyToBssid()));
    connect(d->ui.clonedMacAddressRandom, SIGNAL(clicked()), this, SLOT(generateRandomClonedMac()));

    if (shared) {
        d->ui.btnScan->hide();
        d->ui.label_2->hide();
        d->ui.cmbMode->hide();
        d->ui.label_5->hide();
        d->ui.bssid->hide();
        d->ui.btnSelectBssid->hide();
        d->ui.label_8->hide();
        d->ui.clonedMacAddress->hide();
        d->ui.clonedMacAddressRandom->hide();
    } else {
        modeChanged(d->ui.cmbMode->currentIndex());
    }
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
        case Knm::WirelessSetting::EnumMode::apMode:
            d->ui.cmbMode->setCurrentIndex(d->ApModeIndex);
            break;
        case Knm::WirelessSetting::EnumMode::infrastructure:
        default:
            d->ui.cmbMode->setCurrentIndex(d->InfrastructureIndex);
    }
    switch(d->setting->band())
    {
        case Knm::WirelessSetting::EnumBand::automatic:
            d->ui.band->setCurrentIndex(d->AutoIndex);
            d->ui.channel->setBand(d->AutoIndex);
            break;
        case Knm::WirelessSetting::EnumBand::a:
            d->ui.band->setCurrentIndex(d->AIndex);
            d->ui.channel->setBand(d->AIndex);
            d->ui.channel->setValue(d->ui.channel->posFromChannel(d->setting->channel()));
            break;
        case Knm::WirelessSetting::EnumBand::bg:
        default:
            d->ui.band->setCurrentIndex(d->BGIndex);
            d->ui.channel->setBand(d->BGIndex);
            d->ui.channel->setValue(d->ui.channel->posFromChannel(d->setting->channel()));
            break;
    }
    // need to check that ssids containing international characters are restored correctly
    if (!d->setting->ssid().isEmpty()) {
        d->ui.ssid->setText(QString::fromAscii(d->setting->ssid()));
        d->originalSsid = QString::fromAscii(d->setting->ssid());
    }
    d->ui.bssid->setText(UiUtils::macAddressAsString(d->setting->bssid()));
    if (!d->setting->macaddress().isEmpty()) {
        int i = d->ui.cmbMacAddress->findData(d->setting->macaddress());
        if (i == -1) {
            d->ui.cmbMacAddress->addItem(i18nc("@item:inlist item for hardware that is currently not attached to the machine with MAC address", "Disconnected interface (%1)", UiUtils::macAddressAsString(d->setting->macaddress())));
            d->ui.cmbMacAddress->setCurrentIndex(d->ui.cmbMacAddress->count() - 1);
        } else {
            d->ui.cmbMacAddress->setCurrentIndex(i);
        }
    }
    d->ui.clonedMacAddress->setText(UiUtils::macAddressAsString(d->setting->clonedmacaddress()));
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
        case Wireless80211WidgetPrivate::ApModeIndex:
            if (d->ui.cmbMode->currentIndex() == Wireless80211WidgetPrivate::ApModeIndex) {
                d->setting->setMode(Knm::WirelessSetting::EnumMode::apMode);
            }
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
        d->setting->setMacaddress(d->ui.cmbMacAddress->itemData(i).toByteArray());
    }

    if (d->ui.bssid->text() != QString::fromLatin1(":::::")) {
        d->setting->setBssid(UiUtils::macAddressFromString(d->ui.bssid->text()));
    } else {
        d->setting->setBssid(QByteArray());
    }
    d->setting->setMtu(d->ui.mtu->value());
    if (d->ui.clonedMacAddress->text() != QString::fromLatin1(":::::")) {
        d->setting->setClonedmacaddress(UiUtils::macAddressFromString(d->ui.clonedMacAddress->text()));
    } else {
        d->setting->setClonedmacaddress(QByteArray());
    }
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
    connect(&scanWid,SIGNAL(doubleClicked()),&scanDialog,SLOT(accept()));
    if (scanDialog.exec() == QDialog::Accepted) {
        QPair<QString,QString> accessPoint = scanWid.currentAccessPoint();
        d->ui.ssid->setText(accessPoint.first);
        d->ui.bssid->setText(accessPoint.second);
        const QPair<Solid::Control::WirelessNetworkInterfaceNm09 *, Solid::Control::AccessPointNm09 *> pair = scanWid.currentAccessPointUni();
        emit ssidSelected(pair.first, pair.second);
        setAccessPointData(pair.first, pair.second);
    }
}

void Wireless80211Widget::setAccessPointData(const Solid::Control::WirelessNetworkInterfaceNm09 *wiface, const Solid::Control::AccessPointNm09 * ap) const
{
    if (!wiface || !ap) {
        return;
    }

    Q_D(const Wireless80211Widget);
    QPair<int, int> bandAndChannel = UiUtils::findBandAndChannel((int)ap->frequency());

    switch(bandAndChannel.first)
    {
        case Knm::WirelessSetting::EnumBand::a:
            d->ui.band->setCurrentIndex(d->AIndex);
            break;
        case Knm::WirelessSetting::EnumBand::bg:
        default:
            d->ui.band->setCurrentIndex(d->BGIndex);
            break;
    }

    switch (ap->mode()) {
        case Solid::Control::WirelessNetworkInterfaceNm09::Adhoc:
            d->ui.cmbMode->setCurrentIndex(d->AdhocIndex);
        case Solid::Control::WirelessNetworkInterfaceNm09::ApMode:
            if (ap->mode() == Solid::Control::WirelessNetworkInterfaceNm09::ApMode) {
                d->ui.cmbMode->setCurrentIndex(d->ApModeIndex);
            }

            // This one has to go after the d->ui.band->setCurrentIndex() above;
            d->ui.channel->setValue(d->ui.channel->posFromChannel(bandAndChannel.second));
            break;
        default:
            d->ui.cmbMode->setCurrentIndex(d->InfrastructureIndex);
            // Channel defaults to "automatic" in this case.
            break;
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
    if (d->ui.ssid->text().length() > 0 && d->ui.ssid->text().length() < 33) {
        d->valid = true;
        d->ui.btnSelectBssid->setEnabled(true);
    } else {
        d->ui.btnSelectBssid->setEnabled(false);
    }
    emit valid(d->valid);
}

void Wireless80211Widget::modeChanged(int index)
{
    Q_D(Wireless80211Widget);
    switch (index) {
        case Wireless80211WidgetPrivate::AdhocIndex:
        case Wireless80211WidgetPrivate::ApModeIndex:
            d->ui.channel->setBand(d->ui.band->currentIndex());
            d->ui.band->setVisible(true);
            d->ui.label_3->setVisible(true);
            d->ui.channel->setVisible(true);
            d->ui.label_4->setVisible(true);
            break;
        case Wireless80211WidgetPrivate::InfrastructureIndex:
        default:
            d->ui.band->setVisible(false);
            d->ui.label_3->setVisible(false);
            d->ui.channel->setVisible(false);
            d->ui.label_4->setVisible(false);
            break;
    }
}

void Wireless80211Widget::copyToBssid()
{
    Q_D(Wireless80211Widget);
    QString hardwareAddress;
    int maxSignalStrength = 0;
    foreach (Solid::Control::NetworkInterfaceNm09 * iface, Solid::Control::NetworkManagerNm09::networkInterfaces()) {
        if (iface->type() == Solid::Control::NetworkInterfaceNm09::Wifi) {
            Solid::Control::WirelessNetworkInterfaceNm09 * wiface = static_cast<Solid::Control::WirelessNetworkInterfaceNm09*>(iface);
            int i = d->ui.cmbMacAddress->currentIndex();
            if (i == 0 || d->ui.cmbMacAddress->itemData(i).toString() == wiface->hardwareAddress()){
                QString activeAp = wiface->activeAccessPoint();
                Solid::Control::AccessPointNm09 * ap = 0;
                if (!activeAp.isEmpty() && activeAp != QLatin1String("/")) {
                    ap = wiface->findAccessPoint(activeAp);
                    if (ap && ap->ssid() != d->ui.ssid->text())
                        ap = 0;
                }
                if (!ap && !d->ui.ssid->text().isEmpty()) {
                    Solid::Control::WirelessNetworkInterfaceEnvironment environment(wiface);
                    Solid::Control::WirelessNetwork * network = environment.findNetwork(d->ui.ssid->text());
                    if (network)
                        ap = wiface->findAccessPoint(network->referenceAccessPoint());
                }
                if (ap && ap->signalStrength() > maxSignalStrength) {
                    maxSignalStrength = ap->signalStrength();
                    hardwareAddress = ap->hardwareAddress();
                }
            }
        }
    }

    if (maxSignalStrength > 0) {
        d->ui.bssid->setText(hardwareAddress);
    }
}

void Wireless80211Widget::generateRandomClonedMac()
{
    Q_D(Wireless80211Widget);
    QByteArray mac;
    mac.resize(6);
    for (int i = 0; i < 6; i++) {
        int random = qrand() % 255;
        mac[i] = random;
    }
    d->ui.clonedMacAddress->setText(UiUtils::macAddressAsString(mac));
}

Wireless80211WidgetBand::Wireless80211WidgetBand(QWidget * parent)
    :QSpinBox(parent)
{
    QList<int> channels_a;
    QList<int> channels_b;

    channels_a << 0 << 7 << 8 << 9 << 11 << 12 << 16 << 34 << 36 << 38 << 40 << 42 << 44 << 46 << 48 << 52 << 56 << 60 << 64 << 100 << 104 << 108 << 112 << 116 << 120 << 124 << 128 << 132 << 136 << 140 << 149 << 153 << 157 << 161 << 165 << 183 << 184 << 185 << 187 << 188 << 189 << 192 << 196;
    channels_b << 0 << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9 << 10 << 11 << 12 << 13;

    channels << channels_a << channels_b;

    setBand(Wireless80211WidgetPrivate::AIndex);
}

QString Wireless80211WidgetBand::textFromValue(int value) const
{
    QString text = QString::number(channels.at(selectedBand).at(value),10);
    return text;
}

int Wireless80211WidgetBand::valueFromText(const QString &text) const
{
    int channel = text.toInt();
    int pos = posFromChannel(channel);
    return (pos < 0) ? 0 : pos;
}

QValidator::State Wireless80211WidgetBand::validate(QString &text, int &pos) const
{
    int channel = text.toInt();
    int position = posFromChannel(channel);
    int maxsize = QString::number(channels.at(selectedBand).last(),10).size();
    if (position < 0 && pos < maxsize) {
        return QValidator::Intermediate;
    } else if (position < 0 ) {
        return QValidator::Invalid;
    } else {
        return QValidator::Acceptable;
    }
}

int Wireless80211WidgetBand::channelFromPos(int pos) const
{
    return channels.at(selectedBand).at(pos);
}

int Wireless80211WidgetBand::posFromChannel(int channel) const
{
    int pos = channels.at(selectedBand).indexOf(channel);
    return pos;
}

void Wireless80211WidgetBand::setBand(int band)
{
    switch (band)
    {
        case Wireless80211WidgetPrivate::AutoIndex:
            setEnabled(false);
            break;
        case Wireless80211WidgetPrivate::AIndex:
            selectedBand = a;
            setEnabled(true);
            break;
        case Wireless80211WidgetPrivate::BGIndex:
            selectedBand = bg;
            setEnabled(true);
            break;
    }
    setMaximum(channels.at(selectedBand).size()-1);
    setValue(0);
}
// vim: sw=4 sts=4 et tw=100
