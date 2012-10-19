/*
Copyright 2010 Sebastian Kügler <sebas@kde.org>
Copyright 2010 Alexander Naumov <posix.ru@gmail.com>
Copyright 2010-2012 Lamarque Souza <lamarque@kde.org>

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

// own
#include "interfacedetailswidget.h"

// system
#include <arpa/inet.h>

// Qt
//#include <QGraphicsLinearLayout>
#include <QGridLayout>
//#include <QGraphicsLayout>
#include <QLabel>
#include <QMetaProperty>

// KDE
#include <KGlobalSettings>
#include <kdebug.h>

// Plasma
#include <Plasma/DataEngineManager>
#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/SignalPlotter>

//Solid
#include <QtNetworkManager/accesspoint.h>
#include <QtNetworkManager/wirelessdevice.h>
#include <QtNetworkManager/wireddevice.h>
#include <QtNetworkManager/bluetoothdevice.h>
#include <QtNetworkManager/ipv4config.h>
#include <QtNetworkManager/manager.h>
#include <QtModemManager/manager.h>
#include <Solid/Device>

#include <NetworkManager/NetworkManager.h>

#include <uiutils.h>

#include "interfaceitem.h"
#include "../libs/internals/settings/802-11-wireless.h"

class InterfaceDetails
{
    public:
        QStringList keys;
        NetworkManager::Device::Type type;
        NetworkManager::Device::State connectionState;
        QString ipv4Address;
        QString ipv4Gateway;
        int bitRate;
        QString interfaceName;
        QString mac;
        QString driver;
        QString activeAccessPoint;
        uint wifiChannelFrequency;
        int wifiChannel;
        int wifiBand;

        /* ModemManager */
        ModemManager::ModemGsmNetworkInterface::RegistrationInfoType registrationInfo;
        uint signalQuality;
        ModemManager::ModemInterface::Type modemType;
        ModemManager::ModemInterface::AccessTechnology accessTechnology;
        ModemManager::ModemInterface::Band band;
        ModemManager::ModemInterface::AllowedMode allowedMode;
        bool enabled;
        QString udi;
        QString device;
        QString masterDevice;
        QString unlockRequired;
        QString imei;
        QString imsi;
};

