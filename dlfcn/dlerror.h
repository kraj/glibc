#ifndef _DLERROR_H
#define _DLERROR_H

#include <dlfcn.h>
#include <ldsodefs.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* Source of the errstring member in struct dl_action_result, for
   finding the right deallocation routine.  */
enum dl_action_result_errstring_source
  {
   dl_action_result_errstring_constant, /* String literal, no deallocation.  */
   dl_action_result_errstring_rtld, /* libc in the primary namespace.  */
   dl_action_result_errstring_local, /* libc in the current namespace.  */
  };

struct dl_action_result
{
  int errcode;
  char errstring_source;
  bool returned;
  const char *objname;
  char *errstring;
};

/* Used to free the errstring member of struct dl_action_result in the
   dl_action_result_errstring_rtld case.  */
static inline void
dl_error_free (void *ptr)
{
#ifdef SHARED
  /* In the shared case, ld.so may use a different malloc than this
     namespace.  */
  GLRO (dl_error_free (ptr));
#else
  /* Call the implementation directly.  It still has to check for
     pointers which cannot be freed, so do not call free directly
     here.  */
  _dl_error_free (ptr);
#endif
}

/* Deallocate RESULT->errstring, leaving *RESULT itself allocated.  */
static inline void
dl_action_result_errstring_free (struct dl_action_result *result)
{
  switch (result->errstring_source)
    {
    case dl_action_result_errstring_constant:
      break;
    case dl_action_result_errstring_rtld:
      dl_error_free (result->errstring);
      break;
    case dl_action_result_errstring_local:
      free (result->errstring);
      break;
    }
}

static inline struct dl_action_result *
dl_action_result_malloc_failed (void)
{
  return (struct dl_action_result *) (intptr_t) -1;
}

/* Thread-local variable for storing dlfcn failures for subsequent
   reporting via dlerror.  */
extern __thread struct dl_action_result *__libc_dlerror_result
  attribute_tls_model_ie;
void __libc_dlerror_result_free (void) attribute_hidden;

#endif /* _DLERROR_H */
