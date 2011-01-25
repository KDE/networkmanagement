/*
Copyright 2010 Lamarque Souza <lamarque@gmail.com>

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

#include <QFile>
#include <QTextStream>

#include <KDebug>

#include "mobileproviders.h"

#ifndef MOBILE_BROADBAND_PROVIDER_INFO
#define MOBILE_BROADBAND_PROVIDER_INFO "/usr/share/mobile-broadband-provider-info/serviceproviders.xml"
#endif

const QString MobileProviders::CountryCodesFile = "/usr/share/zoneinfo/iso3166.tab";
const QString MobileProviders::ProvidersFile = MOBILE_BROADBAND_PROVIDER_INFO;

MobileProviders::MobileProviders()
{
    QFile file(CountryCodesFile);
    mError = Success;

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith('#')) {
                continue;
            }
            QStringList pieces = line.split('\t');
            mCountries.insert(pieces.at(0), pieces.at(1));
        }
        file.close();
    } else {
        mError = CountryCodesMissing;
    }

    QFile file2(ProvidersFile);

    if (file2.open(QIODevice::ReadOnly)) {
        if (mDocProviders.setContent(&file2)) {
            docElement = mDocProviders.documentElement();

            if (docElement.isNull()) {
                kDebug() << ProvidersFile << ": document is null";
                mError = ProvidersIsNull;
            } else {
                if (docElement.isNull() || docElement.tagName() != "serviceproviders") {
                    kDebug() << ProvidersFile << ": wrong format";
                    mError = ProvidersWrongFormat;
                } else {
                    if (docElement.attribute("format") != "2.0") {
                        kDebug() << ProvidersFile << ": mobile broadband provider database format '" << docElement.attribute("format") << "' not supported.";
                        mError = ProvidersFormatNotSupported;
                    } else {
                        //kDebug() << "Everything is alright so far";
                    }
                }
            }
        }

        file2.close();
    } else {
        kDebug() << "Error opening providers file" << ProvidersFile;
        mError = ProvidersMissing;
    }
}

MobileProviders::~MobileProviders()
{
}

QStringList MobileProviders::getCountryList()
{
    QStringList temp = mCountries.values();
    temp.sort();
    return temp;
}

QString MobileProviders::countryFromLocale()
{
    QString lang(getenv("LC_ALL"));

    if (lang.isEmpty()) {
        lang = QString(getenv("LANG"));
    }
    if (lang.contains('_')) {
        lang = lang.section('_', 1);
    }
    if (lang.contains('.')) {
        lang = lang.section('.', 0, 0);
    }
    return lang.toUpper();
}

QStringList MobileProviders::getProvidersList(QString country, const Solid::Control::NetworkInterface::Type type)
{
    mProvidersGsm.clear();
    mProvidersCdma.clear();
    QDomNode n = docElement.firstChild();

    // country is a country name and we parse country codes.
    if (country.length() > 2) {
        country = mCountries.key(country);
    }
    while (!n.isNull()) {
        QDomElement e = n.toElement(); // <country ...>

        if (!e.isNull() && e.attribute("code").toUpper() == country) {
            QDomNode n2 = e.firstChild();
            while (!n2.isNull()) {
                QDomElement e2 = n2.toElement(); // <provider ...>

                if (!e2.isNull() && e2.tagName().toLower() == "provider") {
                    QDomNode n3 = e2.firstChild();
                    QString m_provider;
                    QDomNode m_sibling;
                    bool localized = false;
                    while (!n3.isNull()) {
                        QDomElement e3 = n3.toElement(); // <name | gsm | cdma>

                        if (!e3.isNull()) {
                            if (e3.tagName().toLower() == "gsm") {
                                mProvidersGsm.insert(m_provider, m_sibling);
                            } else if (e3.tagName().toLower() == "cdma") {
                                mProvidersCdma.insert(m_provider, m_sibling);
                            } else if (e3.tagName().toLower() == "name") {
                                if (e3.attribute("xml:lang", "") == country.toLower()) {
                                    m_provider = e3.text();
                                    localized = true;
                                } else if (!localized) {
                                    m_provider = e3.text();
                                }
                                m_sibling = n3.nextSibling();
                            }
                        }
                        n3 = n3.nextSibling();
                    }
                }
                n2 = n2.nextSibling();
            }
            break;
        }
        n = n.nextSibling();
    }

    QStringList temp;
    if (type == Solid::Control::NetworkInterface::Gsm) {
        temp = mProvidersGsm.keys();
    } else if (type == Solid::Control::NetworkInterface::Cdma) {
        temp = mProvidersCdma.keys();
    }
    temp.sort();
    return temp;
}

QStringList MobileProviders::getApns(const QString provider)
{
    mApns.clear();
    mNetworkIds.clear();
    if (!mProvidersGsm.contains(provider)) {
        return QStringList();
    }

    QDomNode n = mProvidersGsm[provider];

    while (!n.isNull()) {
        QDomElement e = n.toElement(); // <gsm | cdma>

        if (!e.isNull() && e.tagName().toLower() == "gsm") {
            QDomNode n2 = e.firstChild();
            while (!n2.isNull()) {
                QDomElement e2 = n2.toElement(); // <apn | network-id>

                if (!e2.isNull() && e2.tagName().toLower() == "apn") {
                    mApns.insert(e2.attribute("value"), e2.firstChild());
                } else if (!e2.isNull() && e2.tagName().toLower() == "network-id") {
                    mNetworkIds.append(e2.attribute("mcc") + "-" + e2.attribute("mnc"));
                }

                n2 = n2.nextSibling();
            }
        }
        n = n.nextSibling();
    }

    QStringList temp = mApns.keys();
    temp.sort();
    return temp;
}


QStringList MobileProviders::getNetworkIds(const QString provider)
{
    if (mNetworkIds.isEmpty()) {
        getApns(provider);
    }
    return mNetworkIds;
}

QVariantMap MobileProviders::getApnInfo(const QString apn)
{
    QVariantMap temp;
    QDomNode n = mApns[apn];
    QStringList dnsList;

    while (!n.isNull()) {
        QDomElement e = n.toElement(); // <name|username|password|dns(*)>

        if (!e.isNull()) {
            if (e.tagName().toLower() == "name") {
                temp.insert("name", e.text());
            } else if (e.tagName().toLower() == "username") {
                temp.insert("username", e.text());
            } else if (e.tagName().toLower() == "password") {
                temp.insert("password", e.text());
            } else if (e.tagName().toLower() == "dns") {
                dnsList.append(e.text());
            }
        }

        n = n.nextSibling();
    }

    temp.insert("number", getGsmNumber());
    temp.insert("apn", apn);
    temp.insert("dnsList", dnsList);

    return temp;
}

QVariantMap MobileProviders::getCdmaInfo(const QString provider)
{
    if (!mProvidersCdma.contains(provider)) {
        return QVariantMap();
    }

    QVariantMap temp;
    QDomNode n = mProvidersCdma[provider];
    QStringList sidList;

    while (!n.isNull()) {
        QDomElement e = n.toElement(); // <gsm or cdma ...>

        if (!e.isNull() && e.tagName().toLower() == "cdma") {
            QDomNode n2 = e.firstChild();
            while (!n2.isNull()) {
                QDomElement e2 = n2.toElement(); // <name | username | password | sid>

                if (!e2.isNull()) {
                    if (e2.tagName().toLower() == "name") {
                        temp.insert("name", e2.text());
                    } if (e2.tagName().toLower() == "username") {
                        temp.insert("username", e2.text());
                    } else if (e2.tagName().toLower() == "password") {
                        temp.insert("password", e2.text());
                    } else if (e2.tagName().toLower() == "sid") {
                        sidList.append(e2.text());
                    }
                }

                n2 = n2.nextSibling();
            }
        }
        n = n.nextSibling();
    }

    temp.insert("number", getCdmaNumber());
    temp.insert("sidList", sidList);
    return temp;
}
