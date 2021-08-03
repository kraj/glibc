/* Main infrastructure for tst-dlmopen-rtld-*
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

#ifndef _TST_DLMOPEN_MAIN_H
#define _TST_DLMOPEN_MAIN_H

/* dlmopen +/- RTLD_SHARED/RTLD_ISOLATE semantics:

   RTLD_ISOLATE's purpose is to suppress all shared behaviour,
   mainly used for LD_AUDIT code paths but available to the user
   and also useful for constructing test case preconditions.

   dlmopen should have the following behaviour:

   Notation:
        Number of namespace ('+' means make a new one in an empty NS)
        |
     [+]X[p] - p indicates a proxy
      |
      + -> new entry after the dlmopen call

      Need to be able to inspect:

      list of dl handles before we start (base state)
      list of dl handles after an action in each namespace
      ns of a given dl handle
      _real_ ns of a given handle (ie where does a proxy point)

      In the tables below each line represents a separate test.

      In practice many tests can be implemented in the same executable
      as some tests set up the required preconditions for other tests
      if they complete successfully.

      Target is a normal DSO:
      Before | Target NS | RTLD Flags | After  | handle NS | libc NS
      =======+===========+============+========+===========+=========
      dlmopen-rtld-shared1:
      -------+-----------+------------+--------+-----------+---------
       -     | 0         | -          | +0     | 0         | 0
       0     | 0         | -          | 0      | 0         | 0
       0     | 0         | SHARED     | 0      | 0         | 0
       0     | +         | -          | 0,+1   | 1         | 0
       0,1   | 0         | -          | 0,1    | 0         | 0
       0,1   | 0         | SHARED     | 0,1    | 0         | 0
      =======+===========+============+========+===========+=========
      dlmopen-rtld-shared2:
      -------+-----------+------------+--------+-----------+---------
       -     | 0         | SHARED     | +0     | 0         | 0
       0     | +         | SHARED     | 0,+1p  | 1p        | 0
       0,1p  | 0         | -          | 0,1p   | 0         | 0
       0,1p  | 0         | SHARED     | 0,1p   | 0         | 0
       0,1p  | 1         | -          | 0,1p   | 1p        | 0
       0,1p  | 1         | SHARED     | 0,1p   | 1p        | 0
      =======+===========+============+========+===========+=========
      dlmopen-rtld-shared3
      -------+-----------+------------+--------+-----------+---------
       -     | +         | -          | +1     | 1         | 0
       1     | 0         | -          | +0,1   | 0         | 0
       0,1   | 1         | -          | 0,1    | 1         | 0
       0,1   | 1         | SHARED     | 0,1    | ERR       | -
      =======+===========+============+========+===========+=========
      dlmopen-rtld-shared4
      -------+-----------+------------+--------+-----------+---------
       -     | +         | SHARED     | +0,+1p | 1p        | 0
      =======+===========+============+========+===========+=========
      dlmopen-rtld-shared5
      -------+-----------+------------+--------+-----------+---------
       1     | 0         | SHARED     | +0,1   | 0         | 0
      =======+===========+============+========+===========+=========
      dlmopen-rtld-shared6
      -------+-----------+------------+--------+-----------+---------
       1     | 1         | -          | 1      | 0         | 0
       1     | 1         | SHARED     | 1      | ERR       | -

      Target is a DF_GNU_1_UNIQUE DSO:
      Before | Target NS | RTLD Flags | After  | handle NS | libc NS
      =======+===========+============+========+===========+=========
      dlmopen-rtld-unique1:
      -------+-----------+------------+--------+-----------+---------
       -     | 0         | -          | +0     | 0         | 0
       0     | 0         | -          | 0      | 0         | 0
       0     | 0         | SHARED     | 0      | 0         | 0
       0     | +         | -          | 0,+1p  | 1p        | 0
       0,1p  | 0         | -          | 0,1p   | 0         | 0
       0,1p  | 0         | SHARED     | 0,1p   | 0         | 0
       0,1p  | 1         | -          | 0,1p   | 1p        | 0
       0,1p  | 1         | SHARED     | 0,1p   | 1p        | 0
      =======+===========+============+========+===========+=========
      dlmopen-rtld-unique2:
      -------+-----------+------------+--------+-----------+---------
       -     | 0         | SHARED     | +0     | 0         | 0
       0     | +         | SHARED     | 0,+1p  | 1p        | 0
      =======+===========+============+========+===========+=========
      dlmopen-rtld-unique3:
      -------+-----------+------------+--------+-----------+---------
       -     | +         | -          | +0,+1p | 1p        | 0
      =======+===========+============+========+===========+=========
      dlmopen-rtld-unique4:
      -------+-----------+------------+--------+-----------+---------
       -     | +         | SHARED     | +0,+1p | 1p        | 0
      =======+===========+============+========+===========+=========
      dlmopen-rtld-unique5:
      -------+-----------+------------+--------+-----------+---------
       -     | +         | ISOLATE    | +1     | 1         | 1
       1     | 0         | -          | +0,1   | 0         | 0
       0,1   | 0         | -          | 0,1    | 0         | 0
       0,1   | 0         | SHARED     | 0,1    | 0         | 0
       0,1   | 1         | -          | 0,1    | ERR       | -
      =======+===========+============+========+===========+=========
      dlmopen-rtld-unique6:
      -------+-----------+------------+--------+-----------+---------
       -     | +1        | ISOLATE    | +1     | 1         | 1
       1     | 1         | -          | 1      | ERR       | -
       1     | 1         | SHARED     | 1      | ERR       | -
       1     | 0         | SHARED     | +0,1   | 0         | 0
       0,1   | 1         | SHARED     | 0,1    | ERR       | -
*/