InterfaceDetailsWidget::InterfaceDetailsWidget(QGraphicsItem * parent) : QGraphicsWidget(parent, 0),
    m_ifaceItem(0), m_iface(0), m_updateEnabled(false)
{
    m_gridLayout = new QGraphicsGridLayout(this);

    int row = 1;
    m_gridLayout->addItem(m_ifaceItem, row, 0);
    //Info
    row++;
    m_info = new Plasma::Label(this);
    m_info->nativeWidget()->setTextFormat(Qt::RichText);
    m_info->setFont(KGlobalSettings::smallestReadableFont());
    m_info->setTextSelectable(true);
    m_info->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_gridLayout->addItem(m_info, row, 0);

    // Traffic

    //Interface
    row++;
    m_trafficNameLabel = new Plasma::Label(this);
    m_trafficNameLabel->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_trafficNameLabel->setText(i18nc("interface details", "<b>Traffic</b>"));
    m_gridLayout->addItem(m_trafficNameLabel, row, 0, 1, 2);

    // Traffic plotter
    row++;
    m_rxColor = QColor("#0099FF"); // blue
    m_txColor = QColor("#91FF00"); // green
    m_txColor.setAlphaF(0.6);
    m_trafficPlotter = new Plasma::SignalPlotter(this);
    m_trafficPlotter->setFont(KGlobalSettings::smallestReadableFont());
    m_trafficPlotter->addPlot(m_rxColor); // receiver green
    m_trafficPlotter->addPlot(m_txColor); // transmitter yellow
    m_trafficPlotter->setThinFrame(true);
    m_trafficPlotter->setShowLabels(true);
    m_trafficPlotter->setShowTopBar(true);
    m_trafficPlotter->setShowVerticalLines(false);
    m_trafficPlotter->setShowHorizontalLines(true);
    m_trafficPlotter->setHorizontalLinesCount(2);
    m_trafficPlotter->setUseAutoRange(true);
    m_trafficPlotter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_trafficPlotter->setMinimumHeight(5 * QFontMetrics(KGlobalSettings::smallestReadableFont()).height());

    m_gridLayout->addItem(m_trafficPlotter, row, 0, 1, 2);

    row++;
    m_traffic = new Plasma::Label(this);
    m_traffic->setFont(KGlobalSettings::smallestReadableFont());
    m_traffic->nativeWidget()->setWordWrap(false);
    m_traffic->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_gridLayout->addItem(m_traffic, row, 0);

#if 0
    // add pushbutton for "disconnect" action
    m_disconnectButton = new Plasma::PushButton(this);
    m_disconnectButton->setMaximumHeight(22);
    //m_disconnectButton->setMaximumWidth(22);
    m_disconnectButton->setIcon(KIcon("user-offline"));
    m_disconnectButton->setText(i18n("Disconnect"));
    m_disconnectButton->setToolTip(i18nc("@info:tooltip", "Click here to disconnect"));
    m_disconnectButton->setZValue(100);
#else
    m_disconnectButton = new Plasma::IconWidget(this);
    m_disconnectButton->setDrawBackground(true);
    m_disconnectButton->setOrientation(Qt::Horizontal);
    m_disconnectButton->setAcceptsHoverEvents(true);
    m_disconnectButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_disconnectButton->setIcon(KIcon("user-offline"));
    m_disconnectButton->setText(i18nc("disconnect button in interface details window", "Disconnect"));
    int disconnectButtonHeight = QFontMetrics(m_disconnectButton->font()).height() + QFontMetrics(m_disconnectButton->font()).xHeight() * 2;
    m_disconnectButton->setMinimumHeight(disconnectButtonHeight);
    m_disconnectButton->setMaximumHeight(disconnectButtonHeight);
#endif

    connect(m_disconnectButton, SIGNAL(clicked()), this, SLOT(emitDisconnectInterfaceRequest()));

    m_gridLayout->addItem(m_disconnectButton, 0, 0, 2, 1, Qt::AlignLeft | Qt::AlignVCenter);

    // add pushbutton for "back" action
    m_backButton = new Plasma::PushButton(this);
    int backButtonHeight = QFontMetrics(KGlobalSettings::generalFont()).height() + QFontMetrics(KGlobalSettings::generalFont()).xHeight();
    m_backButton->setMaximumHeight(backButtonHeight);
    m_backButton->setMaximumWidth(backButtonHeight);
    m_backButton->setIcon(KIcon("go-previous"));
    m_backButton->setToolTip(i18n("Go Back"));
    m_backButton->setZValue(100);

    connect(m_backButton, SIGNAL(clicked()), this, SIGNAL(back()));

    m_gridLayout->addItem(m_backButton, 0, 1, 2, 1);
    setLayout(m_gridLayout);

    // Add spacer to push content to the top
    row++;
    //QGraphicsWidget *spacer = new QGraphicsWidget(this);
    //spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    //m_gridLayout->addItem(spacer, row, 0);

    //resetUi();

    Plasma::DataEngineManager::self()->loadEngine("systemmonitor");

    //connect(e, SIGNAL(sourceAdded(QString)), this, SLOT(sourceAdded(QString)));

    details = new InterfaceDetails();
}

void InterfaceDetailsWidget::emitDisconnectInterfaceRequest()
{
    if (m_iface) {
        //kDebug() << m_iface->uni();
        emit disconnectInterfaceRequested(m_iface->uni());
    }
}

void InterfaceDetailsWidget::resetUi()
{
    QString na = i18nc("entry not available", "not available");
    QString format = "<b>%1:</b>&nbsp;%2";
    QString temp;

    temp = QString("<qt><table align=\"center\" border=\"0\"><tr><td align=\"right\" width=\"50%\">");
    temp += QString(format).arg(i18nc("traffic received empty", "Received")).arg("-");
    temp += QString("</td><td width=\"50%\">&nbsp;");
    temp += QString(format).arg(i18nc("traffic transmitted empty", "Transmitted")).arg("-");
    temp += QString("</td></tr></table></qt>");
    m_traffic->setText(temp);

    showDetails(true);

    // Quite ugly, but I need to investigate why I'm getting those crashes after removePlot calls
    for (int i = 0; i < 500; i++) {
        QList<double> v;
        v << 0 << 0;
        m_trafficPlotter->addSample(v);
    }
    /*
    kDebug() << m_trafficPlotter->plotColors().count() << "clrs, now removing";
    m_trafficPlotter->removePlot(0);
    m_trafficPlotter->removePlot(1);
    kDebug() << "Now:" << m_trafficPlotter->plotColors().count();
    m_trafficPlotter->addPlot(m_rxColor); // receiver green
    m_trafficPlotter->addPlot(m_txColor); // transmitter yellow
    kDebug() << "and adding back" << m_trafficPlotter->plotColors().count();
    */
}

void InterfaceDetailsWidget::resetInterfaceDetails()
{
    delete details;
    details = new InterfaceDetails();
    NetworkManager::ModemDevice *giface = qobject_cast<NetworkManager::ModemDevice*>(m_iface);
    if (giface) {
        giface->setModemCardIface(0);
        giface->setModemNetworkIface(0);
    }
    getDetails();
    showDetails();
}

