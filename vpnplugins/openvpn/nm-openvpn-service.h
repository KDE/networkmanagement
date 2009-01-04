/* -*- Mode: C; tab-width: 5; indent-tabs-mode: t; c-basic-offset: 5 -*- */
/* nm-openvpn-service - openvpn integration with NetworkManager
 *
 * Tim Niemueller <tim@niemueller.de>
 * Based on work by Dan Williams <dcbw@redhat.com>
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
 *
 */

#ifndef KNM_OPENVPN_SERVICE_H
#define KNM_OPENVPN_SERVICE_H

#define NM_OPENVPN_KEY_CA "ca"
#define NM_OPENVPN_KEY_CERT "cert"
#define NM_OPENVPN_KEY_CIPHER "cipher"
#define NM_OPENVPN_KEY_COMP_LZO "comp-lzo"
#define NM_OPENVPN_KEY_CONNECTION_TYPE "connection-type"
#define NM_OPENVPN_KEY_TAP_DEV "tap-dev"
#define NM_OPENVPN_KEY_KEY "key"
#define NM_OPENVPN_KEY_LOCAL_IP "local-ip"
#define NM_OPENVPN_KEY_PROTO_TCP "proto-tcp"
#define NM_OPENVPN_KEY_PORT "port"
#define NM_OPENVPN_KEY_REMOTE "remote"
#define NM_OPENVPN_KEY_REMOTE_IP "remote-ip"
#define NM_OPENVPN_KEY_STATIC_KEY "static-key"
#define NM_OPENVPN_KEY_STATIC_KEY_DIRECTION "static-key-direction"
#define NM_OPENVPN_KEY_TA "ta"
#define NM_OPENVPN_KEY_TA_DIR "ta-dir"
#define NM_OPENVPN_KEY_USERNAME "username"

#define NM_OPENVPN_KEY_PASSWORD "password"
#define NM_OPENVPN_KEY_CERTPASS "cert-pass"
/* Internal auth-dialog -> service token indicating that no secrets are
 * required for the connection.
 */
#define NM_OPENVPN_KEY_NOSECRET "no-secret"

#define NM_OPENVPN_CONTYPE_TLS          "tls"
#define NM_OPENVPN_CONTYPE_STATIC_KEY   "static-key"
#define NM_OPENVPN_CONTYPE_PASSWORD     "password"
#define NM_OPENVPN_CONTYPE_PASSWORD_TLS "password-tls"

#endif /* KNM_OPENVPN_SERVICE_H */
