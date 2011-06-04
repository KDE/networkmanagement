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

#include "connectionprefs.h"

#include <QFile>
#include <QVBoxLayout>

#include <KTabWidget>
#include <KDebug>
#include <KStandardDirs>

#include "connectionwidget.h"

#include "connection.h"
#include "knmserviceprefs.h"

ConnectionPreferences::ConnectionPreferences(const QVariantList &, QWidget * parent)
    : QWidget(parent),
      m_contents(new ConnectionWidget(this)), m_connection(0)
{
    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->addWidget(m_contents);

    connect(m_contents, SIGNAL(valid(bool)), this, SLOT(validate()));
}

ConnectionPreferences::~ConnectionPreferences()
{
}

Knm::Connection * ConnectionPreferences::connection() const
{
    return m_connection;
}

int ConnectionPreferences::addToTabWidget(SettingWidget * widget)
{
    int tabindex = -1;
    if (!m_settingWidgets.contains(widget)) {
        tabindex = m_contents->connectionSettingsWidget()->addTab(widget, widget->windowTitle());
        const bool v = widget->isValid();
        m_settingWidgets.insert(widget, v);

        connect(widget, SIGNAL(valid(bool)), this, SLOT(updateSettingValidation(bool)));
        validate();
    }
    return tabindex;
}

void ConnectionPreferences::updateSettingValidation(bool valid)
{
    SettingWidget * widget = static_cast<SettingWidget*>(sender());
    if (m_settingWidgets.contains(widget)) {
        m_settingWidgets.insert(widget, valid);
    }
    validate();
}

void ConnectionPreferences::validate()
{
    bool isValid = true;
    foreach (const bool validity, m_settingWidgets) {
        isValid &= validity;
    }
    if (m_contents)
        isValid &= m_contents->isValid();

    emit valid(isValid);
}

void ConnectionPreferences::load()
{
    // initialise the UI from the Connection
    m_contents->readConfig();
    foreach (SettingWidget * wid, m_settingWidgets.keys()) {
        wid->readConfig();
        wid->readSecrets(); //necessary to load secret info from connections
    }
}

void ConnectionPreferences::save()
{
    // save the UI to the Connection
    m_contents->writeConfig();
    foreach (SettingWidget * wid, m_settingWidgets.keys()) {
        wid->writeConfig();
    }
}

// vim: sw=4 sts=4 et tw=100


