/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.net>

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

#ifndef UIUTILS_P_H
#define UIUTILS_P_H

class UiUtilsPrivate
{
public:
    UiUtilsPrivate()
    {
        freqs_bgn.append(QPair<int, int>(1, 2412));
        freqs_bgn.append(QPair<int, int>(2, 2417));
        freqs_bgn.append(QPair<int, int>(3, 2422));
        freqs_bgn.append(QPair<int, int>(4, 2427));
        freqs_bgn.append(QPair<int, int>(5, 2432));
        freqs_bgn.append(QPair<int, int>(6, 2437));
        freqs_bgn.append(QPair<int, int>(7, 2442));
        freqs_bgn.append(QPair<int, int>(8, 2447));
        freqs_bgn.append(QPair<int, int>(9, 2452));
        freqs_bgn.append(QPair<int, int>(10, 2457));
        freqs_bgn.append(QPair<int, int>(11, 2462));
        freqs_bgn.append(QPair<int, int>(12, 2467));
        freqs_bgn.append(QPair<int, int>(13, 2472));
        freqs_bgn.append(QPair<int, int>(14, 2484));
        freqs_ahjn.append(QPair<int, int>(183, 4915));
        freqs_ahjn.append(QPair<int, int>(184, 4920));
        freqs_ahjn.append(QPair<int, int>(185, 4925));
        freqs_ahjn.append(QPair<int, int>(187, 4935));
        freqs_ahjn.append(QPair<int, int>(188, 4940));
        freqs_ahjn.append(QPair<int, int>(189, 4945));
        freqs_ahjn.append(QPair<int, int>(192, 4960));
        freqs_ahjn.append(QPair<int, int>(196, 4980));
        freqs_ahjn.append(QPair<int, int>(7, 5035));
        freqs_ahjn.append(QPair<int, int>(8, 5040));
        freqs_ahjn.append(QPair<int, int>(9, 5045));
        freqs_ahjn.append(QPair<int, int>(11, 5055));
        freqs_ahjn.append(QPair<int, int>(12, 5060));
        freqs_ahjn.append(QPair<int, int>(16, 5080));
        freqs_ahjn.append(QPair<int, int>(34, 5170));
        freqs_ahjn.append(QPair<int, int>(36, 5180));
        freqs_ahjn.append(QPair<int, int>(38, 5190));
        freqs_ahjn.append(QPair<int, int>(40, 5200));
        freqs_ahjn.append(QPair<int, int>(42, 5210));
        freqs_ahjn.append(QPair<int, int>(44, 5220));
        freqs_ahjn.append(QPair<int, int>(46, 5230));
        freqs_ahjn.append(QPair<int, int>(48, 5240));
        freqs_ahjn.append(QPair<int, int>(52, 5260));
        freqs_ahjn.append(QPair<int, int>(56, 5280));
        freqs_ahjn.append(QPair<int, int>(60, 5300));
        freqs_ahjn.append(QPair<int, int>(64, 5320));
        freqs_ahjn.append(QPair<int, int>(100, 5500));
        freqs_ahjn.append(QPair<int, int>(104, 5520));
        freqs_ahjn.append(QPair<int, int>(108, 5540));
        freqs_ahjn.append(QPair<int, int>(112, 5560));
        freqs_ahjn.append(QPair<int, int>(116, 5580));
        freqs_ahjn.append(QPair<int, int>(120, 5600));
        freqs_ahjn.append(QPair<int, int>(124, 5620));
        freqs_ahjn.append(QPair<int, int>(128, 5640));
        freqs_ahjn.append(QPair<int, int>(132, 5660));
        freqs_ahjn.append(QPair<int, int>(136, 5680));
        freqs_ahjn.append(QPair<int, int>(140, 5700));
        freqs_ahjn.append(QPair<int, int>(149, 5745));
        freqs_ahjn.append(QPair<int, int>(153, 5765));
        freqs_ahjn.append(QPair<int, int>(157, 5785));
        freqs_ahjn.append(QPair<int, int>(161, 5805));
        freqs_ahjn.append(QPair<int, int>(165, 5825));
    }
    QList<QPair<int, int> > getBFreqs()
    {
        return freqs_bgn;
    }
    QList<QPair<int, int> > getAFreqs()
    {
        return freqs_ahjn;
    }
private:
    QList<QPair<int, int> > freqs_bgn;
    QList< QPair<int, int> > freqs_ahjn;
};
#endif
