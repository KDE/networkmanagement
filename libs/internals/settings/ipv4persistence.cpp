// This file is generated by kconfig_compiler from ipv4.kcfg.
// All changes you do to this file will be lost.

#include "ipv4persistence.h"

#include "ipv4.h"

using namespace Knm;

Ipv4Persistence::Ipv4Persistence(Ipv4Setting * setting, KSharedConfig::Ptr config, ConnectionPersistence::SecretStorageMode mode) : SettingPersistence(setting, config, mode)
{
}

Ipv4Persistence::~Ipv4Persistence()
{
}

void Ipv4Persistence::load()
{
  Ipv4Setting * setting = static_cast<Ipv4Setting *>(m_setting);
  {
    QString contents = m_config->readEntry("method", "Automatic");
    if (contents == "Automatic")
      setting->setMethod(Ipv4Setting::EnumMethod::Automatic);
    else     if (contents == "LinkLocal")
      setting->setMethod(Ipv4Setting::EnumMethod::LinkLocal);
    else     if (contents == "Manual")
      setting->setMethod(Ipv4Setting::EnumMethod::Manual);
    else     if (contents == "Shared")
      setting->setMethod(Ipv4Setting::EnumMethod::Shared);

  }

  // dns
  QList<QHostAddress> dnsServers;
  QStringList rawDnsServers = m_config->readEntry("dns", QStringList());
  foreach (QString server, rawDnsServers) {
      dnsServers.append(QHostAddress(server));
  }
  setting->setDns(dnsServers);

  setting->setDnssearch(m_config->readEntry("dnssearch", QStringList()));

  // addresses
  QList<Solid::Control::IPv4Address> addresses;
  QStringList rawAddresses = m_config->readEntry("addresses", QStringList());
  foreach (QString rawAddress, rawAddresses) {
      QStringList parts = rawAddress.split(';');
      if (parts.count() != 3) { // sanity check
          continue;
      }
      QHostAddress ip(parts[0]);
      QHostAddress gateway(parts[2]);
      Solid::Control::IPv4Address addr(ip.toIPv4Address(), parts[1].toUInt(), gateway.toIPv4Address());
      addresses.append(addr);
  }
  setting->setAddresses(addresses);

  // routes
  QList<Solid::Control::IPv4Route> routes;
  QStringList rawRoutes = m_config->readEntry("routes", QStringList());
  foreach (QString rawRoute, rawRoutes) {
      QStringList parts = rawRoute.split(';');
      if (parts.count() != 4) { // sanity check
          continue;
      }
      QHostAddress address(parts[0]);
      quint32 prefix = parts[1].toUInt();
      QHostAddress nextHop(parts[2]);
      quint32 metric = parts[3].toUInt();
      Solid::Control::IPv4Route route(address.toIPv4Address(), prefix, nextHop.toIPv4Address(), metric);
      routes.append(route);
  }
  setting->setRoutes(routes);
  setting->setIgnoredhcpdns(m_config->readEntry("ignoredhcpdns", false));
  setting->setInitialized();
}

void Ipv4Persistence::save()
{
  Ipv4Setting * setting = static_cast<Ipv4Setting *>(m_setting);
  switch (setting->method()) {
    case Ipv4Setting::EnumMethod::Automatic:
      m_config->writeEntry("method", "Automatic");
      break;
    case Ipv4Setting::EnumMethod::LinkLocal:
      m_config->writeEntry("method", "LinkLocal");
      break;
    case Ipv4Setting::EnumMethod::Manual:
      m_config->writeEntry("method", "Manual");
      break;
    case Ipv4Setting::EnumMethod::Shared:
      m_config->writeEntry("method", "Shared");
      break;
  }

  QStringList rawDns;
  foreach (QHostAddress dns, setting->dns()) {
    rawDns.append(dns.toString());
  }
  if (!rawDns.isEmpty())
      m_config->writeEntry("dns", rawDns);

  if (!setting->dnssearch().isEmpty())
      m_config->writeEntry("dnssearch", setting->dnssearch());

  QStringList rawAddresses;
  foreach (Solid::Control::IPv4Address addr, setting->addresses()) {
      QStringList rawAddress;
      rawAddress << QHostAddress(addr.address()).toString()
          << QString::number(addr.netMask())
          << QHostAddress(addr.gateway()).toString();
      rawAddresses << rawAddress.join(";");
  }
  m_config->writeEntry("addresses", rawAddresses);

  QStringList rawRoutes;
  foreach (Solid::Control::IPv4Route route, setting->routes()) {
      QStringList rawRoute;
      rawRoute << QHostAddress(route.route()).toString()
          << QString::number(route.prefix())
          << QHostAddress(route.nextHop()).toString()
          << QString::number(route.metric());
      rawRoutes << rawRoute;
  }
  m_config->writeEntry("routes", rawRoutes);

  m_config->writeEntry("ignoredhcpdns", setting->ignoredhcpdns());
}

QMap<QString,QString> Ipv4Persistence::secrets() const
{
  QMap<QString,QString> map;
  return map;
}

void Ipv4Persistence::restoreSecrets(QMap<QString,QString> secrets) const
{
  if (m_storageMode == ConnectionPersistence::Secure) {
  Q_UNUSED(secrets);
  }
}
