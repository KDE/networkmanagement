/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#include <KAboutData>
#include <KCmdLineArgs>

#include "monolithic.h"

static const char description[] =
    I18N_NOOP("KNetworkManager, the KDE 4 NetworkManager client");

static const char version[] = "v0.8";

int main( int argc, char** argv )
{
    KAboutData about("KNetworkManager", 0, ki18n("knetworkmanager"), version, ki18n(description), KAboutData::License_GPL, ki18n("(C) 2009 Will Stephenson"), KLocalizedString(), 0, "wstephenson@kde.org");
    about.addAuthor( ki18n("Will Stephenson"), KLocalizedString(), "wstephenson@kde.org" );
    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    KCmdLineArgs::addCmdLineOptions(options);
    Monolithic app;
    app.setQuitOnLastWindowClosed(false);
    app.init();

    int i = app.exec();
    // workaround KNotificationItem crashing when deleted from QCoreApp dtor
    return i;
}
