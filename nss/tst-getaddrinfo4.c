/* Test getaddrinfo return value, [BZ #15339].
   Copyright (C) 2013-2026 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <support/check.h>
#include <support/resolv_test.h>

static void
response (const struct resolv_response_context *ctx,
          struct resolv_response_builder *b,
          const char *qname, uint16_t qclass, uint16_t qtype)
{
  resolv_response_init (b, (struct resolv_response_flags) { });
  resolv_response_add_question (b, qname, qclass, qtype);
  resolv_response_section (b, ns_s_an);
  resolv_response_open_record (b, qname, qclass, qtype, 0);
  switch (qtype)
    {
    case T_A:
      resolv_response_add_data (b, "\xc0\x00\x02\x01", 4);
      break;
    case T_AAAA:
      resolv_response_add_data
        (b, "\x20\x01\x0d\xb8\0\0\0\0\0\0\0\0\0\0\0\x01", 16);
      break;
    }
  resolv_response_close_record (b);
}

static void
try (const char *service, int family, int flags)
{
  struct addrinfo hints, *ai;

  memset (&hints, 0, sizeof hints);
  hints.ai_family = family;
  hints.ai_flags = flags;

  int res = getaddrinfo ("example.net", service,
                         (family || flags) ? &hints : NULL, &ai);
  switch (res)
    {
    case 0:
      freeaddrinfo (ai);
      /* Fall through.  */
    case EAI_AGAIN:
    case EAI_NONAME:
      printf ("SUCCESS getaddrinfo(service=%s, family=%d, flags=%d): %s: %m\n",
              service ?: "NULL", family, flags, gai_strerror (res));
      return;
    }
  FAIL ("getaddrinfo(service=%s, family=%d, flags=%d): %s",
        service ?: "NULL", family, flags, gai_strerror (res));
}

static int
do_test (void)
{
  struct resolv_test *obj = resolv_test_start
    ((struct resolv_redirect_config) { .response_callback = response });

  try (NULL, 0, 0);
  try (NULL, AF_UNSPEC, AI_ADDRCONFIG);
  try (NULL, AF_INET, 0);
  try (NULL, AF_INET6, 0);
  try ("http", 0, 0);
  try ("http", AF_UNSPEC, AI_ADDRCONFIG);
  try ("http", AF_INET, 0);
  try ("http", AF_INET6, 0);

  resolv_test_end (obj);
  return 0;
}

#include <support/test-driver.c>
