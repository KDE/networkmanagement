/*
Copyright 2010 Sebastian Kügler <sebas@kde.org>
Copyright 2010 Alexander Naumov <posix.ru@gmail.com>
Copyright 2010-2011 Lamarque Souza <lamarque@gmail.com>

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
#include <interfacedetailswidget.h>

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
#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirednetworkinterface.h>
#include <solid/control/networkipv4config.h>
#include <solid/control/networkinterface.h>
#include <solid/control/networkmanager.h>
#ifdef COMPILE_MODEM_MANAGER_SUPPORT
    #include <solid/control/modemmanager.h>
#endif
#ifdef NM_0_8
    #include <solid/control/networkbtinterface.h>
#endif
#include <Solid/Device>

#include <uiutils.h>

#include "interfaceitem.h"

class InterfaceDetails
{
    public:
        Solid::Control::NetworkInterface::Type type;
        Solid::Control::NetworkInterface::ConnectionState connectionState;
        QString ipAddress;
        int bitRate;
        QString interfaceName;
        QString mac;
        QString driver;
        QString activeAccessPoint;

#ifdef COMPILE_MODEM_MANAGER_SUPPORT
        Solid::Control::ModemGsmNetworkInterface::RegistrationInfoType registrationInfo;
        uint signalQuality;
        Solid::Control::ModemInterface::Type modemType;
        Solid::Control::ModemInterface::AccessTechnology accessTechnology;
        Solid::Control::ModemInterface::Band band;
        Solid::Control::ModemInterface::AllowedMode allowedMode;
        bool enabled;
        QString udi;
        QString device;
        QString masterDevice;
        QString unlockRequired;
        QString imei;
        QString imsi;
#endif
};

InterfaceDetailsWidget::InterfaceDetailsWidget(QGraphicsItem * parent) : QGraphicsWidget(parent, 0),
    m_iface(0), m_updateEnabled(false)
{
    m_gridLayout = new QGraphicsGridLayout(this);

    int row = 0;
    //Info
    //row++;
    m_info = new Plasma::Label(this);
    m_info->nativeWidget()->setTextFormat(Qt::RichText);
    m_info->setFont(KGlobalSettings::smallestReadableFont());
#if KDE_IS_VERSION(4, 5, 0)
    m_info->setTextSelectable(true);
#else
    // FIXME 4.5: setWordWrap() is now a method in Plasma::Label, we're keeping this
    // for a while for backwards compat though. Remove the nativeWidget() call in between
    // when we depend on 4.5
    m_info->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
#endif
    m_info->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
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
    m_trafficPlotter->setMinimumHeight(4 * QFontMetrics(KGlobalSettings::smallestReadableFont()).height());

    m_gridLayout->addItem(m_trafficPlotter, row, 0, 1, 2);

    row++;
    m_traffic = new Plasma::Label(this);
    m_traffic->setFont(KGlobalSettings::smallestReadableFont());
    m_traffic->nativeWidget()->setWordWrap(false);
    m_traffic->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_gridLayout->addItem(m_traffic, row, 0);

    // add pushbutton for "back" action
    m_backButton = new Plasma::PushButton(this);
    m_backButton->setMaximumHeight(22);
    m_backButton->setMaximumWidth(22);
    m_backButton->setIcon(KIcon("go-previous"));
    m_backButton->setToolTip(i18n("Go Back"));

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

    //connect(e, SIGNAL(sourceAdded(const QString&)), this, SLOT(sourceAdded(const QString&)));

    details = new InterfaceDetails();
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
    getDetails();
    showDetails();
}

void InterfaceDetailsWidget::getDetails()
{
    if (!m_iface) {
        return;
    }

    details->type = m_iface->type();
    details->connectionState = m_iface->connectionState();
    details->ipAddress = currentIpAddress();
    details->bitRate = bitRate();
#ifdef NM_0_8
    details->interfaceName = m_iface->ipInterfaceName();
    if (details->interfaceName.isEmpty()) {
        details->interfaceName = m_iface->interfaceName();
    }
#else
    details->interfaceName = m_iface->interfaceName();
#endif
    details->mac = getMAC();
    details->driver = m_iface->driver();

    Solid::Control::WirelessNetworkInterface *wiface = qobject_cast<Solid::Control::WirelessNetworkInterface*>(m_iface);
    if (wiface) {
        details->activeAccessPoint = wiface->activeAccessPoint();
    } else {
        details->activeAccessPoint = QString();
    }

#ifdef COMPILE_MODEM_MANAGER_SUPPORT
    Solid::Control::GsmNetworkInterface *giface = qobject_cast<Solid::Control::GsmNetworkInterface*>(m_iface);
    if (giface) {
        Solid::Control::ModemGsmNetworkInterface *modemNetworkIface = giface->getModemNetworkIface();
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

        /*
         * These calls are protectec by policy kit. Without proper configuration policy kit agent
         * will ask por password, which is bothering users (BUG #266807).
         * Plasma NM still does not use this information, so I will leave them
         * commented until I really need them.
        Solid::Control::ModemGsmCardInterface *modemCardIface = giface->getModemCardIface();
        if (modemCardIface) {
            details->imei = modemCardIface->getImei();
            details->imsi = modemCardIface->getImsi();
        }*/
    }
