/*
Copyright 2010 Sebastian Kügler <sebas@kde.org>
Copyright 2010 Alexander Naumov <posix.ru@gmail.com>

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
#include <Solid/Device>

#include <uiutils.h>

#include "interfaceitem.h"

InterfaceDetailsWidget::InterfaceDetailsWidget(QGraphicsItem * parent) : QGraphicsWidget(parent, 0),
    m_iface(0)
{
    m_gridLayout = new QGraphicsGridLayout(this);

    int row = 0;
    //Info
    //row++;
    m_info = new Plasma::Label(this);
    m_info->setFont(KGlobalSettings::smallestReadableFont());
    // FIXME 4.5: setWordWrap() is now a method in Plasma::Label, we're keeping this
    // for a while for backwards compat though. Remove the nativeWidget() call in between
    // when we depend on 4.5
    m_info->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
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
    //m_rxColor = QColor("lightblue"); // blue
    //m_rxColor.setAlphaF(0.6);
    m_txColor = QColor("#91FF00"); // yellow
    //m_txColor = QColor("lightgreen"); // yellow
    m_txColor.setAlphaF(0.6);
    m_trafficPlotter = new Plasma::SignalPlotter(this);
    m_trafficPlotter->setMinimumHeight(50);
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
    m_backButton->setToolTip(i18n("back"));

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

    updateInfo(true);

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

void InterfaceDetailsWidget::updateInfo(bool reset)
{
    QString info;
    QString na = i18nc("entry not available", "not available");
    QString format = "<tr><td align=\"right\" width=\"50%\" style=\"white-space: nowrap\"><b>%1:</b></td><td width=\"50%\">&nbsp;%2</td></tr>";

    // generate html table header
    info = QLatin1String("<qt><table align=\"center\" border=\"0\">");

    if (!reset && m_iface) {
        info += QString(format)
                       .arg(i18nc("interface details", "Type"))
                       .arg(UiUtils::interfaceTypeLabel(m_iface->type()));
        info += QString(format)
                       .arg(i18nc("interface details", "Connection State"))
                       .arg(UiUtils::connectionStateToString(m_iface->connectionState()));
        info += QString(format)
                       .arg(i18nc("interface details", "IP Address"))
                       .arg(currentIpAddress());
        info += QString(format)
                       .arg(i18nc("interface details", "Connection Speed"))
                       .arg(bitRate());
        info += QString(format)
                       .arg(i18nc("interface details", "System Name"))
                       .arg(m_iface->interfaceName());
        info += QString(format)
                       .arg(i18nc("interface details", "MAC Address"))
                       .arg(getMAC());
        info += QString(format)
                       .arg(i18nc("interface details", "Driver"))
                       .arg(m_iface->driver());
    } else {
        info += QString(format)
                       .arg(i18nc("interface details", "Type"))
                       .arg(na);
        info += QString(format)
                       .arg(i18nc("interface details", "Connection State"))
                       .arg(na);
        info += QString(format)
                       .arg(i18nc("interface details", "Network Address (IP)"))
                       .arg(na);
        info += QString(format)
                       .arg(i18nc("interface details", "Connection Speed"))
                       .arg(na);
        info += QString(format)
                       .arg(i18nc("interface details", "System Name"))
                       .arg(na);
        info += QString(format)
                       .arg(i18nc("interface details", "Hardware Address (MAC)"))
                       .arg(na);
        info += QString(format)
                       .arg(i18nc("interface details", "Driver"))
                       .arg(na);
    }

    // For same reason the last row sometimes is not shown when there is a certain number of rows in the html table.
    format.remove(QChar(':'));
    info += QString(format).arg("").arg("");

    info += QLatin1String("</table></qt>");
    m_info->setText(info);
}

QString InterfaceDetailsWidget::currentIpAddress()
{
    if (m_iface && m_iface->connectionState() != Solid::Control::NetworkInterface::Activated) {
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

QString InterfaceDetailsWidget::bitRate()
{
    int bitRate = 0;

    //wifi?
    Solid::Control::WirelessNetworkInterface * wliface =
                    dynamic_cast<Solid::Control::WirelessNetworkInterface*>(m_iface);
    if (wliface) {
        bitRate = wliface->bitRate(); //Bit
    } else {     // wired?
        Solid::Control::WiredNetworkInterface * wdiface =
                                    dynamic_cast<Solid::Control::WiredNetworkInterface*> (m_iface);
        if (wdiface) {
            bitRate = wdiface->bitRate();
        }
    }
    if (bitRate) {
        return UiUtils::connectionSpeed(bitRate);
    } else {
        return i18nc("bitrate", "Unknown");
    }
}

void InterfaceDetailsWidget::sourceAdded(const QString &source)
{
    kDebug() << "Source added:" << source;
}

InterfaceDetailsWidget::~InterfaceDetailsWidget()
{
}

void InterfaceDetailsWidget::setUpdateEnabled(bool enable)
{
    // disconnect / connect goes here
    Plasma::DataEngine *e = engine();
    if (e) {
        int interval = 1000;
        if (enable) {
            if (m_iface) {
                kDebug() << "connecting ..." << m_rxSource << m_txSource;
                e->connectSource(m_rxSource, this, interval);
                e->connectSource(m_txSource, this, interval);
                e->connectSource(m_rxTotalSource, this, interval);
                e->connectSource(m_txTotalSource, this, interval);
            }
        } else {
            kDebug() << "disconnecting ..." << m_rxSource << m_txSource;
            e->disconnectSource(m_rxSource, this);
            e->disconnectSource(m_txSource, this);
            e->disconnectSource(m_rxTotalSource, this);
            e->disconnectSource(m_txTotalSource, this);
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

    temp = QString("<qt><table align=\"center\" border=\"0\"><tr><td align=\"right\" width=\"50%\">");
    temp += QString(format).arg(i18nc("", "Received")).arg(KGlobal::locale()->formatByteSize(m_rxTotal*1000, 2));
    temp += QString("</td><td width=\"50%\">&nbsp;");
    temp += QString(format).arg(i18nc("", "Transmitted")).arg(KGlobal::locale()->formatByteSize(m_txTotal*1000, 2));
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
    if ((new_state == Solid::Control::NetworkInterface::Unavailable ||
                     Solid::Control::NetworkInterface::Unmanaged ||
                     Solid::Control::NetworkInterface::UnknownState) &&
        reason == (Solid::Control::NetworkInterface::UnknownReason ||
	           Solid::Control::NetworkInterface::DeviceRemovedReason)) {
        setInterface(0);
    } else {
        updateInfo(false);
    }
}

void InterfaceDetailsWidget::setInterface(Solid::Control::NetworkInterface* iface)
{
    if (m_iface == iface) {
        return;
    }
    resetUi();
    if (iface) {
        if (m_iface) {
            disconnect(m_iface, SIGNAL(connectionStateChanged(int,int,int)), this, SLOT(handleConnectionStateChange(int,int,int)));
        }

        m_iface = iface;
        m_ifaceUni = iface->uni();
        updateInfo(false);
        connect(m_iface, SIGNAL(connectionStateChanged(int,int,int)), this, SLOT(handleConnectionStateChange(int,int,int)));

        m_rxSource = QString("network/interfaces/%1/receiver/data").arg(m_iface->interfaceName());
        m_txSource = QString("network/interfaces/%1/transmitter/data").arg(m_iface->interfaceName());
        m_rxTotalSource = QString("network/interfaces/%1/receiver/dataTotal").arg(m_iface->interfaceName());
        m_txTotalSource = QString("network/interfaces/%1/transmitter/dataTotal").arg(m_iface->interfaceName());
    } else {
        m_iface = iface;
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
    if (!m_iface) {
        return QString();
    }

    QList<Solid::Device> list = Solid::Device::listFromQuery(QString::fromLatin1("NetworkInterface.ifaceName == '%1'").arg(m_iface->interfaceName()));
    QList<Solid::Device>::iterator it = list.begin();

    if (it != list.end()) {
        Solid::Device device = *it;
        Solid::DeviceInterface *interface = it->asDeviceInterface(Solid::DeviceInterface::NetworkInterface);

        if (interface) {
            const QMetaObject *meta = interface->metaObject();

            for (int i = meta->propertyOffset(); i<meta->propertyCount(); i++) {
                QMetaProperty property = meta->property(i);

                if (QString(meta->className()).mid(7) + "." + property.name() == QString::fromLatin1("NetworkInterface.hwAddress")) {
                    QVariant value = property.read(interface);
                    return value.toString();
                    break;
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

QSizeF InterfaceDetailsWidget::sizeHint (Qt::SizeHint which, const QSizeF & constraint) const
{
    QSizeF sh = QGraphicsWidget::sizeHint(which, constraint);
    QSize infoSh = m_info->nativeWidget()->sizeHint();
    QSize infoMinSh = m_info->nativeWidget()->minimumSizeHint();

    qreal temp = (infoSh.width() - infoMinSh.width()) / 2 + infoMinSh.width();
    sh.setWidth(qMax(temp, 330.0));

    return sh;
}
// vim: sw=4 sts=4 et tw=100
