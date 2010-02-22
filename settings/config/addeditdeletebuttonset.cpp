/*
Copyright 2010 Aurélien Gâteau <aurelien.gateau@canonical.com>

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

#include "addeditdeletebuttonset.h"

#include <QVBoxLayout>

#include <KDebug>
#include <KLocale>
#include <KPushButton>

AddEditDeleteButtonSet::AddEditDeleteButtonSet(QWidget* parent)
: QWidget(parent)
, mAddButton(new KPushButton)
, mEditButton(new KPushButton)
, mDeleteButton(new KPushButton)
{
    mAddButton->setGuiItem(KGuiItem(i18n("Add..."), "list-add"));
    mEditButton->setGuiItem(KGuiItem(i18n("Edit..."), "configure"));
    mDeleteButton->setGuiItem(KStandardGuiItem::del());

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(mAddButton);
    layout->addWidget(mEditButton);
    layout->addWidget(mDeleteButton);
    layout->addStretch();
}
