/* Common implementation for qsort and qsort_r.
   This file is part of the GNU C Library.
   Copyright (C) 2017 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   GLicense as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifdef R_VERSION
# define R_NAME(func)        func ## _r
# define R_CMP_TYPE	     __compar_fn_t
# define R_CMP(args, p1, p2) (args)->cmp (p1, p2, args->a)
#else
# define R_NAME(func)        func
# define R_CMP_TYPE	     __compar_d_fn_t
# define R_CMP(args, p1, p2) (args)->cmp (p1, p2)
#endif

static void
R_NAME (sift) (const struct R_NAME(args_t) *args, size_t r, struct state_t s)
{
  while (s.b >= 3)
    {
      size_t r2 = r - s.b + s.c;
      if (R_CMP (args, arr (args->m, r - 1, args->s),
		 arr (args->m, r2, args->s)) >= 0)
	{
	  r2 = r - 1;
	  down (&s, 0);
        }
      if (R_CMP (args, arr (args->m, r2, args->s),
		 arr (args->m, r, args->s)) < 0)
        break;
      args->swap (arr (args->m, r, args->s),
	          arr (args->m, r2, args->s),
	          args->s);
      r = r2;
      down (&s, 0);
    }
}

static void
R_NAME (trinkle) (const struct R_NAME(args_t) *args, size_t r,
		  struct state_t s)
{
  while (s.p[0] != 0)
    {
      while ((s.p[0] & 1) == 0)
	up (&s);

      if (s.p[0] == 1)
	break;
      size_t r3 = r - s.b;
      if (R_CMP (args, arr (args->m, r3, args->s),
		 arr (args->m, r, args->s)) < 0)
        break;
      decr (s.p);
      if (s.b < 3)
	{
          args->swap (arr (args->m, r, args->s),
		      arr (args->m, r3, args->s), args->s);
	  r = r3;
          continue;
        }
      size_t r2 = r - s.b + s.c;
      if (R_CMP (args, arr (args->m, r2, args->s),
		 arr (args->m, r - 1, args->s)) < 0)
	{
	  r2 = r - 1;
	  down (&s, 0);
        }

      if (R_CMP (args, arr (args->m, r2, args->s),
		 arr (args->m, r3, args->s)) < 0)
	{
          args->swap (arr (args->m, r, args->s),
		      arr (args->m, r3, args->s),
		      args->s);
	  r = r3;
          continue;
        }

      args->swap (arr (args->m, r, args->s),
		  arr (args->m, r2, args->s),
		  args->s);
      r = r2;
      down (&s, 0);
      break;
    }

  R_NAME (sift) (args, r, s);
}

static void 
R_NAME (semitrinkle) (const struct R_NAME (args_t) *args, size_t r,
		      struct state_t s)
{
  size_t r1 = r - s.c;
  if (R_CMP (args, arr (args->m, r, args->s), arr (args->m, r1, args->s)) < 0)
    {
      args->swap (arr (args->m, r, args->s),
		  arr (args->m, r1, args->s),
		  args->s);
      R_NAME (trinkle) (args, r1, s);
    }
}

/* Construct the implicit Leonardo heap based on state S and argumetn ARGS.
   The state is expected to set Q to 1 (initial state), N to total number
   of elements and R to 0.  */
static inline void
R_NAME (buildtree) (const struct R_NAME (args_t) *args, struct state_t *s)
{
  for (; s->q != s->n; s->q++, s->r++, incr (s->p))
    {
      if ((s->p[0] & 7) == 3)
	{
	  R_NAME (sift) (args, s->r, *s);
	  up (s);
	  up (s);
        }
      else /* if ((s.p[0] & 3) == 1)  */
	{
	  if (s->q + s->c < s->n)
	    R_NAME (sift) (args, s->r, *s);
          else
            R_NAME (trinkle) (args, s->r, *s);
          while (down (s, 0) > 1);
        }
    }
}

static inline void
R_NAME (buildsorted) (const struct R_NAME (args_t) *args, struct state_t *s)
{
  for (; s->q > 1; s->q--)
    {
      decr (s->p);
      if (s->b <= 1)
	{
	  while ((s->p[0] & 1) == 0)
	   up (s);
	  --s->r;
        }
      else /* if s.b >= 3 */
	{
	  if (s->p[0])
	    R_NAME (semitrinkle) (args, s->r - (s->b - s->c), *s);
	  down (s, 1);
	  R_NAME (semitrinkle) (args, --s->r, *s);
	  down (s, 1);
        }
    }
}

#undef R_NAME
#undef R_CMP
#undef R_CMP_TYPE
#undef R_VERSION