void InterfaceDetailsWidget::getDetails()
{
    if (!m_iface) {
        return;
    }

    KNetworkManagerServicePrefs::self()->readConfig();
    KConfigGroup config(KNetworkManagerServicePrefs::self()->config(), QLatin1String("General"));
    details->keys = config.readEntry(QLatin1String("DetailKeys"), QStringList());

    details->type = m_iface->type();
    details->connectionState = static_cast<NetworkManager::Device::State>(m_iface->state());
    updateIPv4Details();
    details->bitRate = bitRate();
    details->interfaceName = m_iface->ipInterfaceName();
    if (details->interfaceName.isEmpty()) {
        details->interfaceName = m_iface->interfaceName();
    }
    details->mac = getMAC();
    details->driver = m_iface->driver();

    NetworkManager::WirelessDevice *wiface = qobject_cast<NetworkManager::WirelessDevice*>(m_iface);
    if (wiface) {
        details->activeAccessPoint = wiface->activeAccessPoint();
        NetworkManager::AccessPoint *ap = wiface->findAccessPoint(details->activeAccessPoint);
        if (ap) {
            details->wifiChannelFrequency = ap->frequency();
            QPair<int, int> bandAndChannel = UiUtils::findBandAndChannel(details->wifiChannelFrequency);
            details->wifiBand = bandAndChannel.first;
            details->wifiChannel = bandAndChannel.second;
        } else {
            details->wifiChannelFrequency = 0;
            details->wifiBand = Knm::WirelessSetting::EnumBand::bg;
            details->wifiChannel = -1;
        }
    } else {
        details->activeAccessPoint.clear();
    }

    NetworkManager::ModemDevice *giface = qobject_cast<NetworkManager::ModemDevice*>(m_iface);
    if (giface) {
        ModemManager::ModemGsmNetworkInterface *modemNetworkIface = giface->getModemNetworkIface();
        if (modemNetworkIface) {
            details->registrationInfo = modemNetworkIface->getRegistrationInfo();
            details->signalQuality = modemNetworkIface->getSignalQuality();
            details->accessTechnology = modemNetworkIface->getAccessTechnology();

            details->modemType = modemNetworkIface->type();
            details->band = modemNetworkIface->getBand();
            details->allowedMode = modemNetworkIface->getAllowedMode();
            details->enabled = modemNetworkIface->enabled();
            details->udi = modemNetworkIface->udi();
            details->device = modemNetworkIface->device();
            details->masterDevice = modemNetworkIface->masterDevice();
            details->unlockRequired = modemNetworkIface->unlockRequired();
        }

        ModemManager::ModemGsmCardInterface *modemCardIface = giface->getModemCardIface();
        if (modemCardIface) {
            details->imei = modemCardIface->getImei();
            details->imsi = modemCardIface->getImsi();
        }
    }
}

QString InterfaceDetailsWidget::connectionStateToString(NetworkManager::Device::State state, const QString &connectionName)
{
    NetworkManager::ModemDevice *giface = qobject_cast<NetworkManager::ModemDevice*>(m_iface);
    if (giface && !details->enabled) {
        return i18nc("state of mobile broadband connection", "not enabled");
    }

    return UiUtils::connectionStateToString(state, connectionName);
}

