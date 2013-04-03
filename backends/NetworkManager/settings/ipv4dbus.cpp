// This file is generated by kconfig_compiler from ipv4.kcfg.
// All changes you do to this file will be lost.

#include "ipv4dbus.h"

#include <arpa/inet.h>
#include <QtNetworkManager/generic-types.h>

#include "ipv4.h"

Ipv4Dbus::Ipv4Dbus(Knm::Ipv4Setting * setting) : SettingDbus(setting)
{
}

Ipv4Dbus::~Ipv4Dbus()
{
}

void Ipv4Dbus::fromMap(const QVariantMap & map)
{

  /*kDebug() << "IPv4 map: ";
  foreach(const QString & key, map.keys())
      kDebug() << key << " : " << map.value(key);*/

  Knm::Ipv4Setting * setting = static_cast<Knm::Ipv4Setting*>(m_setting);

  if (map.contains(QLatin1String(NM_SETTING_IP4_CONFIG_METHOD))) {
      setting->setMethod(methodStringToEnum(map.value(QLatin1String(NM_SETTING_IP4_CONFIG_METHOD)).value<QString>())); }

  if (map.contains(QLatin1String(NM_SETTING_IP4_CONFIG_DNS))) {
      QList<QHostAddress> dbusDns;
      QList<uint> temp;
      if (map.value(QLatin1String(NM_SETTING_IP4_CONFIG_DNS)).canConvert<QDBusArgument>()) {
          QDBusArgument dnsArg = map.value(QLatin1String(NM_SETTING_IP4_CONFIG_DNS)).value<QDBusArgument>();
          temp = qdbus_cast<QList<uint> >(dnsArg);
      } else {
          temp = map.value(QLatin1String(NM_SETTING_IP4_CONFIG_DNS)).value<QList<uint> >();
      }
    
      foreach(const uint utmp, temp) {
          QHostAddress tmpHost(ntohl(utmp));
          dbusDns << tmpHost;
          kDebug() << "DNS IP is " << tmpHost.toString();
      }
      //NO dnsArg.endArray(); it's fatal in debug builds.
      setting->setDns(dbusDns);
  }

  if (map.contains(QLatin1String(NM_SETTING_IP4_CONFIG_DNS_SEARCH)) &&
      !map.value(QLatin1String(NM_SETTING_IP4_CONFIG_DNS_SEARCH)).value<QStringList>().isEmpty()
     ) {
    setting->setDnssearch(map.value(QLatin1String(NM_SETTING_IP4_CONFIG_DNS_SEARCH)).value<QStringList>());
  }

  if (map.contains(QLatin1String(NM_SETTING_IP4_CONFIG_ADDRESSES))) {
      QList<NetworkManager::IpAddress> addresses;
      QList<QList<uint> > temp;
      if (map.value(QLatin1String(NM_SETTING_IP4_CONFIG_ADDRESSES)).canConvert< QDBusArgument>()) {
          QDBusArgument addressArg = map.value(QLatin1String(NM_SETTING_IP4_CONFIG_ADDRESSES)).value<QDBusArgument>();
          temp = qdbus_cast<QList<QList<uint> > >(addressArg);
      } else {
          temp = map.value(QLatin1String(NM_SETTING_IP4_CONFIG_ADDRESSES)).value<QList<QList<uint> > >();
      }

      foreach(const QList<uint> & uintList, temp) {
          if (uintList.count() != 3)
          {
            kWarning() << "Invalid address format detected. UInt count is " << uintList.count();
            continue;
          }

          NetworkManager::IpAddress address;
          address.setIp(QHostAddress((quint32)ntohl(uintList.at(0))));
          address.setPrefixLength((quint32)uintList.at(1));
          address.setGateway(QHostAddress((quint32) ntohl(uintList.at(2))));
          if (!address.isValid())
          {
            kWarning() << "Invalid address format detected.";
            continue;
          }
          kDebug() << "IP Address:" << address.ip().toString() << " Subnet:" << address.prefixLength() << "Gateway:" << address.gateway().toString();

          addresses << address;
      }
      //NO addressArg.endArray(); it's fatal in debug builds.

      setting->setAddresses(addresses);
  }

  if (map.contains(QLatin1String(NM_SETTING_IP4_CONFIG_ROUTES)))
  {
      QList<NetworkManager::IpRoute> routes;
      QList<QList<uint> > temp;
      if (map.value(QLatin1String(NM_SETTING_IP4_CONFIG_ROUTES)).canConvert< QDBusArgument>()) {
          QDBusArgument routeArg = map.value(QLatin1String(NM_SETTING_IP4_CONFIG_ROUTES)).value<QDBusArgument>();
          temp = qdbus_cast<QList<QList<uint> > >(routeArg);
      } else {
          temp = map.value(QLatin1String(NM_SETTING_IP4_CONFIG_ROUTES)).value<QList<QList<uint> > >();
      }

      foreach(const QList<uint> & uintList, temp) {
          if (uintList.count() != 4)
          {
              kWarning() << "Invalid route format detected. UInt count is " << uintList.count();
              continue;
          }

          NetworkManager::IpRoute route;
          route.setIp(QHostAddress((quint32)ntohl(uintList.at(0))));
          route.setPrefixLength((quint32)uintList.at(1));
          route.setNextHop(QHostAddress((quint32)ntohl(uintList.at(2))));
          route.setMetric((quint32)uintList.at(3));
          if (!route.isValid())
          {
              kWarning() << "Invalid route format detected.";
              continue;
          }
          routes << route;
      }
      if (!routes.isEmpty())
          setting->setRoutes(routes);
  }

  if (map.contains(QLatin1String(NM_SETTING_IP4_CONFIG_IGNORE_AUTO_DNS))) {
    setting->setIgnoredhcpdns(map.value(QLatin1String(NM_SETTING_IP4_CONFIG_IGNORE_AUTO_DNS)).value<bool>());
  } else {
    // this is needed when the connection is being updated instead of created.
    setting->setIgnoredhcpdns(false);
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
  } else {
    setting->setMayfail(true);
  }
}