#include "tst-dlmopen-common.h"
#include <dl-dtprocnum.h>
#include <link.h>
#include <array_length.h>
#include <support/check.h>
#include <support/support.h>
#include <support/test-driver.h>

#define DSO_NORMAL "$ORIGIN/tst-dlmopen-sharedmod-norm.so"
#define DSO_UNIQUE "$ORIGIN/tst-dlmopen-sharedmod-uniq.so"
#define DSO_TESTFN "rtld_shared_testfunc"
#define DSO_NAMESTUB "tst-dlmopen-sharedmod-"
#define MAX_NS 16

typedef enum
{
  NONE  = 0,
  DSO   = 1,
  PROXY = 2,
  NEW   = 4,
} dso_type;

typedef struct
{
  const char *name;
  const char *desc;
  int is_prep_stage;
  const char *dso_name;
  int failure;

  struct
  {
    const char *dso_path;
    Lmid_t ns;
    int flags;
  } args;

  dso_type preloaded[MAX_NS];
  dso_type loaded[MAX_NS];
  dso_type handle_type;
  Lmid_t handle_ns;
  Lmid_t free_ns;
} dlmopen_test_spec;

typedef struct { void *handle; Lmid_t ns; } test_handle;
static test_handle cached_handles[32];

static void
cache_test_handle (void *handle, Lmid_t ns)
{
  int i;

  for (i = 0; i < array_length (cached_handles); i++)
    if (cached_handles[i].handle == NULL)
      {
        cached_handles[i].handle = handle;
        cached_handles[i].ns = ns;
        break;
      }
}

__attribute__((unused))
static struct link_map *
link_map_of_dl_handle (void *handle)
{
  struct link_map *lm = NULL;

  if (dlinfo (handle, RTLD_DI_LINKMAP, &lm) == 0)
    return lm;

  printf ("dlinfo (LINKMAP) for %s in %s failed: %s\n",
          LIBC_SO, __func__, dlerror ());

  return NULL;
}

__attribute__((unused))
static Lmid_t
ns_of_dl_handle (void *handle)
{
  Lmid_t ns = 0;

  if (dlinfo (handle, RTLD_DI_LMID, &ns) == 0)
    return ns;

  printf ("dlinfo (LMID) for %s in %s failed: %s\n",
          LIBC_SO, __func__, dlerror ());

  return -1;
}

__attribute__((unused))
static Lmid_t
real_ns_of_dl_handle (void *handle)
{
  Lmid_t ns = 0;
  struct link_map *lm = link_map_of_dl_handle (handle);

  if (lm == NULL)
    return -1;

  if (lm->l_proxy)
    ns = ns_of_dl_handle ((void *) lm->l_real);
  else
    ns = ns_of_dl_handle (handle);

  return ns;
}

__attribute__((unused))
static const char *
str_soname (const char *name)
{
  char *slash = NULL;

  if (name == NULL)
    return NULL;

  if ((slash = strrchr (name, '/')))
    return ++slash;
  else
    return name;
}