void InterfaceDetailsWidget::showDetails(bool reset)
{
    QString info;
    QString na = i18nc("entry not available", "not available");
    QString format = "<tr><td align=\"right\" width=\"30%\" style=\"white-space: nowrap\"><b>%1:</b></td><td width=\"30%\">&nbsp;%2</td></tr>";

    // generate html table header
    info = QString("<qt><table align=\"center\" border=\"0\">");

    m_disconnectButton->setVisible(details->connectionState == NetworkManager::Device::Activated);
    if (!reset && m_iface) {
        NetworkManager::WirelessDevice *wiface = qobject_cast<NetworkManager::WirelessDevice*>(m_iface);
        NetworkManager::AccessPoint *ap = 0;
        NetworkManager::ModemDevice *giface = 0;

        if (wiface) {
            ap = wiface->findAccessPoint(details->activeAccessPoint);

            if (ap && details->wifiChannelFrequency != ap->frequency()) {
                details->wifiChannelFrequency = ap->frequency();
                QPair<int, int> bandAndChannel = UiUtils::findBandAndChannel(details->wifiChannelFrequency);
                details->wifiBand = bandAndChannel.first;
                details->wifiChannel = bandAndChannel.second;
            }
        } else {
            giface = qobject_cast<NetworkManager::ModemDevice*>(m_iface);
        }

        foreach (const QString & key, details->keys) {
            if (key == QLatin1String("interface:type")) {
                info += QString(format).arg(i18nc("interface details", "Type"), UiUtils::interfaceTypeLabel(details->type, m_iface));
            } else if (key == QLatin1String("interface:status")) {
                info += QString(format).arg(i18nc("interface details", "Connection State"), connectionStateToString(details->connectionState));
            } else if (key == QLatin1String("interface:bitrate")) {
                info += QString(format).arg(i18nc("interface details", "Connection Speed"), details->bitRate ? UiUtils::connectionSpeed(details->bitRate) : i18nc("bitrate", "Unknown"));
            } else if (key == QLatin1String("interface:name")) {
                info += QString(format).arg(i18nc("interface details", "System Name"), details->interfaceName);
            } else if (key == QLatin1String("interface:hardwareaddress") && !details->mac.isEmpty()) {
                info += QString(format).arg(i18nc("interface details", "MAC Address"), details->mac);
            } else if (key == QLatin1String("interface:driver")) {
                info += QString(format).arg(i18nc("interface details", "Driver"), details->driver);
            } else if (key == QLatin1String("ipv4:address")) {
                info += QString(format).arg(i18nc("interface details", "IPv4 Address"), details->ipv4Address);
            } else if (key == QLatin1String("ipv4:gateway")) {
                info += QString(format).arg(i18nc("interface details", "IPv4 Gateway"), details->ipv4Gateway);
            } else if (key == QLatin1String("wireless:ssid") && ap) {
                info += QString(format).arg(i18nc("interface details", "Access Point (SSID)"), ap->ssid());
            } else if (key == QLatin1String("wireless:accesspoint") && ap) {
                info += QString(format).arg(i18nc("interface details", "Access Point (MAC)"), ap->hardwareAddress());
            } else if (key == QLatin1String("wireless:band") && ap) {
                info += QString(format).arg(i18nc("@item:intable wireless band", "Band"), UiUtils::wirelessBandToString(details->wifiBand));
            } else if (key == QLatin1String("wireless:channel") && ap) {
                info += QString(format).arg(i18nc("@item:intable wireless channel", "Channel"), QString("%1 (%2 MHz)").arg(details->wifiChannel).arg(details->wifiChannelFrequency));
            } else if (key == QLatin1String("mobile:operator") && giface) {
                info += QString(format).arg(i18nc("interface details", "Operator"), details->registrationInfo.operatorName);
            } else if (key == QLatin1String("mobile:quality") && giface) {
                info += QString(format).arg(i18nc("interface details", "Signal Quality"), QString("%1 %").arg(details->signalQuality));
            } else if (key == QLatin1String("mobile:technology") && giface) {
                info += QString(format).arg(i18nc("interface details", "Access Technology"), QString("%1/%2").arg(UiUtils::convertTypeToString(details->modemType), UiUtils::convertAccessTechnologyToString(details->accessTechnology)));
            } else if (key == QLatin1String("mobile:band") && giface) {
                info += QString(format).arg(i18nc("interface details", "Frequency Band"), UiUtils::convertBandToString(details->band));
            } else if (key == QLatin1String("mobile:mode") && giface) {
                info += QString(format).arg(i18nc("interface details", "Allowed Mode"), UiUtils::convertAllowedModeToString(details->allowedMode));
            } else if (key == QLatin1String("mobile:device") && giface) {
                info += QString(format).arg(i18nc("interface details", "Device Name"), details->device);
            } else if (key == QLatin1String("mobile:unlock") && giface) {
                info += QString(format).arg(i18nc("interface details", "Unlock Required"), details->unlockRequired.isEmpty() ? i18nc("Answer to the question: is modem unlock required?", "No") : QString("%1: %2").arg(i18nc("Answer to the question: is modem unlock required?", "Yes"), details->unlockRequired));
            } else if (key == QLatin1String("mobile:imei") && giface) {
                info += QString(format).arg(i18nc("interface details", "IMEI"), details->imei);
            } else if (key == QLatin1String("mobile:imsi") && giface) {
                info += QString(format).arg(i18nc("interface details", "IMSI"), details->imsi);
            }
        }
    } else {
        info += QString(format).arg(i18nc("interface details", "Type"), na);
        info += QString(format).arg(i18nc("interface details", "Connection State"), na);
        info += QString(format).arg(i18nc("interface details", "Network Address (IP)"), na);
        info += QString(format).arg(i18nc("interface details", "Connection Speed"), na);
        info += QString(format).arg(i18nc("interface details", "System Name"), na);
        info += QString(format).arg(i18nc("interface details", "Hardware Address (MAC)"), na);
        info += QString(format).arg(i18nc("interface details", "Driver"), na);
    }

    // For same reason the last row sometimes is not shown when there is a certain number of rows in the html table.
    format.remove(QChar(':'));
    info += QString(format).arg("", "");

    info += QLatin1String("</table></qt>");
    m_info->setText(info);
    update();
}

