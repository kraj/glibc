/* Global list of NSS service modules.
   Copyright (c) 2020 Free Software Foundation, Inc.
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

#include <nss_module.h>

#include <array_length.h>
#include <assert.h>
#include <atomic.h>
#include <dlfcn.h>
#include <gnu/lib-names.h>
#include <libc-lock.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Suffix after .so of NSS service modules.  */
static const char *const __nss_shlib_revision = LIBNSS_FILES_SO + 15;

/* A single-linked list used to implement a mapping from names to NSS
   modules.  (Most systems only use five or so service modules, so a
   list is sufficient here.)  Elements of this list are never freed
   during normal operation.  */
static struct nss_module *nss_module_list;

/* Covers the list and also loading of individual NSS service
   modules.  */
__libc_lock_define (static, nss_module_list_lock);

struct nss_module *
__nss_module_allocate (const char *name)
{
  __libc_lock_lock (nss_module_list_lock);

  struct nss_module *result = NULL;
  for (struct nss_module *p = nss_module_list; p != NULL; p = p->next)
    if (strcmp (p->name, name) == 0)
      {
        /* Return the previously existing object.  */
        result = p;
        break;
      }

  if (result == NULL)
    {
      /* Allocate a new list entry if the name was not found in the
         list.  */
      size_t name_length = strlen (name);
      result = malloc (sizeof (*result) + name_length + 1);
      if (result != NULL)
        {
          result->state = nss_module_uninitialized;
          memcpy (result->name, name, name_length + 1);
          result->handle = NULL;
          result->next = nss_module_list;
          nss_module_list = result;
        }
    }

  __libc_lock_unlock (nss_module_list_lock);
  return result;
}

/* Long enough to store the name of any function.  */
typedef char function_name[18];

/* This must be lexicographically sorted and match struct
   nss_module_functions.  */
static const function_name nss_function_name_array[] =
  {
   "endaliasent",
   "endetherent",
   "endgrent",
   "endhostent",
   "endnetent",
   "endnetgrent",
   "endprotoent",
   "endpwent",
   "endrpcent",
   "endservent",
   "endsgent",
   "endspent",
   "getaliasbyname_r",
   "getaliasent_r",
   "getcanonname_r",
   "getetherent_r",
   "getgrent_r",
   "getgrgid_r",
   "getgrnam_r",
   "gethostbyaddr2_r",
   "gethostbyaddr_r",
   "gethostbyname2_r",
   "gethostbyname3_r",
   "gethostbyname4_r",
   "gethostbyname_r",
   "gethostent_r",
   "gethostton_r",
   "getnetbyaddr_r",
   "getnetbyname_r",
   "getnetent_r",
   "getnetgrent_r",
   "getntohost_r",
   "getprotobyname_r",
   "getprotobynumber_r",
   "getprotoent_r",
   "getpublickey",
   "getpwent_r",
   "getpwnam_r",
   "getpwuid_r",
   "getrpcbyname_r",
   "getrpcbynumber_r",
   "getrpcent_r",
   "getsecretkey",
   "getservbyname_r",
   "getservbyport_r",
   "getservent_r",
   "getsgent_r",
   "getsgnam_r",
   "getspent_r",
   "getspnam_r",
   "initgroups_dyn",
   "netname2user",
   "setaliasent",
   "setetherent",
   "setgrent",
   "sethostent",
   "setnetent",
   "setnetgrent",
   "setprotoent",
   "setpwent",
   "setrpcent",
   "setservent",
   "setsgent",
   "setspent",
  };

_Static_assert ((array_length (nss_function_name_array) * sizeof (void *))
                == sizeof (struct nss_module_functions),
                "length of nss_module_name_array");
_Static_assert (array_length (nss_function_name_array)
                == array_length ((struct nss_module) { 0 }.functions.untyped),
                "length of nss_module_name_array");

static bool
module_load (struct nss_module *module)
{
  void *handle;
  {
    char *shlib_name;
    if (asprintf (&shlib_name, "libnss_%s.so%s",
                  module->name, __nss_shlib_revision) < 0)
      /* This is definitely a temporary failure.  Do not update
         module->state.  This will trigger another attempt at the next
         call.  */
      return false;

    handle = __libc_dlopen (shlib_name);
    free (shlib_name);
  }

  if (handle == NULL)
    {
      /* dlopen failure.  We do not know if this a temporary or
         permanent error.  See bug 22041.  Update the state using the
         double-checked locking idiom.  */

      __libc_lock_lock (nss_module_list_lock);
      bool result = result;
      switch ((enum nss_module_state) atomic_load_acquire (&module->state))
        {
        case nss_module_uninitialized:
          atomic_store_release (&module->state, nss_module_failed);
          result = false;
          break;
        case nss_module_loaded:
          result = true;
          break;
        case nss_module_failed:
          result = false;
          break;
        }
      __libc_lock_unlock (nss_module_list_lock);
      return result;
    }

  nss_module_functions_untyped pointers;

  for (size_t idx = 0; idx < array_length (nss_function_name_array); ++idx)
    {
      char *function_name;
      if (asprintf (&function_name, "_nss_%s_%s",
                    module->name, nss_function_name_array[idx]) < 0)
        {
          /* Definitely a temporary error.  */
          __libc_dlclose (handle);
          return false;
        }
      pointers[idx] = __libc_dlsym (handle, function_name);
      free (function_name);
#ifdef PTR_MANGLE
      PTR_MANGLE (pointers[idx]);
#endif
    }

  /* Intall the function pointers, following the double-checked
     locking idiom.  Delay this after all processing, in case loading
     the module triggers unwinding.  */
  __libc_lock_lock (nss_module_list_lock);
  switch ((enum nss_module_state) atomic_load_acquire (&module->state))
    {
    case nss_module_uninitialized:
    case nss_module_failed:
      memcpy (module->functions.untyped, pointers,
              sizeof (module->functions.untyped));
      module->handle = handle;
      atomic_store_release (&module->state, nss_module_loaded);
      break;
    case nss_module_loaded:
      /* This does not actually unload the module, only the reference
         counter is decremented.  */
      __libc_dlclose (handle);
      break;
    }
  __libc_lock_unlock (nss_module_list_lock);
  return true;
}

/* Ensures that MODULE is in a loaded or failed state.  */
bool
__nss_module_load (struct nss_module *module)
{
  switch ((enum nss_module_state) atomic_load_acquire (&module->state))
    {
    case nss_module_uninitialized:
      return module_load (module);
    case nss_module_loaded:
      /* Loading has already succeeded.  */
      return true;
    case nss_module_failed:
      /* Loading previously failed.  */
      return false;
    }
  __builtin_unreachable ();
}

static int
name_search (const void *left, const void *right)
{
  return strcmp (left, right);
}

void *
__nss_module_get_function (struct nss_module *module, const char *name)
{
  if (!__nss_module_load (module))
    return NULL;

  function_name *name_entry = bsearch (name, nss_function_name_array,
                                       array_length (nss_function_name_array),
                                       sizeof (function_name), name_search);
  assert (name_entry != NULL);
  size_t idx = nss_function_name_array - name_entry;
  void *fptr = module->functions.untyped[idx];
#ifdef PTR_DEMANGLE
  PTR_DEMANGLE (fptr);
#endif
  return fptr;
}

void
__nss_module_freeres (void)
{
  struct nss_module *current = nss_module_list;
  while (current != NULL)
    {
      if (current->state == nss_module_loaded)
        __libc_dlclose (current->handle);

      struct nss_module *next = current->next;
      free (current);
      current = next;
    }
  nss_module_list = NULL;
}
