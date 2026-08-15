/* Test res_queriesmatch buffer handling (bug 34345, bug 34346).
   Copyright (C) 2026 Free Software Foundation, Inc.
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

#include <netdb.h>
#include <resolv.h>
#include <support/check.h>
#include <support/check_nss.h>
#include <support/resolv_test.h>

static void
response (const struct resolv_response_context *ctx,
          struct resolv_response_builder *b,
          const char *qname, uint16_t qclass, uint16_t qtype)
{
  switch (ctx->server_index)
    {
    case 0:
      {
        struct resolv_response_flags flags = { .rcode = 3 }; /* NXDOMAIN.  */
        resolv_response_init (b, flags);
        resolv_response_add_question (b, qname, qclass, qtype);

        /* Cause a mismatch in the transaction ID.  */
        *resolv_response_buffer (b) ^= 1;
      }
      break;

    case 1:
      {
        struct resolv_response_flags flags = { .rcode = 3 }; /* NXDOMAIN.  */
        resolv_response_init (b, flags);
        resolv_response_add_question (b, qname, qclass, qtype);

        /* Truncate the packet.  If bug 34346 is present, this
           response will be accepted because the final byte (which is
           overread) has the expected value, carried over from the
           previous response.  With bug 34345, the response is
           accepted (as NXDOMAIN) because the packet is corrupt.  */
        resolv_response_set_buffer (b,
                                  resolv_response_buffer (b),
                                  resolv_response_length (b) - 1);

      }
      break;

    case 2:
      {
        /* Finally, provide a valid response.  With either bug
           present, this is never reached because the stub resolver
           uses the response from the second server above.  */
        resolv_response_init (b, (struct resolv_response_flags) {});
        resolv_response_add_question (b, qname, qclass, qtype);
        resolv_response_section (b, ns_s_an);
        resolv_response_open_record (b, qname, qclass, qtype, 0);
        char ipv4[4] = { 192, 0, 2, 17 };
        resolv_response_add_data (b, &ipv4, sizeof (ipv4));
        resolv_response_close_record (b);
      }
      break;
    }
}

static int
do_test (void)
{
  struct resolv_test *aux = resolv_test_start
    ((struct resolv_redirect_config)
     {
       .response_callback = response,
     });

  /* Reduce test run time.  The test hits multiple timeouts as it
     switches between name server.  */
  _res.retrans = 1;
  _res.retry = 1;

  struct addrinfo hints =
    {
      .ai_family = AF_INET,
      .ai_socktype = SOCK_STREAM,
    };
  struct addrinfo *ai;
  int ret = getaddrinfo ("www.example", "80", &hints, &ai);
  check_addrinfo ("www.example", ai, ret,
                  "address: STREAM/TCP 192.0.2.17 80\n");
  if (ret == 0)
    freeaddrinfo (ai);

  resolv_test_end (aux);

  return 0;
}

#include <support/test-driver.c>
