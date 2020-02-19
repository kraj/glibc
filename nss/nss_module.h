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

#ifndef _NSS_MODULE_H
#define _NSS_MODULE_H

#include <nss.h>
#include <stdbool.h>

/* Typed function pointers for all functions that can be defined by a
   service module.  */
struct nss_module_functions
{
  nss_endaliasent *endaliasent;
  nss_endetherent *endetherent;
  nss_endgrent *endgrent;
  nss_endhostent *endhostent;
  nss_endnetent *endnetent;
  nss_endnetgrent *endnetgrent;
  nss_endprotoent *endprotoent;
  nss_endpwent *endpwent;
  nss_endrpcent *endrpcent;
  nss_endservent *endservent;
  nss_endsgent *endsgent;
  nss_endspent *endspent;
  nss_getaliasbyname_r *getaliasbyname_r;
  nss_getaliasent_r *getaliasent_r;
  nss_getcanonname_r *getcanonname_r;
  nss_getetherent_r *getetherent_r;
  nss_getgrent_r *getgrent_r;
  nss_getgrgid_r *getgrgid_r;
  nss_getgrnam_r *getgrnam_r;
  nss_gethostbyaddr2_r *gethostbyaddr2_r;
  nss_gethostbyaddr_r *gethostbyaddr_r;
  nss_gethostbyname2_r *gethostbyname2_r;
  nss_gethostbyname3_r *gethostbyname3_r;
  nss_gethostbyname4_r *gethostbyname4_r;
  nss_gethostbyname_r *gethostbyname_r;
  nss_gethostent_r *gethostent_r;
  nss_gethostton_r *gethostton_r;
  nss_getnetbyaddr_r *getnetbyaddr_r;
  nss_getnetbyname_r *getnetbyname_r;
  nss_getnetent_r *getnetent_r;
  nss_getnetgrent_r *getnetgrent_r;
  nss_getntohost_r *getntohost_r;
  nss_getprotobyname_r *getprotobyname_r;
  nss_getprotobynumber_r *getprotobynumber_r;
  nss_getprotoent_r *getprotoent_r;
  nss_getpublickey *getpublickey;
  nss_getpwent_r *getpwent_r;
  nss_getpwnam_r *getpwnam_r;
  nss_getpwuid_r *getpwuid_r;
  nss_getrpcbyname_r *getrpcbyname_r;
  nss_getrpcbynumber_r *getrpcbynumber_r;
  nss_getrpcent_r *getrpcent_r;
  nss_getsecretkey *getsecretkey;
  nss_getservbyname_r *getservbyname_r;
  nss_getservbyport_r *getservbyport_r;
  nss_getservent_r *getservent_r;
  nss_getsgent_r *getsgent_r;
  nss_getsgnam_r *getsgnam_r;
  nss_getspent_r *getspent_r;
  nss_getspnam_r *getspnam_r;
  nss_initgroups_dyn *initgroups_dyn;
  nss_netname2user *netname2user;
  nss_setaliasent *setaliasent;
  nss_setetherent *setetherent;
  nss_setgrent *setgrent;
  nss_sethostent *sethostent;
  nss_setnetent *setnetent;
  nss_setnetgrent *setnetgrent;
  nss_setprotoent *setprotoent;
  nss_setpwent *setpwent;
  nss_setrpcent *setrpcent;
  nss_setservent *setservent;
  nss_setsgent *setsgent;
  nss_setspent *setspent;
};

/* Untyped version of struct nss_module_functions, for consistent
   processing purposes.  */
typedef void *nss_module_functions_untyped[sizeof (struct nss_module_functions)
                                           / sizeof (void *)];

/* Initialization state of a NSS module.  */
enum nss_module_state
{
  nss_module_uninitialized,
  nss_module_loaded,
  nss_module_failed,
};

/* A NSS service module (potentially unloaded).  Client code should
   use the functions below.  */
struct nss_module
{
  /* Actual type is enum nss_module_state.  Use int due to atomic
     access.  Used in a double-checked locking idiom.  */
  int state;

  /* The function pointers in the module.  */
  union
  {
    struct nss_module_functions typed;
    nss_module_functions_untyped untyped;
  } functions;

  /* Only used for __libc_freeres unloading.  */
  void *handle;

  /* The next module in the list. */
  struct nss_module *next;

  /* The name of the module (as it appears in /etc/nsswitch.conf).  */
  char name[];
};

/* Allocates the NSS module NAME and places it into the global list.
   If it already exists in the list, return the pre-existing module.
   This does not actually load the module.  Returns NULL on memory
   allocation failure.  */
struct nss_module *__nss_module_allocate (const char *name) attribute_hidden;

/* Ensures that MODULE is in a loaded or failed state.  */
bool __nss_module_load (struct nss_module *module) attribute_hidden;

/* Ensures that MODULE is loaded and returns a pointer to the function
   NAME defined in it.  Returns NULL if MODULE could not be loaded, or
   if the function NAME is not defined in the module.  */
void *__nss_module_get_function (struct nss_module *module, const char *name)
  attribute_hidden;

/* Called from __libc_freeres.  */
void __nss_module_freeres (void) attribute_hidden;

#endif /* NSS_MODULE_H */
