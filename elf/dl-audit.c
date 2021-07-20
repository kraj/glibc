/* Audit common functions.
   Copyright (C) 2021 Free Software Foundation, Inc.
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

#include <ldsodefs.h>

#ifdef SHARED
void
_dl_audit_activity_map (struct link_map *l, int action)
{
  struct audit_ifaces *afct = GLRO(dl_audit);
  for (unsigned int cnt = 0; cnt < GLRO(dl_naudit); ++cnt)
    {
      if (afct->activity != NULL)
	afct->activity (&link_map_audit_state (l, cnt)->cookie, action);
      afct = afct->next;
    }
}

void
_dl_audit_activity_nsid (Lmid_t nsid, int action)
{
  struct link_map *head = GL(dl_ns)[nsid]._ns_loaded;
  if (__glibc_likely (GLRO(dl_naudit) == 0)
      || head == NULL || head->l_auditing)
    return;

  _dl_audit_activity_map (head, action);
}

bool
_dl_audit_objsearch (const char **name, const char **origname,
		     struct link_map *l, unsigned int code)
{
  if (__glibc_likely (GLRO(dl_naudit) == 0)
      || l == NULL || l->l_auditing
      || code == 0)
    return true;

  struct audit_ifaces *afct = GLRO(dl_audit);
  for (unsigned int cnt = 0; cnt < GLRO(dl_naudit); ++cnt)
    {
      if (afct->objsearch != NULL)
	{
	  const char *before = *name;
	  struct auditstate *state = link_map_audit_state (l, cnt);
	  *name = afct->objsearch (*name, &state->cookie, code);
	  if (*name == NULL)
	    return false;

	  if (origname != NULL && before != *name
	      && strcmp (before, *name) != 0)
	    {
	      if (__glibc_unlikely (GLRO(dl_debug_mask) & DL_DEBUG_FILES))
		_dl_debug_printf ("audit changed filename %s -> %s\n",
				  before, *name);

	      if (*origname == NULL)
		*origname = before;
	    }
	}
      afct = afct->next;
   }

  return true;
}

void
_dl_audit_objopen (struct link_map *l, Lmid_t nsid, bool check_audit)
{
  if (__glibc_likely (GLRO(dl_naudit) == 0)
      || (check_audit && GL(dl_ns)[l->l_ns]._ns_loaded->l_auditing))
    return;

  struct audit_ifaces *afct = GLRO(dl_audit);
  for (unsigned int cnt = 0; cnt < GLRO(dl_naudit); ++cnt)
    {
      if (afct->objopen != NULL)
	{
	  struct auditstate *state = link_map_audit_state (l, cnt);
	  state->bindflags = afct->objopen (l, nsid, &state->cookie);
	  l->l_audit_any_plt |= state->bindflags != 0;
	}

      afct = afct->next;
   }
}

void
_dl_audit_objclose (struct link_map *l, Lmid_t nsid)
{
  if (__glibc_likely (GLRO(dl_naudit) == 0)
      || GL(dl_ns)[l->l_ns]._ns_loaded->l_auditing)
    return;

  struct audit_ifaces *afct = GLRO(dl_audit);
  for (unsigned int cnt = 0; cnt < GLRO(dl_naudit); ++cnt)
    {
      if (afct->objclose != NULL)
	{
	  struct auditstate *state= link_map_audit_state (l, cnt);
	  /* Return value is ignored.  */
	  afct->objclose (&state->cookie);
	}

      afct = afct->next;
    }
}
#endif
