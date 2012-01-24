// This file is generated by kconfig_compiler from ipv6.kcfg.
// All changes you do to this file will be lost.

#include "ipv6dbus.h"

#include <arpa/inet.h>
#include <QtNetworkManager/generic-types.h>
#include "ipv6.h"

Ipv6Dbus::Ipv6Dbus(Knm::Ipv6Setting * setting) : SettingDbus(setting)
{
}

Ipv6Dbus::~Ipv6Dbus()
{
}

void Ipv6Dbus::fromMap(const QVariantMap & map)
{
  /*kDebug() << "IPv6 map: ";
  foreach(const QString &key, map.keys())
      kDebug() << key << " : " << map.value(key);*/

  Knm::Ipv6Setting * setting = static_cast<Knm::Ipv6Setting*>(m_setting);

  if (map.contains(QLatin1String(NM_SETTING_IP6_CONFIG_METHOD))) {
      setting->setMethod(methodStringToEnum(map.value(QLatin1String(NM_SETTING_IP6_CONFIG_METHOD)).value<QString>())); }

  if (map.contains(QLatin1String(NM_SETTING_IP6_CONFIG_DNS))) {
      QDBusArgument dnsArg = map.value(QLatin1String(NM_SETTING_IP6_CONFIG_DNS)).value< QDBusArgument>();
      QList<QHostAddress> dbusDns;

      dnsArg.beginArray();
      while(!dnsArg.atEnd())
      {
          QByteArray utmp;
          dnsArg >> utmp;
          Q_IPV6ADDR tmp;
          for (int i = 0; i < 16; i++)
          {
              tmp[i] = utmp[i];
          }
          QHostAddress tmpHost(tmp);
          dbusDns << tmpHost;
      }
      //NO dnsArg.endArray(); it's fatal in debug builds.

      setting->setDns(dbusDns);
  }

  if (map.contains(QLatin1String(NM_SETTING_IP6_CONFIG_DNS_SEARCH)) &&
      !map.value(QLatin1String(NM_SETTING_IP6_CONFIG_DNS_SEARCH)).value<QStringList>().isEmpty()
     ) {
    setting->setDnssearch(map.value(QLatin1String(NM_SETTING_IP6_CONFIG_DNS_SEARCH)).value<QStringList>());
  }

  if (map.contains(QLatin1String(NM_SETTING_IP6_CONFIG_ADDRESSES))) {
      QDBusArgument addressArg = map.value(QLatin1String(NM_SETTING_IP6_CONFIG_ADDRESSES)).value< QDBusArgument>();
      QList<NetworkManager::IPv6Address> addresses;

      addressArg.beginArray();
      while(!addressArg.atEnd())
      {
          IpV6AddressMap addressMap;
          addressArg >> addressMap;

          if (addressMap.address.isEmpty() || !addressMap.netMask || addressMap.gateway.isEmpty())
          {
            kWarning() << "Invalid address format detected.";
            continue;
          }
          Q_IPV6ADDR ip, gateway;
          for (int i = 0; i < 16; i++)
          {
              ip[i] = addressMap.address[i];
          }
          for (int i = 0; i < 16; i++)
          {
              gateway[i] = addressMap.gateway[i];
          }

          NetworkManager::IPv6Address addr(ip, addressMap.netMask, gateway);
          if (!addr.isValid())
          {
            kWarning() << "Invalid address format detected.";
            continue;
          }

          addresses << addr;
      }
      //NO addressArg.endArray(); it's fatal in debug builds.

      setting->setAddresses(addresses);
  }

  if (map.contains(QLatin1String(NM_SETTING_IP6_CONFIG_ROUTES)))
  {
      QDBusArgument routeArg = map.value(QLatin1String(NM_SETTING_IP6_CONFIG_ROUTES)).value< QDBusArgument>();
      QList<NetworkManager::IPv6Route> routes;

      routeArg.beginArray();
      while(!routeArg.atEnd())
      {
          IpV6RouteMap routeMap;
          routeArg >> routeMap;

          if (routeMap.route.isEmpty() || !routeMap.prefix || routeMap.nextHop.isEmpty() || !routeMap.metric)
          {
              kWarning() << "Invalid route format detected.";
              continue;
          }
          Q_IPV6ADDR addr, nexthop;
          for (int i = 0; i < 16; i++)
          {
              addr[i] = routeMap.route[i];
          }
          for (int i = 0; i < 16; i++)
          {
              nexthop[i] = routeMap.nextHop[i];
          }

          NetworkManager::IPv6Route route(addr, routeMap.prefix, nexthop, routeMap.metric);
          if (!route.isValid())
          {
              kWarning() << "Invalid route format detected.";
              continue;
          }

          routes << route;
      }
      setting->setRoutes(routes);
  }

  if (map.contains(QLatin1String(NM_SETTING_IP6_CONFIG_IGNORE_AUTO_DNS))) {
    setting->setIgnoredhcpdns(map.value(QLatin1String(NM_SETTING_IP6_CONFIG_IGNORE_AUTO_DNS)).value<bool>());
  }
  if (map.contains(QLatin1String(NM_SETTING_IP6_CONFIG_IGNORE_AUTO_ROUTES))) {
    setting->setIgnoreautoroute(map.value(QLatin1String(NM_SETTING_IP6_CONFIG_IGNORE_AUTO_ROUTES)).value<bool>());
  }
  if (map.contains(QLatin1String(NM_SETTING_IP6_CONFIG_NEVER_DEFAULT))) {
    setting->setNeverdefault(map.value(QLatin1String(NM_SETTING_IP6_CONFIG_NEVER_DEFAULT)).value<bool>());
  }
  if (map.contains(QLatin1String(NM_SETTING_IP6_CONFIG_MAY_FAIL))) {
    setting->setMayfail(map.value(QLatin1String(NM_SETTING_IP6_CONFIG_MAY_FAIL)).value<bool>());
  }
}

