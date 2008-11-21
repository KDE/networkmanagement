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

#include <KTabWidget>

#include <kcoreconfigskeleton.h>

#include "configxml.h"
#include "connectionwidget.h"

ConnectionPreferences::ConnectionPreferences(const KComponentData& cdata, QWidget * parent, const QVariantList & args) : KCModule( cdata, parent, args ),
    m_contents(0), m_connectionTypeWidget(0)
{

}

ConnectionPreferences::~ConnectionPreferences()
{
}

QString ConnectionPreferences::connectionType() const
{
    return m_connectionType;
#if 0
    KConfigSkeletonItem * configItem = m_contents->configXml()->findItem(m_contents->settingName(), "type");
    if (configItem) {
        return configItem->property().toString();
    } else {
        return QString();
    }
#endif
}

QString ConnectionPreferences::connectionName() const
{
    KConfigSkeletonItem * configItem = m_contents->configXml()->findItem(m_contents->settingName(), "id");
    if (configItem) {
        return configItem->property().toString();
    } else {
        return QString();
    }
}

void ConnectionPreferences::addToTabWidget(SettingWidget * wid)
{
    m_contents->connectionSettingsWidget()->addTab(wid,wid->windowTitle());
    addConfig(wid->configXml(), wid);
    m_settingWidgets.append(wid);
}

void ConnectionPreferences::load()
{
    // first, do the KCModule's load, to give the widgets' load routine a free hand
    KCModule::load();
    foreach (SettingWidget * wid, m_settingWidgets) {
        wid->readConfig();
    }
    // then read the connection settings
    m_contents->readConfig();
}

void ConnectionPreferences::save()
{
    // first, set the type on the connection settings
    // using the type specific widget set in derived classes
    Q_ASSERT( m_connectionTypeWidget);

    KConfigSkeletonItem * typeItem = m_contents->configXml()->findItem(QLatin1String("connection"), QLatin1String("type"));
    if (typeItem) {
        typeItem->setProperty(m_connectionTypeWidget->settingName());
    }

    // secondly, do the KCModule's save, to give the widgets' save routine a free hand
    KCModule::save();

    // thirdly, call each widget's custom save method
    foreach (SettingWidget * wid, m_settingWidgets) {
        wid->writeConfig();
    }
    // finally write the connection settings
    m_contents->writeConfig();
    m_contents->configXml()->config()->sync();
}

// vim: sw=4 sts=4 et tw=100