__attribute__((unused))
static const char *
lm_name (struct link_map *lm)
{
  if (lm)
    return lm->l_name;

  return NULL;
}


static int
dlm_dso_is_loaded (void *handle)
{
  if (handle != RTLD_DEFAULT)
    {
      Dl_info sinfo = {};

      /* If it wasn't dynamically loaded it can be of no interest to our test(s).  */
      if (((struct link_map *) handle)->l_type != lt_loaded)
        return 0;

      /* Skip link map entries that have not yet been promoted to dl*foo* handles.  */
      if (((struct link_map *) handle)->l_searchlist.r_list  == NULL ||
          ((struct link_map *) handle)->l_searchlist.r_nlist == 0)
        return 0;

      verbose_printf ("checking %p %s for %s\n",
                      handle, ((struct link_map *)handle)->l_name, DSO_TESTFN);

      void *symbol = dlsym (handle, DSO_TESTFN);

      dladdr (symbol, &sinfo);
      verbose_printf ("  -> %s (in %s (%p))\n",
                      sinfo.dli_fname,
                      sinfo.dli_sname,
                      sinfo.dli_saddr);

      return symbol ? 1 : 0;
    }

  for (int i = 0; i < array_length (cached_handles); i++)
    {
      if (cached_handles[i].handle == NULL)
        break;

      if (((struct link_map *) cached_handles[i].handle)->l_type != lt_loaded)
        continue;

      verbose_printf ("checking %p %s for %s\n",
                      cached_handles[i].handle,
                      ((struct link_map *)cached_handles[i].handle)->l_name,
                      DSO_TESTFN);

      if (dlsym (cached_handles[i].handle, DSO_TESTFN) != NULL)
        return 1;
    }

  return 0;
}

static int
call_testfunc (dlmopen_test_spec *test, void *handle)
{
  Dl_info dli = {};
  struct link_map *lm = NULL;
  dlmopen_testfunc func = NULL;
  dlmopen_testresult *result = NULL;

  if (handle != RTLD_DEFAULT)
    func = dlsym (handle, DSO_TESTFN);

  if (func == NULL)
    FAIL_RET ("test function %s not found\n", DSO_TESTFN);

  result = (func)();

  if (result == NULL)
    FAIL_RET ("test function %s returned NULL\n", DSO_TESTFN);

  dladdr1 (result->free, &dli, (void **)&lm, RTLD_DL_LINKMAP);

  if (lm == NULL)
    FAIL_RET ("free() implementation from test module is invalid\n");

  if (lm->l_ns != test->free_ns)
    FAIL_RET ("free() function from test module was from ns %ld, "
	      "expected %ld\n", lm->l_ns, test->free_ns);

  printf ("%s: %s: %s in ns %ld using free() from ns %ld: OK\n",
          test->name, test->args.dso_path, DSO_TESTFN,
          test->handle_ns, lm->l_ns);

  return 1;
}

static void *
link_map_snapshot_array (void *handle, void *func, Lmid_t ns, size_t *len)
{
  struct link_map *lm = NULL;
  struct link_map *start = NULL;

  if (len != NULL)
    *len = 0;

  if (handle != NULL)
    TEST_COMPARE (dlinfo (handle, RTLD_DI_LINKMAP, &lm), 0);
  else if (func != NULL)
    TEST_VERIFY (dladdr1 (func, &(Dl_info) {}, (void **)&lm,
			   RTLD_DL_LINKMAP)
		 != 0);
  else if (ns >= LM_ID_BASE)
    {
      for (int i = 0; i < array_length(cached_handles); i++)
        {
          if (cached_handles[i].handle == NULL)
            break;

          if (cached_handles[i].ns != ns)
            continue;

          TEST_COMPARE (dlinfo (cached_handles[i].handle, RTLD_DI_LINKMAP,
			       	&lm),
			0);
          break;
        }
    }

  if (lm == NULL)
    return NULL;

  start = lm;

  while (start->l_prev)
    start = start->l_prev;

  size_t lm_size = 0;

  for (lm = start; lm; lm = lm->l_next)
    lm_size++;

  struct link_map **lm_list = xcalloc (lm_size + 1,
                                       sizeof (struct link_map *));

  if (len != NULL)
    *len = lm_size;

  int i = 0;

  for (lm = start; lm; lm = lm->l_next)
    lm_list[i++] = lm;
  lm_list[i] = NULL;

  return lm_list;
}

