/* Test for a getnameinfo memory leak with NI_IDN and a short buffer.
   Copyright (C) 2026 The GNU Toolchain Authors.
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

#include <mcheck.h>
#include <netdb.h>
#include <netinet/in.h>
#include <support/check.h>
#include <support/resolv_test.h>
#include <support/xdlfcn.h>

static void
response (const struct resolv_response_context *ctx,
          struct resolv_response_builder *b,
          const char *qname, uint16_t qclass, uint16_t qtype)
{
  TEST_COMPARE_STRING (qname, "1.2.0.192.in-addr.arpa");
  TEST_COMPARE (qclass, C_IN);
  TEST_COMPARE (qtype, T_PTR);

  resolv_response_init (b, (struct resolv_response_flags) { });
  resolv_response_add_question (b, qname, qclass, qtype);
  resolv_response_section (b, ns_s_an);
  resolv_response_open_record (b, qname, qclass, qtype, 0);
  resolv_response_add_name (b, "non-idn.example");
  resolv_response_close_record (b);
}

static int
do_test (void)
{
  mtrace ();

  /* Load an empty DSO with libidn2's soname to force the
     strdup fallback in __idna_from_dns_encoding.  */
  void *handle = xdlopen ("tst-no-libidn2.so", RTLD_LAZY);
  void *handle2 = xdlopen ("libidn2.so.0", RTLD_LAZY | RTLD_NOLOAD);
  TEST_VERIFY (handle2 == handle);
  xdlclose (handle2);

  struct resolv_test *aux = resolv_test_start
    ((struct resolv_redirect_config)
     {
       .response_callback = response,
       .nscount = 1,
     });

  struct sockaddr_in sa =
    {
      .sin_family = AF_INET,
      .sin_addr = { htonl (0xc0000201) },
    };
  char host[1];

  int ret = getnameinfo ((const struct sockaddr *) &sa, sizeof (sa),
                         host, sizeof (host), NULL, 0, NI_IDN | NI_NAMEREQD);
  TEST_COMPARE (ret, EAI_OVERFLOW);

  resolv_test_end (aux);
  xdlclose (handle);
  return 0;
}

#include <support/test-driver.c>
