/* Copyright (C) 2002-2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fork.h>

#include <sys/queue.h>

struct fork_handler
{
  TAILQ_ENTRY (fork_handler) qe;
  void (*prepare_handler) (void);
  void (*parent_handler) (void);
  void (*child_handler) (void);
  void *dso_handle;
};

typedef TAILQ_HEAD (fork_handler_list, fork_handler) fork_handler_list;
static fork_handler_list fork_handlers = TAILQ_HEAD_INITIALIZER (fork_handlers);
static int atfork_lock = LLL_LOCK_INITIALIZER;

int
__register_atfork (void (*prepare) (void), void (*parent) (void),
		   void (*child) (void), void *dso_handle)
{
  struct fork_handler *entry = malloc (sizeof (struct fork_handler));
  if (entry == NULL)
    return ENOMEM;

  entry->prepare_handler = prepare;

  entry->parent_handler = parent;
  entry->child_handler = child;
  entry->dso_handle = dso_handle;

  lll_lock (atfork_lock, LLL_PRIVATE);
  TAILQ_INSERT_TAIL (&fork_handlers, entry, qe);
  lll_unlock (atfork_lock, LLL_PRIVATE);

  return 0;
}
libc_hidden_def (__register_atfork)

void
__unregister_atfork (void *dso_handle)
{
  fork_handler_list temp_list = TAILQ_HEAD_INITIALIZER (temp_list);
  struct fork_handler *it, *it1;

  lll_lock (atfork_lock, LLL_PRIVATE);
  TAILQ_FOREACH_SAFE (it, &fork_handlers, qe, it1)
    {
      if (it->dso_handle == dso_handle)
	{
	  TAILQ_REMOVE (&fork_handlers, it, qe);
	  TAILQ_INSERT_TAIL (&temp_list, it, qe);
	}
    }
  lll_unlock (atfork_lock, LLL_PRIVATE);

  while ((it = TAILQ_FIRST (&temp_list)) != NULL)
    {
      TAILQ_REMOVE (&temp_list, it, qe);
      free (it);
    }
}

static inline void
lock_atfork (_Bool do_locking)
{
  if (do_locking)
    lll_lock (atfork_lock, LLL_PRIVATE);
}

static inline void
unlock_atfork (_Bool do_locking)
{
  if (do_locking)
    lll_unlock (atfork_lock, LLL_PRIVATE);
}

void
__run_fork_handlers (enum __run_fork_handler_type who, _Bool do_locking)
{
  struct fork_handler *it;

  if (who == atfork_run_prepare)
    {
      lock_atfork (do_locking);
      TAILQ_FOREACH_REVERSE (it, &fork_handlers, fork_handler_list, qe)
	{
	  if (it->prepare_handler != NULL)
	    {
	      /* Unlock the list while we call a foreign function.  */
	      unlock_atfork (do_locking);
	      it->prepare_handler ();
	      lock_atfork (do_locking);
	    }
	}
    }
  else
    {
      TAILQ_FOREACH (it, &fork_handlers, qe)
	{
	  /* Unlock the list while we call a foreign function.  */
	  if (who == atfork_run_child && it->child_handler)
	    {
	      unlock_atfork (do_locking);
	      it->child_handler ();
	      lock_atfork (do_locking);
	    }
	  else if (who == atfork_run_parent && it->parent_handler)
	    {
	      unlock_atfork (do_locking);
	      it->parent_handler ();
	      lock_atfork (do_locking);
	    }
	}
      unlock_atfork (do_locking);
    }
}

libc_freeres_fn (free_mem)
{
  struct fork_handler *it, *it1;

  lll_lock (atfork_lock, LLL_PRIVATE);
  TAILQ_FOREACH_SAFE (it, &fork_handlers, qe, it1)
    {
      TAILQ_REMOVE (&fork_handlers, it, qe);
      free (it);
    }
  lll_unlock (atfork_lock, LLL_PRIVATE);
}
