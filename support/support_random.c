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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/random.h>
#include <support/support_random.h>

int
random_seed (void *buf, size_t len)
{
  ssize_t ret = getrandom (buf, len, 0);
  if (ret == len)
    return 0;

  int fd = open ("/dev/urandom", O_RDONLY);
  if (fd < 0)
    return -1;
  void *end = buf + len;
  while (buf < end)
    {
      ssize_t ret = read (fd, buf, end - buf);
      if (ret <= 0)
	break;
      buf += ret;
    }
  close (fd);
  return buf == end ? 0 : -1;
}

/* The classic Mersenne Twister. Reference:
   M. Matsumoto and T. Nishimura, Mersenne Twister: A 623-Dimensionally
   Equidistributed Uniform Pseudo-Random Number Generator, ACM Transactions
   on Modeling and Computer Simulation, Vol. 8, No. 1, January 1998, pp 3-30.

   This version is based on libstdc++ std::mt19937{_64}.  */

static const size_t mt32_word_size         = 32;
static const size_t mt32_mask_bits         = 31;
static const size_t mt32_state_size        = MT32_STATE_SIZE;
static const size_t mt32_shift_size        = 397;
static const uint32_t mt32_xor_mask        = 0x9908b0dfUL;
static const size_t mt32_tempering_u       = 11;
static const uint32_t mt32_tempering_d     = 0xffffffffUL;
static const size_t mt32_tempering_s       = 7;
static const uint32_t mt32_tempering_b     = 0x9d2c5680UL;
static const size_t mt32_tempering_t       = 15;
static const uint32_t mt32_tempering_c     = 0xefc60000UL;
static const size_t mt32_tempering_l       = 18;
static const uint32_t mt32_init_multiplier = 1812433253UL;
static const uint32_t mt32_default_seed    = 5489u;

static void
mt32_gen_rand (struct mt19937_32 *state)
{
  const uint32_t upper_mask = (uint32_t)-1 << mt32_mask_bits;
  const uint32_t lower_mask = ~upper_mask;

  for (size_t k = 0; k < (mt32_state_size - mt32_shift_size); k++)
    {
      uint32_t y = ((state->mt[k] & upper_mask)
		   | (state->mt[k + 1] & lower_mask));
      state->mt[k] = (state->mt[k + mt32_shift_size] ^ (y >> 1)
		     ^ ((y & 0x01) ? mt32_xor_mask : 0));
    }

  for (size_t k = (mt32_state_size - mt32_shift_size);
       k < (mt32_state_size - 1); k++)
    {
      uint32_t y = ((state->mt[k] & upper_mask)
		   | (state->mt[k + 1] & lower_mask));
      state->mt[k] = (state->mt[k + (mt32_shift_size - mt32_state_size)]
		      ^ (y >> 1) ^ ((y & 0x01) ? mt32_xor_mask : 0));
    }

  uint32_t y = ((state->mt[mt32_state_size - 1] & upper_mask)
		| (state->mt[0] & lower_mask));
  state->mt[mt32_state_size - 1] = (state->mt[mt32_shift_size -1] ^ (y >> 1)
				    ^ (( y & 0x01) ? mt32_xor_mask : 0));
  state->p = 0;
}

void
mt32_seed (struct mt19937_32 *state, uint32_t seed)
{
  /* Generators based on linear-feedback shift-register techniques can not
     handle all zero initial state (they will output zero continually).  In
     such cases we use the default initial state).  */
  if (seed == 0x0)
    seed = mt32_default_seed;

  state->mt[0] = mt32_default_seed;
  for (size_t i = 1; i < mt32_state_size; i++)
    {
      uint32_t x = state->mt[i - 1];
      x ^= x >> (mt32_word_size - 2);
      x *= mt32_init_multiplier;
      x += i;
      state->mt[i] = x;
    }
  state->p = mt32_state_size;
}