#endif
}

QString InterfaceDetailsWidget::connectionStateToString(Solid::Control::NetworkInterface::ConnectionState connectionState)
{
#ifdef COMPILE_MODEM_MANAGER_SUPPORT
    Solid::Control::GsmNetworkInterface *giface = qobject_cast<Solid::Control::GsmNetworkInterface*>(m_iface);
    if (giface && !details->enabled) {
        return i18nc("state of mobile broadband connection", "not enabled");
    }
#endif

    return UiUtils::connectionStateToString(connectionState);
}

void InterfaceDetailsWidget::showDetails(bool reset)
{
    QString info;
    QString na = i18nc("entry not available", "not available");
    QString format = "<tr><td align=\"right\" width=\"30%\" style=\"white-space: nowrap\"><b>%1:</b></td><td width=\"30%\">&nbsp;%2</td></tr>";

    // generate html table header
    info = QString("<qt><table align=\"center\" border=\"0\">");

    if (!reset && m_iface) {
        info += QString(format).arg(i18nc("interface details", "Type"), UiUtils::interfaceTypeLabel(details->type));
        info += QString(format).arg(i18nc("interface details", "Connection State"), connectionStateToString(details->connectionState));
        info += QString(format).arg(i18nc("interface details", "IP Address"), details->ipAddress);
        info += QString(format).arg(i18nc("interface details", "Connection Speed"), details->bitRate ? UiUtils::connectionSpeed(details->bitRate) : i18nc("bitrate", "Unknown"));
        info += QString(format).arg(i18nc("interface details", "System Name"), details->interfaceName);
        info += QString(format).arg(i18nc("interface details", "MAC Address"), details->mac);
        info += QString(format).arg(i18nc("interface details", "Driver"), details->driver);

        Solid::Control::WirelessNetworkInterface *wiface = qobject_cast<Solid::Control::WirelessNetworkInterface*>(m_iface);
        if (wiface) {
            Solid::Control::AccessPoint *ap = wiface->findAccessPoint(details->activeAccessPoint);
            if (ap) {
                info += QString(format).arg(i18nc("interface details", "Access Point (SSID)"), ap->ssid());
                info += QString(format).arg(i18nc("interface details", "Access Point (MAC)"), ap->hardwareAddress());
            }
        }

#ifdef COMPILE_MODEM_MANAGER_SUPPORT
        Solid::Control::GsmNetworkInterface *giface = qobject_cast<Solid::Control::GsmNetworkInterface*>(m_iface);

        if (giface) {
            info += QString(format).arg(i18nc("interface details", "Operator"), details->registrationInfo.operatorName);
            info += QString(format).arg(i18nc("interface details", "Signal Quality"), QString("%1 %").arg(details->signalQuality));
            info += QString(format).arg(i18nc("interface details", "Access Technology"), QString("%1/%2").arg(Solid::Control::ModemInterface::convertTypeToString(details->modemType), Solid::Control::ModemInterface::convertAccessTechnologyToString(details->accessTechnology)));

            /* TODO: create another tab to show this stuff
            info += QString(format).arg(i18nc("interface details", "Frequency Band"), Solid::Control::ModemInterface::convertBandToString(details->band));
            info += QString(format).arg(i18nc("interface details", "Allowed Mode"), Solid::Control::ModemInterface::convertAllowedModeToString(details->allowedMode));
            info += QString(format).arg(i18nc("interface details", "UDI"), details->udi);
            info += QString(format).arg(i18nc("interface details", "Device"), details->device);
            info += QString(format).arg(i18nc("interface details", "Master Device"), details->masterDevice);
            info += QString(format).arg(i18nc("interface details", "Unlock Required"), details->unlockRequired.isEmpty() ? i18n("No") : QString("%1: %2").arg(i18n("Yes"), details->unlockRequired));*/

            /* TODO: create another tab to show this stuff
            info += QString(format).arg(i18nc("interface details", "IMEI"), details->imei);
            info += QString(format).arg(i18nc("interface details", "IMSI"), details->imsi);
            */
        }
#endif
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
}

QString InterfaceDetailsWidget::currentIpAddress()
{
    if (!m_iface)
        return QString();

    if (m_iface->connectionState() != Solid::Control::NetworkInterface::Activated) {
        return i18nc("label of the network interface", "No IP address.");
    }
    Solid::Control::IPv4Config ip4Config = m_iface->ipV4Config();
    QList<Solid::Control::IPv4Address> addresses = ip4Config.addresses();
    if (addresses.isEmpty()) {
        return i18nc("label of the network interface", "IP display error.");
    }
    QHostAddress addr(addresses.first().address());
    return addr.toString();
}

int InterfaceDetailsWidget::bitRate()
{
    int bitRate = 0;

    //wifi?
    Solid::Control::WirelessNetworkInterface * wliface =
                    qobject_cast<Solid::Control::WirelessNetworkInterface*>(m_iface);
    if (wliface) {
        bitRate = wliface->bitRate(); //Bit
    } else {     // wired?
        Solid::Control::WiredNetworkInterface * wdiface =
                                    qobject_cast<Solid::Control::WiredNetworkInterface*> (m_iface);
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

    double _r = m_rx.toDouble();
    double _t = m_tx.toDouble();
    QList<double> v;
    v << _r << _t;
    m_trafficPlotter->addSample(v);
    m_trafficPlotter->setUnit(m_rxUnit);
    //m_trafficPlotter->setTitle(i18nc("traffic, e.g. n KB/s / m KB/s", "%1 %2 %3 %4", m_rx, m_rxUnit, m_tx, m_txUnit));

    QString r = KGlobal::locale()->formatByteSize(_r*1024);
    r.append("/s");
    QString t = KGlobal::locale()->formatByteSize(_t*1024);
    t.append("/s");
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

void InterfaceDetailsWidget::handleConnectionStateChange(int new_state, int old_state, int reason)
{
    Q_UNUSED(old_state)
    if ((new_state == Solid::Control::NetworkInterface::Unavailable ||
                     Solid::Control::NetworkInterface::Unmanaged ||
                     Solid::Control::NetworkInterface::UnknownState) &&
        reason == (Solid::Control::NetworkInterface::UnknownReason ||
                   Solid::Control::NetworkInterface::DeviceRemovedReason)) {
        setInterface(0, false);
        emit back();
    } else {
        details->ipAddress = currentIpAddress();
        details->connectionState = static_cast<Solid::Control::NetworkInterface::ConnectionState>(new_state);
#ifdef NM_0_8
        if (m_iface->type() == Solid::Control::NetworkInterface::Bluetooth) {
            QString interfaceName = m_iface->ipInterfaceName();
            if (interfaceName != details->interfaceName) {
                // Hack to force updating interfaceName and traffic plot source.
                Solid::Control::NetworkInterface *temp = m_iface;
                m_iface = 0;
                kDebug() << "Reseting interface " << temp->uni() << "(" << interfaceName << ")";
                setInterface(temp);
                setUpdateEnabled(m_updateEnabled);
            } else {
                showDetails();
            }
        } else
#endif
        showDetails();
    }
}

void InterfaceDetailsWidget::setInterface(Solid::Control::NetworkInterface* iface, bool disconnectOld)
{
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

#ifdef NM_0_8
        QString interfaceName = m_iface->ipInterfaceName();
        if (details->interfaceName.isEmpty()) {
            details->interfaceName = m_iface->interfaceName();
        }
#else
        QString interfaceName = m_iface->interfaceName();

        /* Ugly and error prone hack: if more than one 3G modem/cellphone is connected to the Internet this
         * will make Plasma NM always report connection speed for the first one. */
        if (interfaceName.contains("ttyACM") || interfaceName.contains("ttyUSB") || // USB modems
            interfaceName.contains("rfcomm")) { // bluetooth modems
            interfaceName = "ppp0";
        }
#endif

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
    Solid::Control::WirelessNetworkInterface * wliface =
                    qobject_cast<Solid::Control::WirelessNetworkInterface*>(m_iface);
    if (wliface) {
        return wliface->hardwareAddress();
        /*
         * for later use ...
        Solid::Control::AccessPoint * ap = wliface->findAccessPoint(wliface->activeAccessPoint());
        if(ap) {
            temp = ap->ssid();
            kDebug() << "temp = " << temp;
        }
        */
    } else {
#ifdef NM_0_8
        Solid::Control::BtNetworkInterface * btiface =
                    qobject_cast<Solid::Control::BtNetworkInterface*>(m_iface);
        if (btiface) {
            return btiface->interfaceName();
        }
#endif

        // wired?
        Solid::Control::WiredNetworkInterface * wdiface =
                                    qobject_cast<Solid::Control::WiredNetworkInterface*> (m_iface);
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
            
                            if (QString(QString(meta->className()).mid(7) + '.' + property.name()) == QString::fromLatin1("NetworkInterface.hwAddress")) {
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
    connect(m_iface, SIGNAL(connectionStateChanged(int,int,int)), this, SLOT(handleConnectionStateChange(int,int,int)));

    if (m_iface->type() == Solid::Control::NetworkInterface::Ieee8023 ||
        m_iface->type() == Solid::Control::NetworkInterface::Ieee80211) {
        connect(m_iface, SIGNAL(bitRateChanged(int)), this, SLOT(updateBitRate(int)));

        if (m_iface->type() == Solid::Control::NetworkInterface::Ieee80211) {
            connect(m_iface, SIGNAL(activeAccessPointChanged(const QString &)), this, SLOT(updateActiveAccessPoint(const QString &)));
        }
    }
#ifdef COMPILE_MODEM_MANAGER_SUPPORT
    if (m_iface->type() == Solid::Control::NetworkInterface::Gsm
#ifdef NM_0_8
        || m_iface->type() == Solid::Control::NetworkInterface::Bluetooth
#endif
       ) {
            Solid::Control::GsmNetworkInterface *giface = qobject_cast<Solid::Control::GsmNetworkInterface*>(m_iface);

            if (giface) {
                Solid::Control::ModemGsmNetworkInterface *modemNetworkIface = giface->getModemNetworkIface();

                if (modemNetworkIface) {
                    // this one is for bluetooth devices, which always have a NetworkManager object but do not always have a ModemManager object.
                    connect(Solid::Control::ModemManager::notifier(), SIGNAL(modemInterfaceRemoved(const QString &)), this, SLOT(resetInterfaceDetails()));
                    connect(modemNetworkIface, SIGNAL(enabledChanged(const bool)), this, SLOT(modemUpdateEnabled(const bool)));
                    connect(modemNetworkIface, SIGNAL(unlockRequiredChanged(const QString &)), this, SLOT(modemUpdateUnlockRequired(const QString &)));

                    connect(modemNetworkIface, SIGNAL(registrationInfoChanged(const Solid::Control::ModemGsmNetworkInterface::RegistrationInfoType &)), this, SLOT(modemUpdateRegistrationInfo(const Solid::Control::ModemGsmNetworkInterface::RegistrationInfoType &)));
                    connect(modemNetworkIface, SIGNAL(accessTechnologyChanged(Solid::Control::ModemInterface::AccessTechnology)), this, SLOT(modemUpdateAccessTechnology(const Solid::Control::ModemInterface::AccessTechnology &)));
                    connect(modemNetworkIface, SIGNAL(signalQualityChanged(const uint)), this, SLOT(modemUpdateSignalQuality(const uint)));
                    connect(modemNetworkIface, SIGNAL(allowedModeChanged(const Solid::Control::ModemInterface::AllowedMode)), this, SLOT(modemUpdateAllowedMode(const Solid::Control::ModemInterface::AllowedMode)));
                }
            }
    }
#endif
}

void InterfaceDetailsWidget::disconnectSignals()
{
    if (!m_iface) {
        return;
    }

    disconnect(m_iface, 0, this, 0);

#ifdef COMPILE_MODEM_MANAGER_SUPPORT
    if (m_iface && (m_iface->type() == Solid::Control::NetworkInterface::Gsm
#ifdef NM_0_8
                 || m_iface->type() == Solid::Control::NetworkInterface::Bluetooth
#endif
       )) {
        Solid::Control::GsmNetworkInterface *giface = qobject_cast<Solid::Control::GsmNetworkInterface*>(m_iface);

        if (giface) {
            Solid::Control::ModemGsmNetworkInterface *modemNetworkIface = giface->getModemNetworkIface();

            if (modemNetworkIface) {
                disconnect(modemNetworkIface, 0, this, 0);
            }
        }
    }
#endif
}

void InterfaceDetailsWidget::updateIpAddress()
{
    details->ipAddress = currentIpAddress();
    showDetails();
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


#ifdef COMPILE_MODEM_MANAGER_SUPPORT
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
    Solid::Control::GsmNetworkInterface *giface = qobject_cast<Solid::Control::GsmNetworkInterface*>(m_iface);
    if (giface) {
        Solid::Control::ModemGsmNetworkInterface *modemNetworkIface = giface->getModemNetworkIface();
        if (modemNetworkIface) {
            details->band = modemNetworkIface->getBand();
        }
    }
}

void InterfaceDetailsWidget::modemUpdateRegistrationInfo(const Solid::Control::ModemGsmNetworkInterface::RegistrationInfoType & registrationInfo)
{
    modemUpdateBand();
    details->registrationInfo = registrationInfo;
    showDetails();
}

void InterfaceDetailsWidget::modemUpdateAccessTechnology(const Solid::Control::ModemInterface::AccessTechnology & tech)
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

void InterfaceDetailsWidget::modemUpdateAllowedMode(const Solid::Control::ModemInterface::AllowedMode mode)
{
    details->allowedMode = mode;
    showDetails();
}
#endif

// vim: sw=4 sts=4 et tw=100