int InterfaceDetailsWidget::bitRate()
{
    int bitRate = 0;

    //wifi?
    NetworkManager::WirelessDevice * wliface =
                    qobject_cast<NetworkManager::WirelessDevice*>(m_iface);
    if (wliface) {
        bitRate = wliface->bitRate(); //Bit
    } else {     // wired?
        NetworkManager::WiredDevice * wdiface =
                                    qobject_cast<NetworkManager::WiredDevice*> (m_iface);
        if (wdiface) {
            bitRate = wdiface->bitRate();
        }
    }
    return bitRate;
}

/*void InterfaceDetailsWidget::sourceAdded(const QString &source)
{
    kDebug() << "Source added:" << source;
}*/

InterfaceDetailsWidget::~InterfaceDetailsWidget()
{
    delete details;
}

void InterfaceDetailsWidget::setUpdateEnabled(bool enable)
{
    // disconnect / connect goes here
    Plasma::DataEngine *e = engine();
    if (e) {
        int interval = 2000;
        if (enable) {
            if (m_iface) {
                kDebug() << "connecting ..." << m_rxSource << m_txSource;
                e->connectSource(m_rxSource, this, interval);
                e->connectSource(m_txSource, this, interval);
                e->connectSource(m_rxTotalSource, this, interval);
                e->connectSource(m_txTotalSource, this, interval);
            }

            getDetails();
            showDetails();
            connectSignals();
        } else {
            kDebug() << "disconnecting ..." << m_rxSource << m_txSource;
            e->disconnectSource(m_rxSource, this);
            e->disconnectSource(m_txSource, this);
            e->disconnectSource(m_rxTotalSource, this);
            e->disconnectSource(m_txTotalSource, this);

            disconnectSignals();
        }
    }
    m_updateEnabled = enable;
}

void InterfaceDetailsWidget::updateWidgets()
{
    //kDebug() << s;
    //m_traffic->setText(s);

    double _r;
    double _t;
    QString r, t;

    if (m_speedUnit == KNetworkManagerServicePrefs::KBits_s) {
        _r = m_rx.toInt() << 3;
        _t = m_tx.toInt() << 3;

        if (_r < 1000) {
            m_rxUnit = i18n("KBit/s");
        } else if (_r < 1000000) {
            m_rxUnit = i18n("MBit/s");
            _r /= 1000;
            _t /= 1000;
        } else {
            m_rxUnit = i18n("GBit/s");
            _r /= 1000000;
            _t /= 1000000;
        }

        m_txUnit = m_rxUnit;
        r = QString("%1 %2").arg(QString::number(_r, 'f', 0), m_rxUnit);
        t = QString("%1 %2").arg(QString::number(_t, 'f', 0), m_txUnit);
    } else {
        _r = m_rx.toDouble();
        _t = m_tx.toDouble();

        r = KGlobal::locale()->formatByteSize(_r*1024);
        r.append("/s");
        t = KGlobal::locale()->formatByteSize(_t*1024);
        t.append("/s");
    }

    QList<double> v;
    v << _r << _t;
    m_trafficPlotter->addSample(v);
    m_trafficPlotter->setUnit(m_rxUnit);
    //m_trafficPlotter->setTitle(i18nc("traffic, e.g. n KB/s / m KB/s", "%1 %2 %3 %4", m_rx, m_rxUnit, m_tx, m_txUnit));

    QString s = i18nc("traffic, e.g. n KB/s\n m KB/s", "%1 %2", r, t);
    m_trafficPlotter->setTitle(s);

    QString format = "<b>%1:</b>&nbsp;%2";
    QString temp;

    temp = QString("<qt><table align=\"center\" border=\"0\"><tr>");
    temp += QString("<td width=\"20pt\" bgcolor=\"%1\">&nbsp;&nbsp;").arg(m_rxColor.name());
    temp += QString("</td><td width=\"50%\">");
    temp += QString(format).arg(i18n("Received"), KGlobal::locale()->formatByteSize(m_rxTotal*1000, 2));
    temp += QString("&nbsp;&nbsp;</td><td width=\"20pt\" bgcolor=\"%1\">&nbsp;&nbsp;").arg(m_txColor.name());
    temp += QString("</td><td width=\"50%\">");
    temp += QString(format).arg(i18n("Transmitted"), KGlobal::locale()->formatByteSize(m_txTotal*1000, 2));
    temp += QString("</td></tr></table></qt>");
    m_traffic->setText(temp);
}