Knm::Ipv6Setting::EnumMethod::type Ipv6Dbus::methodStringToEnum(QString method)
{
    if (method.toLower() == QLatin1String(NM_SETTING_IP6_CONFIG_METHOD_AUTO))
        return Knm::Ipv6Setting::EnumMethod::Automatic;
    else if (method.toLower() == QLatin1String(NM_SETTING_IP6_CONFIG_METHOD_DHCP))
        return Knm::Ipv6Setting::EnumMethod::Dhcp;
    else if (method.toLower() == QLatin1String(NM_SETTING_IP6_CONFIG_METHOD_LINK_LOCAL))
        return Knm::Ipv6Setting::EnumMethod::LinkLocal;
    else if (method.toLower() == QLatin1String(NM_SETTING_IP6_CONFIG_METHOD_MANUAL))
        return Knm::Ipv6Setting::EnumMethod::Manual;
    else if (method.toLower() == QLatin1String(NM_SETTING_IP6_CONFIG_METHOD_SHARED))
        return Knm::Ipv6Setting::EnumMethod::Shared;
    else if (method.toLower() == QLatin1String(NM_SETTING_IP6_CONFIG_METHOD_IGNORE))
        return Knm::Ipv6Setting::EnumMethod::Ignore;
    else
    {
        kDebug() << "Unknown method given:" << method;
        return Knm::Ipv6Setting::EnumMethod::Automatic;
    }
}

