// This file is generated by kconfig_compiler from ipv4.kcfg.
// All changes you do to this file will be lost.

#include "ipv4dbus.h"

#include <arpa/inet.h>
#include "../../types.h"
#include "ipv4.h"

Ipv4Dbus::Ipv4Dbus(Knm::Ipv4Setting * setting) : SettingDbus(setting)
{
}

Ipv4Dbus::~Ipv4Dbus()
{
}

void Ipv4Dbus::fromMap(const QVariantMap & map)
{

  kDebug() << "IPv4 map: " << map;

  Knm::Ipv4Setting * setting = static_cast<Knm::Ipv4Setting*>(m_setting); 

  if (map.contains("method")) {
      setting->setMethod(methodStringToEnum(map.value("method").value<QString>())); }

  if (map.contains("dns")) {
      QDBusArgument dnsArg = map.value("dns").value< QDBusArgument>();
      QList<QHostAddress> dbusDns;

      dnsArg.beginArray();
      while(!dnsArg.atEnd())
      {
          uint utmp = 0;
          dnsArg >> utmp;
          QHostAddress tmpHost(ntohl(utmp));
          dbusDns << tmpHost;
          kDebug() << "DNS IP is " << tmpHost.toString();
      }
      //NO dnsArg.endArray(); it's fatal in debug builds.

      // try QList<uint> if QDBusArgument demarshalling does not work
      // in toMap method we use QList<uint> to marshall DNS addresses, this hack makes
      // the DNS addresses we marshall at toMap possible to parse
      if (dbusDns.isEmpty())
      {
          QList <uint> dnsArg = map.value("dns").value< QList<uint> >();

          foreach (uint utmp, dnsArg)
          {
              QHostAddress tmpHost(ntohl(utmp));
              dbusDns << tmpHost;
              kDebug() << "DNS IP is " << tmpHost.toString();
          }
      }

      if (!dbusDns.isEmpty())
          setting->setDns(dbusDns);
  }

  if (map.contains(QLatin1String(NM_SETTING_IP4_CONFIG_DNS_SEARCH)) &&
      !map.value(QLatin1String(NM_SETTING_IP4_CONFIG_DNS_SEARCH)).value<QStringList>().isEmpty()
     ) {
    setting->setDnssearch(map.value(QLatin1String(NM_SETTING_IP4_CONFIG_DNS_SEARCH)).value<QStringList>());
  }

  if (map.contains("addresses")) {
      QDBusArgument addressArg = map.value("addresses").value< QDBusArgument>();
      QList<Solid::Control::IPv4Address> addresses;

      addressArg.beginArray();
      while(!addressArg.atEnd())
      {
          QList<uint> uintList;
          addressArg >> uintList;

          if (uintList.count() != 3)
          {
            kWarning() << "Invalid address format detected. UInt count is " << uintList.count();
            continue;
          }

          Solid::Control::IPv4Address addr((quint32)ntohl(uintList.at(0)), (quint32)uintList.at(1), (quint32) ntohl(uintList.at(2)));
          if (!addr.isValid())
          {
            kWarning() << "Invalid address format detected.";
            continue;
          }
          kDebug() << "IP Address:" << QHostAddress(ntohl(uintList.at(0))).toString() << " Subnet:" << uintList.at(1) << "Gateway:" << QHostAddress(ntohl(uintList.at(2))).toString();

          addresses << addr;
      }
      //NO addressArg.endArray(); it's fatal in debug builds.

      if (addresses.isEmpty()) // workaround for reading values come from our toMap function below
      {
          QList<QList <uint> > addressArgUint = map.value("addresses").value< QList <QList <uint> > >();
          foreach(QList<uint> uintList, addressArgUint)
          {
              if (uintList.count() != 3)
              {
                kWarning() << "Invalid address format detected. UInt count is " << uintList.count();
                continue;
              }

              Solid::Control::IPv4Address addr((quint32)ntohl(uintList.at(0)), (quint32)uintList.at(1), (quint32) ntohl(uintList.at(2)));
              if (!addr.isValid())
              {
                kWarning() << "Invalid address format detected.";
                continue;
              }
              kDebug() << "IP Address:" << QHostAddress(ntohl(uintList.at(0))).toString() << " Subnet:" << uintList.at(1) << "Gateway:" << QHostAddress(ntohl(uintList.at(2))).toString();

              addresses << addr;
          }
      }

      if (!addresses.isEmpty())
          setting->setAddresses(addresses);
  }

  if (map.contains(QLatin1String(NM_SETTING_IP4_CONFIG_IGNORE_AUTO_DNS))) {
    setting->setIgnoredhcpdns(map.value(QLatin1String(NM_SETTING_IP4_CONFIG_IGNORE_AUTO_DNS)).value<bool>());
  }
  if (map.contains(QLatin1String(NM_SETTING_IP4_CONFIG_IGNORE_AUTO_ROUTES))) {
    setting->setIgnoreautoroute(map.value(QLatin1String(NM_SETTING_IP4_CONFIG_IGNORE_AUTO_ROUTES)).value<bool>());
  }
  if (map.contains(QLatin1String(NM_SETTING_IP4_CONFIG_NEVER_DEFAULT))) {
    setting->setNeverdefault(map.value(QLatin1String(NM_SETTING_IP4_CONFIG_NEVER_DEFAULT)).value<bool>());
  }
  if (map.contains(QLatin1String(NM_SETTING_IP4_CONFIG_DHCP_CLIENT_ID))) {
    setting->setDhcpclientid(map.value(QLatin1String(NM_SETTING_IP4_CONFIG_DHCP_CLIENT_ID)).value<QString>());
  }
  if (map.contains(QLatin1String(NM_SETTING_IP4_CONFIG_DHCP_HOSTNAME))) {
    setting->setDhcphostname(map.value(QLatin1String(NM_SETTING_IP4_CONFIG_DHCP_HOSTNAME)).value<QString>());
  }
  if (map.contains(QLatin1String(NM_SETTING_IP4_CONFIG_MAY_FAIL))) {
    setting->setMayfail(map.value(QLatin1String(NM_SETTING_IP4_CONFIG_MAY_FAIL)).value<bool>());
  }
}

