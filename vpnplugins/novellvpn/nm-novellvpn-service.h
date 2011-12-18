/* nm-novellvpn-service - novellvpn integration with NetworkManager
 *
 * Authors:
 *          Bin Li <bili@novell.com>
 *          Sureshkumar T <tsureshkumar@novell.com>
 * 
 * Based on work by Dan Williams <dcbw@redhat.com>
 *                  Tim Niemueller <tim@niemueller.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef NM_NOVELLVPN_SERVICE_H
#define NM_NOVELLVPN_SERVICE_H

/* 
   Do not change numbers, only add if needed!
   See properties/nm-novellvpn.c:connection_type_changed() for details
 */

#define NM_DBUS_SERVICE_NOVELLVPN   "org.freedesktop.NetworkManager.novellvpn"
#define NM_DBUS_INTERFACE_NOVELLVPN "org.freedesktop.NetworkManager.novellvpn"
#define NM_DBUS_PATH_NOVELLVPN          "/org/freedesktop/NetworkManager/novellvpn"

#define NM_NOVELLVPN_CONTYPE_INVALID               -1
#define NM_NOVELLVPN_CONTYPE_GROUPAUTH             0
#define NM_NOVELLVPN_CONTYPE_X509                  1

#define NM_NOVELLVPN_KEY_GWTYPE             "gateway-type"
#define NM_NOVELLVPN_KEY_AUTHTYPE           "auth-type"
#define NM_NOVELLVPN_KEY_NAME               "name"
#define NM_NOVELLVPN_KEY_GATEWAY            "remote"
#define NM_NOVELLVPN_KEY_USER_NAME          "username"
#define NM_NOVELLVPN_KEY_GROUP_NAME         "group-name"
#define NM_NOVELLVPN_KEY_DHGROUP            "dhgroup"
#define NM_NOVELLVPN_KEY_PFSGROUP           "pfsgroup"
#define NM_NOVELLVPN_KEY_CERTIFICATE        "certificate"
#define NM_NOVELLVPN_KEY_NOSPLITTUNNEL      "nosplittunnel"

#define NM_NOVELLVPN_KEY_HOW_MANY_PWD       "how_many_passwords"
#define NM_NOVELLVPN_KEY_USER_PWD           "unenc-user-password"
#define NM_NOVELLVPN_KEY_GRP_PWD            "unenc-group-password"
#define NM_NOVELLVPN_KEY_ENC_GRP_PWD        "enc-group-password"
#define NM_NOVELLVPN_KEY_CERT_PWD           "unenc-cert-password"

#define NM_NOVELLVPN_GWTYPE_NORTEL_STRING     "nortel"
#define NM_NOVELLVPN_GWTYPE_STDGW_STRING      "standard-gateway"
#define NM_NOVELLVPN_CONTYPE_GROUPAUTH_STRING "XAUTH"
#define NM_NOVELLVPN_CONTYPE_X509_STRING      "X.509"


#define NM_NOVELLVPN_GWTYPE_INVALID   -1 
#define NM_NOVELLVPN_GWTYPE_NORTEL     0
#define NM_NOVELLVPN_GWTYPE_STDGW      1

typedef enum {
  DHGROUP_INVALID = -1,
  DHGROUP_DH1 = 0,
  DHGROUP_DH2
} DHGroup;


typedef enum {
  PFSGROUP_INVALID = -1,
  PFSGROUP_OFF = 0,
  PFSGROUP_PFS1,
  PFSGROUP_PFS2
} PFSGroup;

#endif