QVariantMap Ipv6Dbus::toMap()
{
  QVariantMap map;
  Knm::Ipv6Setting * setting = static_cast<Knm::Ipv6Setting *>(m_setting);
  switch (setting->method()) {
      case Knm::Ipv6Setting::EnumMethod::Automatic:
          map.insert(QLatin1String(NM_SETTING_IP6_CONFIG_METHOD), QLatin1String(NM_SETTING_IP6_CONFIG_METHOD_AUTO));
          break;
      case Knm::Ipv6Setting::EnumMethod::Dhcp:
          map.insert(QLatin1String(NM_SETTING_IP6_CONFIG_METHOD), QLatin1String(NM_SETTING_IP6_CONFIG_METHOD_DHCP));
      case Knm::Ipv6Setting::EnumMethod::LinkLocal:
          map.insert(QLatin1String(NM_SETTING_IP6_CONFIG_METHOD), QLatin1String(NM_SETTING_IP6_CONFIG_METHOD_LINK_LOCAL));
          break;
      case Knm::Ipv6Setting::EnumMethod::Manual:
          map.insert(QLatin1String(NM_SETTING_IP6_CONFIG_METHOD), QLatin1String(NM_SETTING_IP6_CONFIG_METHOD_MANUAL));
          break;
      case Knm::Ipv6Setting::EnumMethod::Shared:
          map.insert(QLatin1String(NM_SETTING_IP6_CONFIG_METHOD), QLatin1String(NM_SETTING_IP6_CONFIG_METHOD_SHARED));
          break;
      case Knm::Ipv6Setting::EnumMethod::Ignore:
          map.insert(QLatin1String(NM_SETTING_IP6_CONFIG_METHOD), QLatin1String(NM_SETTING_IP6_CONFIG_METHOD_IGNORE));
          break;
  }

  if (!setting->dns().isEmpty()) {
      QList<QByteArray> dbusDns;
      foreach (const QHostAddress &dns, setting->dns()) {
          Q_IPV6ADDR dnsAddress = dns.toIPv6Address();
          QByteArray assembledDnsAddress;
          for (int i = 0; i <16; i++)
          {
              assembledDnsAddress[i] = dnsAddress[i];
          }

          dbusDns << assembledDnsAddress;
      }
      map.insert(QLatin1String(NM_SETTING_IP6_CONFIG_DNS), QVariant::fromValue(dbusDns));
  }

  if (!setting->dnssearch().isEmpty()) {
      map.insert(QLatin1String(NM_SETTING_IP6_CONFIG_DNS_SEARCH), setting->dnssearch());
  }
  if (!setting->addresses().isEmpty()) {
      QList<IpV6AddressMap> dbusAddresses;
      foreach (const NetworkManager::IPv6Address &addr, setting->addresses()) {
          IpV6AddressMap dbusAddress;
          Q_IPV6ADDR address = addr.address();
          QList<quint8> assembledAddress;
          for (int i = 0; i < 16; i++)
          {
              assembledAddress << address[i];
          }

          Q_IPV6ADDR gateway = addr.gateway();
          QList<quint8> assembledGateway;
          for (int i = 0; i < 16; i++)
          {
              assembledGateway << gateway[i];
          }

          dbusAddress.address = assembledAddress;
          dbusAddress.netMask = addr.netMask();
          dbusAddress.gateway = assembledGateway;
          dbusAddresses << dbusAddress;
      }
      map.insert(QLatin1String(NM_SETTING_IP6_CONFIG_ADDRESSES), QVariant::fromValue(dbusAddresses));
  }
  if (!setting->routes().isEmpty()) {
      QList<IpV6RouteMap> dbusRoutes;
      foreach (const NetworkManager::IPv6Route &route, setting->routes()) {
          IpV6RouteMap dbusRoute;

          Q_IPV6ADDR Route = route.route();
          QList<quint8> assembledRoute;
          for (int i = 0; i < 16; i++)
          {
              assembledRoute << Route[i];
          }

          Q_IPV6ADDR nextHop = route.nextHop();
          QList<quint8> assembledNextHop;
          for (int i = 0; i < 16; i++)
          {
              assembledNextHop << nextHop[i];
          }

          dbusRoute.route = assembledRoute;
          dbusRoute.prefix = route.prefix();
          dbusRoute.nextHop = assembledNextHop;
          dbusRoute.metric = route.metric();
          dbusRoutes << dbusRoute;
      }

      map.insert(QLatin1String(NM_SETTING_IP6_CONFIG_ROUTES), QVariant::fromValue(dbusRoutes));
  }


  insertIfTrue(map, NM_SETTING_IP6_CONFIG_IGNORE_AUTO_DNS, setting->ignoredhcpdns());
  insertIfTrue(map, NM_SETTING_IP6_CONFIG_IGNORE_AUTO_ROUTES, setting->ignoreautoroute());
  insertIfTrue(map, NM_SETTING_IP6_CONFIG_NEVER_DEFAULT, setting->neverdefault());
  insertIfTrue(map, NM_SETTING_IP6_CONFIG_MAY_FAIL, setting->mayfail());
  return map;
}

QVariantMap Ipv6Dbus::toSecretsMap()
{
  QVariantMap map;
  return map;
}

QDBusArgument &operator<<(QDBusArgument &argument, const IpV6AddressMap &map)
{
    argument.beginStructure();
    argument << map.address << map.netMask << map.gateway;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, IpV6AddressMap &map)
{
    argument.beginStructure();
    argument >> map.address >> map.netMask >> map.gateway;
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const IpV6RouteMap &map)
{
    argument.beginStructure();
    argument << map.route << map.prefix << map.nextHop << map.metric;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, IpV6RouteMap &map)
{
    argument.beginStructure();
    argument >> map.route >> map.prefix >> map.nextHop >> map.metric;
    argument.endStructure();
    return argument;
}
