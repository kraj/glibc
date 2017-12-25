/* Function for pseudo-random number generation.
   Copyright (C) 2018 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/random.h>

#include <support/support_random.h>
#include <support/check.h>
#include <support/xunistd.h>

void
support_random_seed (support_random_state *state, uint32_t seed)
{
  unsigned short int seed16v[3] = { 0 };
  memcpy (seed16v, &seed, sizeof (seed));
  seed48_r (seed16v, state);
}

void
support_random_rseed (support_random_state *state)
{
  unsigned short int buf[3];
  size_t len = sizeof buf;

  ssize_t ret = getrandom (buf, len, 0);
  if (ret != len)
    {
      int fd = xopen ("/dev/urandom", O_RDONLY, 0);
      uintptr_t pbuf = (uintptr_t) buf;
      uintptr_t pend = pbuf + len;
      while (pbuf < pend)
	{
	  ret = read (fd, buf, pend - pbuf);
	  if (ret <= 0)
	    FAIL_EXIT1 ("read %zu bytes from /dev/urandom failed: %m",
			pend - pbuf);
          pbuf += ret;
	}
      close (fd);
    }

  seed48_r (buf, state);
}

uint32_t
support_random_u32 (support_random_state *state)
{
  long int rl;
  mrand48_r (state, &rl);
  return rl;
}

void
support_random_buf (support_random_state *state, void *buf, size_t nbytes)
{
  size_t nw = nbytes / sizeof (uint32_t);
  for (size_t i = 0; i < nw; i++)
    {
      uint32_t r = support_random_u32 (state);
      memcpy (buf, &r, sizeof (uint32_t));
      buf = (void*)((uintptr_t)buf + sizeof (uint32_t));
    }

  size_t nb = nbytes % sizeof (uint32_t);
  if (nb != 0)
    {
      uint32_t r = support_random_u32 (state);
      memcpy (buf, &r, nb);
    }
}