uint32_t
mt32_rand (struct mt19937_32 *state)
{
  /* Reload the vector - cost is O(n) amortized over n calls.  */
  if (state->p >= mt32_state_size)
   mt32_gen_rand (state);

  /* Calculate o(x(i)).  */
  uint32_t z = state->mt[state->p++];
  z ^= (z >> mt32_tempering_u) & mt32_tempering_d;
  z ^= (z << mt32_tempering_s) & mt32_tempering_b;
  z ^= (z << mt32_tempering_t) & mt32_tempering_c;
  z ^= (z >> mt32_tempering_l);
  return z;
}


static const size_t mt64_word_size         = 64;
static const size_t mt64_mask_bits         = 31;
static const size_t mt64_state_size        = MT64_STATE_SIZE;
static const size_t mt64_shift_size        = 156;
static const uint64_t mt64_xor_mask        = 0xb5026f5aa96619e9ULL;
static const size_t mt64_tempering_u       = 29;
static const uint64_t mt64_tempering_d     = 0x5555555555555555ULL;
static const size_t mt64_tempering_s       = 17;
static const uint64_t mt64_tempering_b     = 0x71d67fffeda60000ULL;
static const size_t mt64_tempering_t       = 37;
static const uint64_t mt64_tempering_c     = 0xfff7eee000000000ULL;
static const size_t mt64_tempering_l       = 43;
static const uint64_t mt64_init_multiplier = 6364136223846793005ULL;
static const uint64_t mt64_default_seed    = 5489u;

static void
mt64_gen_rand (struct mt19937_64 *state)
{
  const uint64_t upper_mask = (uint64_t)-1 << mt64_mask_bits;
  const uint64_t lower_mask = ~upper_mask;

  for (size_t k = 0; k < (mt64_state_size - mt64_shift_size); k++)
    {
      uint64_t y = ((state->mt[k] & upper_mask)
		   | (state->mt[k + 1] & lower_mask));
      state->mt[k] = (state->mt[k + mt64_shift_size] ^ (y >> 1)
		     ^ ((y & 0x01) ? mt64_xor_mask : 0));
    }

  for (size_t k = (mt64_state_size - mt64_shift_size);
       k < (mt64_state_size - 1); k++)
    {
      uint64_t y = ((state->mt[k] & upper_mask)
		   | (state->mt[k + 1] & lower_mask));
      state->mt[k] = (state->mt[k + (mt64_shift_size - mt64_state_size)]
		      ^ (y >> 1) ^ ((y & 0x01) ? mt64_xor_mask : 0));
    }

  uint64_t y = ((state->mt[mt64_state_size - 1] & upper_mask)
		| (state->mt[0] & lower_mask));
  state->mt[mt64_state_size - 1] = (state->mt[mt64_shift_size -1] ^ (y >> 1)
				    ^ (( y & 0x01) ? mt64_xor_mask : 0));
  state->p = 0;
}

void
mt64_seed (struct mt19937_64 *state, uint64_t seed)
{
  /* Generators based on linear-feedback shift-register techniques can not
     handle all zero initial state (they will output zero continually).  In
     such cases we use the default initial state).  */
  if (seed == 0x0)
    seed = mt64_default_seed;

  state->mt[0] = mt64_default_seed;
  for (size_t i = 1; i < mt64_state_size; i++)
    {
      uint64_t x = state->mt[i - 1];
      x ^= x >> (mt64_word_size - 2);
      x *= mt64_init_multiplier;
      x += i;
      state->mt[i] = x;
    }
  state->p = mt64_state_size;
}

uint64_t
mt64_rand (struct mt19937_64 *state)
{
  /* Reload the vector - cost is O(n) amortized over n calls.  */
  if (state->p >= mt64_state_size)
   mt64_gen_rand (state);

  /* Calculate o(x(i)).  */
  uint64_t z = state->mt[state->p++];
  z ^= (z >> mt64_tempering_u) & mt64_tempering_d;
  z ^= (z << mt64_tempering_s) & mt64_tempering_b;
  z ^= (z << mt64_tempering_t) & mt64_tempering_c;
  z ^= (z >> mt64_tempering_l);
  return z;
}
