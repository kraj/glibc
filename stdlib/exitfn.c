/* Internal function to manipulate exit handler lists.
   Copyright (C) 2019 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <assert.h>
#include <exit.h>
#include <array_length.h>

/* Declared at stdlib/exit.h.  */
__libc_lock_define_initialized (, __exit_funcs_lock)
/* Updated each time a new entry is added on any list.  */
uint64_t __new_exitfn_called;
/* Set to indicate exit function processing is complete.  */
bool __exit_funcs_done = false;

/* Allocate a new entry of flavor FLAVOR on list LISTP.  The list must be kept
   in order with following constraints:

   1. el_at nodes should be prior el_cxa.
   2. el_at should contain only ef_at, ef_on, or ef_free elements.
   3. el_cxa should contain only ef_cxa or ef_free elements.
   4. new insertions on each node type should be be kept in lifo order.
   5. the original first element should be last one (since it is static allocated
      and 'exit' will deallocated the nodes in order.  */
static struct exit_function *
__new_exitfn_entry (struct exit_function_list **listp, enum ef_flavor flavor)
{
  enum el_type type = flavor == ef_cxa ? el_cxa : el_at;

  struct exit_function_list *pp = NULL;
  struct exit_function_list *p = NULL;
  struct exit_function_list *l;
  struct exit_function_list *n;
  struct exit_function *r = NULL;
  size_t i = 0;

  if (__exit_funcs_done)
    /* Exit code is finished processing all registered exit functions,
       therefore we fail this registration.  */
    return NULL;

  /* We keep track of currentl 'l', previous 'p', and before previous 'pp'
     nodes.  */
  for (l = *listp; l != NULL;
       pp = (p != NULL && l != NULL) ? p : pp, p = l, l = l->next)
    {
      if (l->type != el_none && l->type != type)
	continue;

      for (i = l->idx; i > 0; --i)
	if (l->fns[i - 1].flavor != ef_free)
	  break;

      if (i > 0)
	break;

      /* This block is completely unused.  */
      l->idx = 0;
      l->type = type;
    }

  if (l == NULL || i == array_length (l->fns))
    {
      /* The last entry in a block is used.  Use the first entry in the
	 previous block if it exists.  Otherwise create a new one.  */
      if (p == NULL || p->type != type)
        {
          n = calloc (1, sizeof (struct exit_function_list));
	  if (n == NULL)
	    return NULL;

	  /* The el_cxa nodes should be after el_at one in the list.  */
	  if (type == el_cxa)
	    {
	      /* It is the first element in the list, set the sentinel
		 pointer.  */
	      if (p == NULL)
		{
		  n->next = *listp;
		  *listp = n;
		  p = n;
		}
	      /* There is already an set of current type, add the new one
		 at the front.  */
	      else if (l && l->type == type)
		{
		  p->next = n;
		  n->next = l;
		  p = n;
		}
	      /* There is already an set, but with different type, and it is
		 the sentinel set.  We need to keep as the last one, since it
		 is static allocated and 'exit' deallocate all elements
		 except the last one.  */
	      else
		{
		  /* It copies the contents of the old element over the newly
		     allocated one, and uses it the new set.  */
		  *n = *p;
		  /* Clear the initial element or its prior contents.  */
		  memset (p, 0, sizeof (struct exit_function_list));

		  n->next = p;

		  /* Sets the sentinel points or update the list.  */
		  if (pp != NULL)
		    pp->next = n;
		  else
		    *listp = n;
		}
	    }
	  /* el_at blocks are always on before el_cxa, so just adds it on the
	     list.  */
	  else
	    {
              n->next = *listp;
              *listp = n;
	      p = n;
	    }
        }

      r = &p->fns[0];
      p->idx = 1;
      p->type = type;
    }
  else
    {
      /* There is more room in the block.  */
      r = &l->fns[i];
      l->idx = i + 1;
    }

  ++__new_exitfn_called;

  return r;
}

int __new_exitfn (struct exit_function_list **listp, enum ef_flavor flavor,
		  void *func, void *arg, void *dso_handle)
{
  struct exit_function *new;

  /* As a QoI issue we detect NULL early with an assertion instead of a
     SIGSEGV at program exit when the handler is run (bug 20544).  */
  assert (func != NULL);

  __libc_lock_lock (__exit_funcs_lock);

  new = __new_exitfn_entry (listp, flavor);
  if (new == NULL)
    {
      __libc_lock_unlock (__exit_funcs_lock);
      return -1;
    }

#ifdef PTR_MANGLE
  PTR_MANGLE (func);
#endif

  new->flavor = flavor;
  switch (flavor)
    {
      case ef_on:
	new->func.on.fn = func;
	new->func.on.arg = arg;
      break;
      case ef_at:
	new->func.at.fn = func;
      break;
      case ef_cxa:
	new->func.cxa.fn = func;
	new->func.cxa.arg = arg;
      break;
      default:
	assert (!"invalid flavor");
    }
  new->dso_handle = dso_handle;

  __libc_lock_unlock (__exit_funcs_lock);
  return 0;
}
