/*
 * SHA1 hash implementation and interface functions
 * Copyright (c) 2003-2005, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#ifndef PBKDF2_H
#define PBKDF2_H

#include <sys/types.h>

#define SHA1_MAC_LEN 20

void sha1_mac(const u_int8_t *key, size_t key_len, const u_int8_t *data, size_t data_len, // krazy:exclude=typedefs
	      u_int8_t *mac);  // krazy:exclude=typedefs
void hmac_sha1_vector(const u_int8_t *key, size_t key_len, size_t num_elem,  // krazy:exclude=typedefs 
		      const u_int8_t *addr[], const size_t *len, u_int8_t *mac);  // krazy:exclude=typedefs 
void hmac_sha1(const u_int8_t *key, size_t key_len, const u_int8_t *data, size_t data_len,  // krazy:exclude=typedefs
	       u_int8_t *mac);  // krazy:exclude=typedefs 
void sha1_prf(const u_int8_t *key, size_t key_len, const char *label,  // krazy:exclude=typedefs
	      const u_int8_t *data, size_t data_len, u_int8_t *buf, size_t buf_len);  // krazy:exclude=typedefs
void pbkdf2_sha1(const char *passphrase, const char *ssid, size_t ssid_len,
		 int iterations, u_int8_t *buf, size_t buflen);  // krazy:exclude=typedefs

#endif /* PBKDF2_H */
