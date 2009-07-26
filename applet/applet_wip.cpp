/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>
Copyright 2008 Sebastian Kügler <sebas@kde.org>

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

#include "applet_wip.h"
#include <QPainter>
#include <KIcon>
#include <KIconLoader>
#include <Plasma/ToolTipManager>

K_EXPORT_PLASMA_APPLET(networkmanagement, NetworkManagerApplet)

NetworkManagerApplet::NetworkManagerApplet(QObject * parent, const QVariantList & args)
    : Plasma::Applet(parent, args)
{
    setHasConfigurationInterface(false);

    setAspectRatioMode(Plasma::ConstrainedSquare);
    setHasConfigurationInterface(true);

    kDebug() << "Displaying Work In Progress plasmoid to protect the innocent from crashing their plasma and spamming bko";
}

NetworkManagerApplet::~NetworkManagerApplet()
{
}

void NetworkManagerApplet::init()
{
    m_svg = new Plasma::Svg(this);
    m_svg->setImagePath("networkmanagement/networkmanagement");

    Plasma::ToolTipContent tip(i18nc("Warning message to deter non-developers from using this", "Network Management is changing!"),
            i18nc("Tooltip sub text", "It is highly unstable and will crash your desktop.\n  Until further notice, please use KDE 4 knetworkmanager instead."),
            KIcon("networkmanager").pixmap(IconSize(KIconLoader::Desktop))
            );
    Plasma::ToolTipManager::self()->setContent(this, tip);
}

void NetworkManagerApplet::constraintsEvent(Plasma::Constraints constraints)
{
   if (constraints & (Plasma::SizeConstraint | Plasma::FormFactorConstraint)) {
        m_svg->resize(contentsRect().size().toSize());
    }
}

void NetworkManagerApplet::paintInterface(QPainter * p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option);
    m_svg->paint(p, contentsRect, "cellular_connected");
}