Plasma::DataEngine* InterfaceDetailsWidget::engine()
{
    Plasma::DataEngine *e = Plasma::DataEngineManager::self()->engine("systemmonitor");

    if (e->isValid()) {
        kDebug() << "engine loaded. :-)";
        return e;
    }

    kDebug() << "engine NOT loaded. )-:";
    return 0;
}

void InterfaceDetailsWidget::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    //kDebug() << "Source Updated!!!" << sourceName << data;
    if (sourceName == m_txSource) {
        m_tx = data["value"].toString();
        m_txUnit = data["units"].toString();
    } else if (sourceName == m_rxSource) {
        m_rx = data["value"].toString();
        m_rxUnit = data["units"].toString();
    } else if (sourceName == m_rxTotalSource) {
        m_rxTotal = data["value"].toString().toLong();
        QString _srx = QString("%1Total").arg(m_rxSource);
    } else if (sourceName == m_txTotalSource) {
        m_txTotal = data["value"].toString().toLong();
    }
    //QString trafficInfo = i18nc("traffic status in details widget", "In: %1%2\nOut: ...", m_tx, m_txUnit);
    updateWidgets();
}

void InterfaceDetailsWidget::handleConnectionStateChange(NetworkManager::Device::State new_state, NetworkManager::Device::State old_state, NetworkManager::Device::StateChangeReason reason)
{
    Q_UNUSED(old_state)
    if ((new_state == NetworkManager::Device::Unavailable || new_state == NetworkManager::Device::Unmanaged || new_state == NetworkManager::Device::UnknownState) &&
        (reason == NetworkManager::Device::UnknownReason ||
         reason == NetworkManager::Device::DeviceRemovedReason)) {
        setInterface(0, false);
        emit back();
    } else {
        updateIPv4Details();
        details->connectionState = static_cast<NetworkManager::Device::State>(new_state);
        if (m_iface->type() == NetworkManager::Device::Bluetooth) {
            QString interfaceName = m_iface->ipInterfaceName();
            if (interfaceName != details->interfaceName) {
                // Hack to force updating interfaceName and traffic plot source.
                NetworkManager::Device *temp = m_iface;
                m_iface = 0;
                kDebug() << "Reseting interface " << temp->uni() << "(" << interfaceName << ")";
                setInterface(temp);
                setUpdateEnabled(m_updateEnabled);
            } else {
                showDetails();
            }
        } else
            showDetails();
    }
}

void InterfaceDetailsWidget::setInterfaceItem(InterfaceItem* item, bool disconnectOld)
{
    if (m_ifaceItem) {
        m_ifaceItem->hide();
    }

    NetworkManager::Device * iface = 0;
    m_ifaceItem = item;

    if (item) {
        iface = item->interface();
        item->show();
    }

    m_gridLayout->addItem(m_ifaceItem, 0, 0);
    setInterface(iface, disconnectOld);
}

void InterfaceDetailsWidget::setInterface(NetworkManager::Device* iface, bool disconnectOld)
{
    m_speedUnit = KNetworkManagerServicePrefs::self()->networkSpeedUnit();

    if (m_iface == iface) {
        return;
    }

    if (disconnectOld) {
        disconnectSignals();
    }
    m_iface = iface;
    resetUi();

    if (m_iface) {
        m_ifaceUni = m_iface->uni();
        getDetails();
        showDetails();
        connectSignals();

        QString interfaceName = m_iface->ipInterfaceName();
        if (details->interfaceName.isEmpty()) {
            details->interfaceName = m_iface->interfaceName();
        }

        m_rxSource = QString("network/interfaces/%1/receiver/data").arg(interfaceName);
        m_txSource = QString("network/interfaces/%1/transmitter/data").arg(interfaceName);
        m_rxTotalSource = QString("network/interfaces/%1/receiver/dataTotal").arg(interfaceName);
        m_txTotalSource = QString("network/interfaces/%1/transmitter/dataTotal").arg(interfaceName);
        m_rxTotal = m_txTotal = 0;

        /* Usb network interfaces are hotpluggable and Plasma::DataEngine seems to have difficulty
         * to recognise them after the engine is loaded, reloading the engine does the trick.
         * Eventually the engine will recognise them but not before the user get upset because
         * the traffic plot is not updating.
         */
        Plasma::DataEngine *e = engine();
        if (e && e->query(m_rxSource).empty()) {
            Plasma::DataEngineManager::self()->unloadEngine("systemmonitor");
            Plasma::DataEngineManager::self()->loadEngine("systemmonitor");
        }
    }
    /*
    Solid::Device *dev = new Solid::Device(iface->uni());
    kDebug() << "IFACE:" << dev->vendor();
    kDebug() << "product:" << dev->product();
    kDebug() << "udi:" << dev->udi();
    kDebug() << "desc:" << dev->description();
    kDebug() << "icon:" << dev->icon();
    */
}