Knm::Ipv4Setting::EnumMethod::type Ipv4Dbus::methodStringToEnum(QString method)
{
    if (method.toLower() == QLatin1String(NM_SETTING_IP4_CONFIG_METHOD_AUTO))
        return Knm::Ipv4Setting::EnumMethod::Automatic;
    else if (method.toLower() == QLatin1String(NM_SETTING_IP4_CONFIG_METHOD_LINK_LOCAL))
        return Knm::Ipv4Setting::EnumMethod::LinkLocal;
    else if (method.toLower() == QLatin1String(NM_SETTING_IP4_CONFIG_METHOD_MANUAL))
        return Knm::Ipv4Setting::EnumMethod::Manual;
    else if (method.toLower() == QLatin1String(NM_SETTING_IP4_CONFIG_METHOD_SHARED))
        return Knm::Ipv4Setting::EnumMethod::Shared;
    else if (method.toLower() == QLatin1String(NM_SETTING_IP4_CONFIG_METHOD_DISABLED))
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
          map.insert(QLatin1String(NM_SETTING_IP4_CONFIG_METHOD), QLatin1String(NM_SETTING_IP4_CONFIG_METHOD_AUTO));
          break;
      case Knm::Ipv4Setting::EnumMethod::LinkLocal:
          map.insert(QLatin1String(NM_SETTING_IP4_CONFIG_METHOD), QLatin1String(NM_SETTING_IP4_CONFIG_METHOD_LINK_LOCAL));
          break;
      case Knm::Ipv4Setting::EnumMethod::Manual:
          map.insert(QLatin1String(NM_SETTING_IP4_CONFIG_METHOD), QLatin1String(NM_SETTING_IP4_CONFIG_METHOD_MANUAL));
          break;
      case Knm::Ipv4Setting::EnumMethod::Shared:
          map.insert(QLatin1String(NM_SETTING_IP4_CONFIG_METHOD), QLatin1String(NM_SETTING_IP4_CONFIG_METHOD_SHARED));
          break;
       case Knm::Ipv4Setting::EnumMethod::Disabled:
          map.insert(QLatin1String(NM_SETTING_IP4_CONFIG_METHOD), QLatin1String(NM_SETTING_IP4_CONFIG_METHOD_DISABLED));
          break;
  }

  if (!setting->dns().isEmpty()) {
      QList<uint> dbusDns;
      foreach (const QHostAddress &dns, setting->dns()) {
          dbusDns << htonl(dns.toIPv4Address());
      }
      map.insert(QLatin1String(NM_SETTING_IP4_CONFIG_DNS), QVariant::fromValue(dbusDns));
  }

  if (!setting->dnssearch().isEmpty()) {
      map.insert(QLatin1String(NM_SETTING_IP4_CONFIG_DNS_SEARCH), setting->dnssearch());
  }
  if (!setting->addresses().isEmpty()) {
      QList<QList<uint> > dbusAddresses;
      foreach (const NetworkManager::IpAddress &address, setting->addresses()) {
          QList<uint> dbusAddress;
          dbusAddress << htonl(address.ip().toIPv4Address())
                      << address.prefixLength()
                      << htonl(address.gateway().toIPv4Address());
          dbusAddresses << dbusAddress;
      }
      map.insert(QLatin1String(NM_SETTING_IP4_CONFIG_ADDRESSES), QVariant::fromValue(dbusAddresses));
  }
  if (!setting->routes().isEmpty()) {
      QList<QList<uint> > dbusRoutes;
      foreach (const NetworkManager::IpRoute &route, setting->routes()) {
          QList<uint> dbusRoute;
          dbusRoute << htonl(route.ip().toIPv4Address())
                    << route.prefixLength()
                    << htonl(route.nextHop().toIPv4Address())
                    << route.metric();
          dbusRoutes << dbusRoute;
      }

  map.insert(QLatin1String(NM_SETTING_IP4_CONFIG_ROUTES), QVariant::fromValue(dbusRoutes));
  }


  //map.insert(QLatin1String(NM_SETTING_IP4_CONFIG_IGNORE_AUTO_DNS), setting->ignoredhcpdns());
  insertIfTrue(map, NM_SETTING_IP4_CONFIG_IGNORE_AUTO_DNS, setting->ignoredhcpdns());
  insertIfTrue(map, NM_SETTING_IP4_CONFIG_IGNORE_AUTO_ROUTES, setting->ignoreautoroute());
  insertIfTrue(map, NM_SETTING_IP4_CONFIG_NEVER_DEFAULT, setting->neverdefault());
  insertIfNonEmpty(map, NM_SETTING_IP4_CONFIG_DHCP_CLIENT_ID, setting->dhcpclientid());
  insertIfNonEmpty(map, NM_SETTING_IP4_CONFIG_DHCP_HOSTNAME, setting->dhcphostname());
  insertIfFalse(map, NM_SETTING_IP4_CONFIG_MAY_FAIL, setting->mayfail());
  return map;
}

QVariantMap Ipv4Dbus::toSecretsMap()
{
  QVariantMap map;
  return map;
}