Knm::Ipv4Setting::EnumMethod::type Ipv4Dbus::methodStringToEnum(QString method)
{
    if (method.toLower() == "automatic" || method.toLower() == "auto")
        return Knm::Ipv4Setting::EnumMethod::Automatic;
    else if (method.toLower() == "linklocal" || method.toLower() == "link-local")
        return Knm::Ipv4Setting::EnumMethod::LinkLocal;
    else if (method.toLower() == "manual")
        return Knm::Ipv4Setting::EnumMethod::Manual;
    else if (method.toLower() == "shared")
        return Knm::Ipv4Setting::EnumMethod::Shared;
    else if (method.toLower() == "disabled")
        return Knm::Ipv4Setting::EnumMethod::Disabled;
    else
    {
        kDebug() << "Unknown method given:" << method;
        return Knm::Ipv4Setting::EnumMethod::Automatic;
    }
}

QVariantMap Ipv4Dbus::toMap()
{
  QVariantMap map;
  Knm::Ipv4Setting * setting = static_cast<Knm::Ipv4Setting *>(m_setting);
  switch (setting->method()) {
      case Knm::Ipv4Setting::EnumMethod::Automatic:
          map.insert("method", "auto");
          break;
      case Knm::Ipv4Setting::EnumMethod::LinkLocal:
          map.insert("method", "link-local");
          break;
      case Knm::Ipv4Setting::EnumMethod::Manual:
          map.insert("method", "manual");
          break;
      case Knm::Ipv4Setting::EnumMethod::Shared:
          map.insert("method", "shared");
          break;
       case Knm::Ipv4Setting::EnumMethod::Disabled:
          map.insert("method", "disabled");
          break;
  }

  if (!setting->dns().isEmpty()) {
      //FIXME: Use QDBusArgument instead of QList<uint> to remove hack in fromMap DNS code
      QList<uint> dbusDns;
      foreach (const QHostAddress &dns, setting->dns()) {
          dbusDns << htonl(dns.toIPv4Address());
      }
      map.insert("dns", QVariant::fromValue(dbusDns));
  }

  if (!setting->dnssearch().isEmpty()) {
      map.insert(QLatin1String(NM_SETTING_IP4_CONFIG_DNS_SEARCH), setting->dnssearch());
  }
  if (!setting->addresses().isEmpty()) {
      //FIXME: Use QDBusArgument instead of QList<QList<uint>> to remove hack in fromMap Ip Address code
      QList<QList<uint> > dbusAddresses;
      foreach (const Solid::Control::IPv4Address &addr, setting->addresses()) {
          QList<uint> dbusAddress;
          dbusAddress << htonl(addr.address())
              << addr.netMask()
              << htonl(addr.gateway());
          dbusAddresses << dbusAddress;
      }
      map.insert("addresses", QVariant::fromValue(dbusAddresses));
  }
  if (!setting->routes().isEmpty()) {
      QList<QList<uint> > dbusRoutes;
      foreach (const Solid::Control::IPv4Route &route, setting->routes()) {
          QList<uint> dbusRoute;
          dbusRoute << htonl(route.route())
              << route.prefix()
              << htonl(route.nextHop())
              << route.metric();
          dbusRoutes << dbusRoute;
      }

      map.insert("routes", QVariant::fromValue(dbusRoutes));
  }


  //map.insert(QLatin1String(NM_SETTING_IP4_CONFIG_IGNORE_AUTO_DNS), setting->ignoredhcpdns());
  insertIfTrue(map, NM_SETTING_IP4_CONFIG_IGNORE_AUTO_DNS, setting->ignoredhcpdns());
  insertIfTrue(map, NM_SETTING_IP4_CONFIG_IGNORE_AUTO_ROUTES, setting->ignoreautoroute());
  insertIfTrue(map, NM_SETTING_IP4_CONFIG_NEVER_DEFAULT, setting->neverdefault());
  insertIfNonEmpty(map, NM_SETTING_IP4_CONFIG_DHCP_CLIENT_ID, setting->dhcpclientid());
  insertIfNonEmpty(map, NM_SETTING_IP4_CONFIG_DHCP_HOSTNAME, setting->dhcphostname());
  insertIfTrue(map, NM_SETTING_IP4_CONFIG_MAY_FAIL, setting->mayfail());
  return map;
}

QVariantMap Ipv4Dbus::toSecretsMap()
{
  QVariantMap map;
  return map;
}