QString InterfaceDetailsWidget::getMAC()
{
    //wifi?
    NetworkManager::WirelessDevice * wliface =
                    qobject_cast<NetworkManager::WirelessDevice *>(m_iface);
    if (wliface) {
        return wliface->hardwareAddress();
        /*
         * for later use ...
        NetworkManager::AccessPoint * ap = wliface->findAccessPoint(wliface->activeAccessPoint());
        if(ap) {
            temp = ap->ssid();
            kDebug() << "temp = " << temp;
        }
        */
    } else {
        NetworkManager::BluetoothDevice * btiface =
                    qobject_cast<NetworkManager::BluetoothDevice *>(m_iface);
        if (btiface) {
            return btiface->interfaceName();
        }

        // wired?
        NetworkManager::WiredDevice * wdiface =
                                    qobject_cast<NetworkManager::WiredDevice *> (m_iface);
        if (wdiface) {
            return wdiface->hardwareAddress();
        } else {
            // prevent crash for unconnected devices
            if (m_iface) { // last resort, although using ifaceName is not portable
                QList<Solid::Device> list = Solid::Device::listFromQuery(QString::fromLatin1("NetworkInterface.ifaceName == '%1'").arg(m_iface->interfaceName()));
                QList<Solid::Device>::iterator it = list.begin();

                if (it != list.end()) {
                    Solid::Device device = *it;
                    Solid::DeviceInterface *interface = it->asDeviceInterface(Solid::DeviceInterface::NetworkInterface);

                    if (interface) {
                        const QMetaObject *meta = interface->metaObject();

                        for (int i = meta->propertyOffset(); i<meta->propertyCount(); i++) {
                            QMetaProperty property = meta->property(i);

                            if (QString(meta->className()).mid(7) + '.' + property.name() == QString::fromLatin1("NetworkManager::Device.hwAddress")) {
                                QVariant value = property.read(interface);
                                return value.toString();
                            }
                        }
                    }
                }
            }
        }
    }

    return QString();
}

QString InterfaceDetailsWidget::getLastIfaceUni()
{
    return m_ifaceUni;
}

void InterfaceDetailsWidget::connectSignals()
{
    if (!m_iface) {
        return;
    }
    connect(m_iface, SIGNAL(stateChanged(NetworkManager::Device::State,NetworkManager::Device::State,NetworkManager::Device::StateChangeReason)), this, SLOT(handleConnectionStateChange(NetworkManager::Device::State,NetworkManager::Device::State,NetworkManager::Device::StateChangeReason)));

    if (m_iface->type() == NetworkManager::Device::Ethernet ||
        m_iface->type() == NetworkManager::Device::Wifi) {
        connect(m_iface, SIGNAL(bitRateChanged(int)), this, SLOT(updateBitRate(int)));

        if (m_iface->type() == NetworkManager::Device::Wifi) {
            connect(m_iface, SIGNAL(activeAccessPointChanged(QString)), this, SLOT(updateActiveAccessPoint(QString)));
        }
    }
    if (m_iface->type() == NetworkManager::Device::Modem ||
        m_iface->type() == NetworkManager::Device::Bluetooth
       ) {
            NetworkManager::ModemDevice *giface = qobject_cast<NetworkManager::ModemDevice*>(m_iface);

            if (giface) {
                ModemManager::ModemGsmNetworkInterface *modemNetworkIface = giface->getModemNetworkIface();

                if (modemNetworkIface) {
                    // this one is for bluetooth devices, which always have a NetworkManager object but do not always have a ModemManager object.
                    connect(ModemManager::notifier(), SIGNAL(modemRemoved(QString)), this, SLOT(resetInterfaceDetails()));
                    connect(modemNetworkIface, SIGNAL(enabledChanged(bool)), this, SLOT(modemUpdateEnabled(bool)));
                    connect(modemNetworkIface, SIGNAL(unlockRequiredChanged(QString)), this, SLOT(modemUpdateUnlockRequired(QString)));

                    connect(modemNetworkIface, SIGNAL(registrationInfoChanged(ModemManager::ModemGsmNetworkInterface::RegistrationInfoType)), this, SLOT(modemUpdateRegistrationInfo(ModemManager::ModemGsmNetworkInterface::RegistrationInfoType)));
                    connect(modemNetworkIface, SIGNAL(accessTechnologyChanged(ModemManager::ModemInterface::AccessTechnology)), this, SLOT(modemUpdateAccessTechnology(ModemManager::ModemInterface::AccessTechnology)));
                    connect(modemNetworkIface, SIGNAL(signalQualityChanged(uint)), this, SLOT(modemUpdateSignalQuality(uint)));
                    connect(modemNetworkIface, SIGNAL(allowedModeChanged(ModemManager::ModemInterface::AllowedMode)), this, SLOT(modemUpdateAllowedMode(ModemManager::ModemInterface::AllowedMode)));
                }
            }
    }
}

