/* Initialize cpu feature data.  PowerPC version.
   this file is part of the gnu c library.
   copyright (c) 2017 free software foundation, inc.

   the gnu c library is free software; you can redistribute it and/or
   modify it under the terms of the gnu lesser general public
   license as published by the free software foundation; either
   version 2.1 of the license, or (at your option) any later version.

   the gnu c library is distributed in the hope that it will be useful,
   but without any warranty; without even the implied warranty of
   merchantability or fitness for a particular purpose.  see the gnu
   lesser general public license for more details.

   you should have received a copy of the gnu lesser general public
   license along with the gnu c library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <stdint.h>
#include <cpu-features.h>

#if HAVE_TUNABLES
# include <elf/dl-tunables.h>
#endif

static inline void
init_cpu_features (struct cpu_features *cpu_features)
{
  /* Default is to use aligned memory access on optimized function unless
     tunables is enable, since for this case user can explicit disable
     unaligned optimizations.  */
#if HAVE_TUNABLES
  int32_t aligned_memfunc = TUNABLE_GET (glibc, tune, aligned_memopt, int32_t,
					 NULL);
  cpu_features->use_aligned_memopt = (aligned_memfunc > 0);
#else
  cpu_features->use_aligned_memopt = true;
#endif
}