__attribute__((unused))
static int
search_link_map_array (struct link_map **lma, size_t len, void *handle)
{
  if (lma == NULL)
    return 0;

  if (len == 0)
    return 0;

  struct link_map *target = link_map_of_dl_handle (handle);

  for (int i = 0; i < len; i++)
    {
      if (handle == (struct link_map *)(lma[i]))
        return 1;

      if (target->l_proxy)
        if (target->l_real == (struct link_map *)(lma[i]))
          return 1;
    }

  return 0;
}

#define TRACE(fmt, ...) \
  if (test_verbose) \
      printf ("  find-test-dso (%s @ %d): " fmt "\n", __FILE__, __LINE__, \
	      ##__VA_ARGS__)

static struct link_map *
find_test_dso_in_link_map_array (struct link_map **lma, size_t len)
{
  if (lma == NULL)
    return NULL;

  if (len == 0)
    return NULL;

  for (int i = 0; i < len; i++)
    {
      if ((lma[i] == NULL) || (lma[i]->l_name == NULL))
        continue;

      TRACE ("%p [%d/%d] %s", lma, i, (int)len - 1,
             lma[i] ? (lma[i]->l_name ?: "???.so") : "NULL" );

      if (strstr (lma[i]->l_name, DSO_NAMESTUB) != NULL)
        if (dlsym (lma[i], DSO_TESTFN) != NULL)
          return (struct link_map *)lma[i];
    }

  return NULL;
}

__attribute__((unused))
static void *
link_map_list (void *handle, void *func, int *len, Lmid_t *ns)
{
  struct link_map *lm = NULL;
  struct link_map *start = NULL;

  if (len)
    *len = 0;

  if (handle != NULL)
    TEST_COMPARE (dlinfo (handle, RTLD_DI_LINKMAP, &lm), 0);
  else if (func != NULL)
    TEST_COMPARE (dladdr1 (func, &(Dl_info) {}, (void **)&lm,
			   RTLD_DL_LINKMAP),
		  0);

  if (lm == NULL)
    return NULL;

  if (ns)
    *ns = lm->l_ns;

  /* Rewind to start of link map list:  */
  start = lm;

  while (start->l_prev != NULL)
    start = start->l_prev;

  size_t lm_size = 0;

  for (lm = start; lm != NULL; lm = lm->l_next)
    lm_size++;

  if (len != NULL)
    *len = lm_size;

  return start;
}

static bool
process_test_spec (dlmopen_test_spec *test)
{
  void *handle = NULL;
  size_t lm_before_len[MAX_NS] = { 0 };
  size_t lm_after_len[MAX_NS] = { 0 };
  struct link_map **lm_before[MAX_NS] = { NULL };
  struct link_map **lm_after[MAX_NS] = { NULL };
  struct link_map *preloads[MAX_NS] = { NULL };
  int want_preload = 0;
  int test_status = false;

  TRACE ("LD_AUDIT = %s", getenv("LD_AUDIT") ?: "-");
  TRACE ("BEFORE SNAPSHOTS: %p", &lm_before[0]);
  TRACE ("AFTER  SNAPSHOTS: %p", &lm_after[0]);
  TRACE ("preloads        : %p", preloads);
  TRACE ("setup done");

  if (test->args.dso_path && *test->args.dso_path && !test->dso_name)
    test->dso_name = str_soname (test->args.dso_path);

  TRACE ("DSO short name: %s", test->dso_name);

  /* Get the existing link map contents before the test runs:  */
  lm_before[0] = link_map_snapshot_array (NULL, process_test_spec,
                                          LM_ID_BASE, &lm_before_len[0]);
  for (int i = 1; i < MAX_NS; i++)
    lm_before[i] = link_map_snapshot_array (NULL, NULL, i, &lm_before_len[i]);

  TRACE ("link map snapshots cached");

  for (int i = 0; i < MAX_NS; i++)
    {
      if (test->preloaded[i] & PROXY)
        {
          struct link_map **lm = lm_before[i];
          want_preload++;

          if (lm != NULL)
            for (int j = 0; (preloads[i] == NULL) && (j < lm_before_len[i]); j++)
              if (dlm_dso_is_loaded (lm[j]) && lm[j]->l_proxy)
                preloads[i] = lm[j];

          if (preloads[i] == NULL)
	    FAIL_RET ("needed proxy for %s preloaded in NS %d, not found\n",
                      test->dso_name, i);
        }
      else if (test->preloaded[i] & DSO)
        {
          struct link_map **lm = lm_before[i];
          int lm_max = lm_before_len[i];
          want_preload++;

          if (lm != NULL)
            for (int j = 0; !preloads[i] && (j < lm_max); j++)
              {
                if (dlm_dso_is_loaded (lm[j]) && !lm[j]->l_proxy)
                  preloads[i] = lm[j];
              }
          if (!preloads[i])
	    FAIL_RET ("needed %s preloaded in NS %d, not found\n",
                      test->dso_name, i);
        }
    }
  TRACE ("preload checks (A)");

  if (dlm_dso_is_loaded (RTLD_DEFAULT))
    {
      /* Test DSO module must _not_ be preloaded, and is:  */
      if (!want_preload)
	FAIL_RET ("DSO %s unexpectedly loaded before test\n",
		  test->dso_name);
    }
  else
    {
      /* DSO is not loaded, and must be.  In theory we can never see
         this error as it should be caught by the preceding preload loop.  */
      if (want_preload)
	FAIL_RET ("DSO %s must be preloaded (and is not)\n",
		  test->args.dso_path);
    }
  TRACE ("preload checks (B) %s", test->name);

  if (!(test->args.flags & (RTLD_NOW | RTLD_LAZY)))
    test->args.flags |= RTLD_NOW;

  handle = dlmopen (test->args.ns, test->args.dso_path, test->args.flags);
  TRACE ("dlmopen returned %p", handle);

  if (handle == NULL)
    {
      const char *status = "failed";
      const char *plabel = "";

      if (test->failure)
        {
          test_status = true;

          printf ("%s: dlmopen(%s, %d, 0x%0x) failed: OK (EXPECTED)\n",
                  test->name, test->args.dso_path,
                  (int)test->args.ns, (int)test->args.flags);
          printf ("Returned: %p\n\n", handle);

          goto cleanup;
        }

      if (test->is_prep_stage)
        plabel = "(during setup of preconditions): ";

      if (test->args.ns == LM_ID_BASE)
        FAIL_RET ("%sdlmopen (LM_ID_BASE, \"%s\", 0x%x) %s: %s\n",
		  plabel, test->args.dso_path, test->args.flags, status,
		  dlerror ());
      else
        FAIL_RET ("%sdlmopen (%d, \"%s\", 0x%x) %s: %s\n",
		  plabel, (int)test->args.ns, test->args.dso_path,
		  test->args.flags, status, dlerror ());
    }
  else if (test->failure)
    FAIL_RET ("dlmopen() call should have failed, but did not\n");

  TRACE ("return status checked");

  if (!dlm_dso_is_loaded (handle))
    FAIL_RET ("DSO %s (%p) missing function (%s)\n",
              test->args.dso_path, handle, DSO_TESTFN);

  TRACE ("loaded DSO sanity checked");

  Lmid_t hns = ns_of_dl_handle (handle);
  Lmid_t real_hns = real_ns_of_dl_handle (handle);
  Lmid_t proxy_ns = 0;

  call_testfunc (test, handle);
  TRACE (DSO_TESTFN "called");

  cache_test_handle (handle, hns);
  TRACE ("handle %p cached (ns %d)", handle, (int)hns);

  /* If the real ns was different to the apparent one
     then we have a proxy and we need to shuffle the values,
     else leave the proxy ns as 0 as an expect.proxy_ns of 0
     means we weren't expecting a proxy:  */
  if (real_hns != hns)
    {
      proxy_ns = hns;
      hns = real_hns;
    }

  if (proxy_ns)
    printf ("Returned: proxy ns:%d (real ns: %d)\n\n", (int)proxy_ns, (int)hns);
  else
    printf ("Returned: dso ns:%d\n\n", (int)hns);

  Lmid_t expected;
  if (test->handle_type & PROXY)
    expected = proxy_ns;
  else
    expected = hns;

  TRACE("check expected ns %d", (int)expected);

  if (test->args.ns == LM_ID_NEWLM)
    {
      if (expected <= LM_ID_BASE)
	FAIL_RET ("DSO should have been in NS > %d, was in %d",
                  LM_ID_BASE, (int)expected);

      /* For any cases where we can't predict the namespace in advance:  */
      if (test->handle_ns == LM_ID_NEWLM)
        test->handle_ns = expected;
    }
  else
    {
      if (test->args.ns != expected)
	FAIL_RET ("DSO should have been in NS %ld, was in %ld",
		  expected, test->args.ns);
    }

  TRACE("ns %d Ok", (int)expected);

if (test->handle_type & PROXY) /* Expecting a proxy.  */
    {
      if (proxy_ns != 0) /* Got a proxy.  */
        {
          if (test->handle_ns != proxy_ns) /* But not in the right place.  */
            FAIL_RET ("DSO proxy should have been in ns %ld, was in %ld",
                      test->handle_ns, proxy_ns);
        }
      else /* Didn't get a proxy.  */
        FAIL_RET ("DSO should have been a proxy in ns %ld,"
		  " was a non-proxy in ns %ld\n",
                  test->handle_ns, hns);
    }
 else /* Not expecting a proxy.  */
    {
      if (proxy_ns > 0)
	FAIL_RET ("DSO should NOT have been a proxy,"
		  " was a proxy in ns %ld (real ns %ld)",
		  proxy_ns, hns);

      if (test->handle_ns != hns)
	FAIL_RET ("DSO should have been in ns %ld,"
                 " was in ns %ld\n",
		 test->handle_ns, hns);
    }
  TRACE ("proxy status Ok");

  /* Get the new link map contents after the test has run:  */
  lm_after[0] = link_map_snapshot_array (NULL, process_test_spec,
                                         LM_ID_BASE, &lm_after_len[0]);
  for (int i = 1; i < MAX_NS; i++)
    lm_after[i] = link_map_snapshot_array (NULL, NULL, i, &lm_after_len[i]);

  for (int i = 0; i < MAX_NS; i++)
    {
      TRACE ("checking status of NS %d", i);
      void *old_handle =
        find_test_dso_in_link_map_array (lm_before[i], lm_before_len[i]);
      TRACE ("old handle is %p", old_handle);

      void *new_handle =
        find_test_dso_in_link_map_array (lm_after[i], lm_after_len[i]);
      TRACE ("new handle is %p", new_handle);

      if (test->loaded[i] == NONE)
        {
          TRACE ("ns %d requirement is NONE", i);
          if (old_handle != NULL)
	    FAIL_RET ("Unexpected preload DSO %s in ns %d\n",
		      lm_name (old_handle), i);
          if (new_handle != NULL)
	    FAIL_RET ("Unexpected new DSO %s in ns %d\n",
		      lm_name (new_handle), i);
          continue;
        }

      if (test->loaded[i] & NEW)
        {
          TRACE ("ns %d requirement is NEW", i);
          if (old_handle != NULL)
	    FAIL_RET ("DSO in ns %d should have been a new load,"
                      " found to have been preloaded", i);
          if (new_handle == NULL)
	    FAIL_RET ("Expected DSO in ns %d, not found", i);
        }
      else
        {
          TRACE ("ns %d requirement is OLD", i);
          if (new_handle == NULL)
	    FAIL_RET ("Expected new DSO in ns %d, not found", i);

          if (old_handle != new_handle)
	    FAIL_RET ("DSO in ns %d changed. This should be impossible, "
		      "sanity check the test code\n", i);
        }

      if (test->loaded[i] & PROXY)
        {
          TRACE ("rechecking DSO status in ns %d", i);
          if (!((struct link_map *)new_handle)->l_proxy)
	    FAIL_RET ("DSO in ns %d should be a proxy but is not", i);
        }
      else
        {
          TRACE ("rechecking proxy status in ns %d", i);
          if (((struct link_map *)new_handle)->l_proxy)
	    FAIL_RET ("DSO in ns %d should NOT be a proxy but is", i);
        }
    }

  test_status = true;

 cleanup:
  for (int i = 0; i < MAX_NS; i++)
     free (lm_after[i]);
  for (int i = 0; i < MAX_NS; i++)
    free (lm_before[i]);

  return test_status;
}

#endif /* _TST_DLMOPEN_MAIN_H  */
