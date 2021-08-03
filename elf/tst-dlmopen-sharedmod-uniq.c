#include "tst-dlmopen-common.h"
#include <signal.h>
#include <time.h>

static char user_name[255];

/* This initialiser checks that relocation from DF_GNU_1_UNIQUE
   dependencies (via DT_NEEDED) work properly - it will segfault
   if _dl_lookup_symbol_x isn't handling that case. The main test
   framework for dlmopen checks relocations/lookups via dlmopen.  */
void __attribute__ ((constructor))
check_relocation (void)
{
  char *user = getenv("USER");

  if (user == NULL)
    return;

  for (int i = 0; i < sizeof (user_name) && *(user + i); i++)
    user_name[i] = *(user + i);

  user[sizeof (user_name) - 1] = 0;
}

dlmopen_testresult *
rtld_shared_testfunc (void)
{
  static dlmopen_testresult result;

  result.name = "noop";
  result.free = free;
  result.timer_create = timer_create;

  return &result;
}
