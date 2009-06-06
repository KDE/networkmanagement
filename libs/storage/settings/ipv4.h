// This file is generated by kconfig_compiler from ipv4.kcfg.
// All changes you do to this file will be lost.
#ifndef KNM_INTERNALS_IPV4SETTING_H
#define KNM_INTERNALS_IPV4SETTING_H

#include <QHostAddress>
#include <kglobal.h>
#include <kdebug.h>
#include <kcoreconfigskeleton.h>
#include <solid/control/networkipv4config.h>
#include "setting.h"
#include "knm_export.h"

Q_DECLARE_METATYPE(Solid::Control::IPv4Address)
Q_DECLARE_METATYPE(Solid::Control::IPv4Route)

namespace Knm {

class KNM_EXPORT Ipv4Setting : public Setting
{
  public:
    class EnumMethod
    {
      public:
      enum type { Automatic, LinkLocal, Manual, Shared, COUNT };
    };

    Ipv4Setting( );
    ~Ipv4Setting();

    QString name() const;

    bool hasSecrets() const;

    /**
      Set Method
    */
    void setMethod( int v )
    {
        mMethod = v;
    }

    /**
      Get Method
    */
    int method() const
    {
      return mMethod;
    }

    /**
      Set DNS Servers
    */
    void setDns( const QList<QHostAddress> & v )
    {
        mDns = v;
    }

    /**
      Get DNS Servers
    */
    QList<QHostAddress> dns() const
    {
      return mDns;
    }

    /**
      Set Search Domains
    */
    void setDnssearch( const QStringList & v )
    {
        mDnssearch = v;
    }

    /**
      Get Search Domains
    */
    QStringList dnssearch() const
    {
      return mDnssearch;
    }

    /**
      Set IP Addresses
    */
    void setAddresses( const QList<Solid::Control::IPv4Address> & v )
    {
        mAddresses = v;
    }

    /**
      Get IP Addresses
    */
    QList<Solid::Control::IPv4Address> addresses() const
    {
      return mAddresses;
    }

    /**
      Set Ignore DHCP DNS
    */
    void setIgnoredhcpdns( bool v )
    {
        mIgnoredhcpdns = v;
    }

    /**
      Get Ignore DHCP DNS
    */
    bool ignoredhcpdns() const
    {
      return mIgnoredhcpdns;
    }

    QList<Solid::Control::IPv4Route> routes() const
    {
        return mRoutes;
    }

    void setRoutes(QList<Solid::Control::IPv4Route> routes)
    {
        mRoutes = routes;
    }

  protected:

    // ipv4
    int mMethod;
    QList<QHostAddress> mDns;
    QStringList mDnssearch;
    QList<Solid::Control::IPv4Address> mAddresses;
    QList<Solid::Control::IPv4Route> mRoutes;
    bool mIgnoredhcpdns;
  private:
};

}

#endif

