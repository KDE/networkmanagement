/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

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

#ifndef WIRELESS802_11_WIDGET_H
#define WIRELESS802_11_WIDGET_H

#include <QSpinBox>
#include <QValidator>

#include "settingwidget.h"

#include "knm_export.h"

class Wireless80211WidgetPrivate;
class KNM_EXPORT Wireless80211Widget : public SettingWidget
{
Q_OBJECT
Q_DECLARE_PRIVATE(Wireless80211Widget)
public:
    Wireless80211Widget(Knm::Connection * connection, const QString &ssid = QString(), QWidget * parent = 0 );
    virtual ~Wireless80211Widget();
    QString settingName() const;
    void readConfig();
    void writeConfig();
    QByteArray selectedInterfaceHardwareAddress() const;
    bool enteredSsidIsDirty() const;
    void setEnteredSsidClean();
    QString enteredSsid() const;
protected Q_SLOTS:
    void scanClicked();
    void validate();
    void modeChanged(int);
    void bandChanged(int);
    void copyToBssid();

Q_SIGNALS:
    void ssidSelected(const QString & ssid);
};

class Wireless80211WidgetBand : public QSpinBox
{
Q_OBJECT
public:
    Wireless80211WidgetBand(QWidget * parent = 0);
    QString textFromValue(int) const;
    int valueFromText(const QString&) const;
    QValidator::State validate(QString&, int&) const;
    int channelFromPos(int) const;
    int posFromChannel(int) const;
    void setBand(int);
private:
    QList< QList<int> > channels;
    int selectedBand;
};

#endif // 802_11_WIRELESSWIDGET_H