void InterfaceDetailsWidget::disconnectSignals()
{
    if (!m_iface) {
        return;
    }

    disconnect(m_iface, 0, this, 0);

    if (m_iface && (m_iface->type() == NetworkManager::Device::Modem
                 || m_iface->type() == NetworkManager::Device::Bluetooth
       )) {
        NetworkManager::ModemDevice *giface = qobject_cast<NetworkManager::ModemDevice*>(m_iface);

        if (giface) {
            ModemManager::ModemGsmNetworkInterface *modemNetworkIface = giface->getModemNetworkIface();

            if (modemNetworkIface) {
                disconnect(modemNetworkIface, 0, this, 0);
            }
        }
    }
}

void InterfaceDetailsWidget::updateIPv4Details()
{
    if (!m_iface) {
        details->ipv4Address.clear();
        details->ipv4Gateway.clear();
        return;
    }

    if (static_cast<NetworkManager::Device::State>(m_iface->state()) != NetworkManager::Device::Activated) {
        details->ipv4Address = i18nc("label of the network interface", "No IP address.");
        details->ipv4Gateway = i18nc("label of the network interface", "No gateway.");
        return;
    }

    QHostAddress addr;
    addr.setAddress(ntohl(m_iface->ipV4Address()));

    if (addr.isNull()) {
        details->ipv4Address = i18nc("label of the network interface", "IP display error.");
    } else {
        details->ipv4Address = addr.toString();
    }

    NetworkManager::Dhcp4Config *dhcp4Config = m_iface->dhcp4Config();
    if (dhcp4Config) {
        details->ipv4Gateway = dhcp4Config->optionValue("routers");
    } else if (!m_iface->ipV4Config().routes().isEmpty()) {
        // OBS: NetworkManager does not report routes if connection uses static IP address (tested with NetworkManager-0.9.4.0)
        QHostAddress addr;
        addr.setAddress(m_iface->ipV4Config().routes().at(0).route());
        details->ipv4Gateway = addr.toString();
    } else {
        details->ipv4Gateway.clear();
    }
    //showDetails();
}

void InterfaceDetailsWidget::updateBitRate(int bitRate)
{
    details->bitRate = bitRate;
    showDetails();
}

void InterfaceDetailsWidget::updateActiveAccessPoint(const QString &ap)
{
    details->activeAccessPoint = ap;
    showDetails();
}


void InterfaceDetailsWidget::modemUpdateEnabled(const bool enabled)
{
    details->enabled = enabled;
    showDetails();
}

void InterfaceDetailsWidget::modemUpdateUnlockRequired(const QString & codeRequired)
{
    details->unlockRequired = codeRequired;
    showDetails();
}

void InterfaceDetailsWidget::modemUpdateBand()
{
    NetworkManager::ModemDevice *giface = qobject_cast<NetworkManager::ModemDevice*>(m_iface);
    if (giface) {
        ModemManager::ModemGsmNetworkInterface *modemNetworkIface = giface->getModemNetworkIface();
        if (modemNetworkIface) {
            details->band = modemNetworkIface->getBand();
        }
    }
}

void InterfaceDetailsWidget::modemUpdateRegistrationInfo(const ModemManager::ModemGsmNetworkInterface::RegistrationInfoType & registrationInfo)
{
    modemUpdateBand();
    details->registrationInfo = registrationInfo;
    showDetails();
}

void InterfaceDetailsWidget::modemUpdateAccessTechnology(const ModemManager::ModemInterface::AccessTechnology & tech)
{
    modemUpdateBand();
    details->accessTechnology = tech;
    showDetails();
}

void InterfaceDetailsWidget::modemUpdateSignalQuality(const uint signalQuality)
{
    details->signalQuality = signalQuality;
    showDetails();
}

void InterfaceDetailsWidget::modemUpdateAllowedMode(const ModemManager::ModemInterface::AllowedMode mode)
{
    details->allowedMode = mode;
    showDetails();
}

// vim: sw=4 sts=4 et tw=100
