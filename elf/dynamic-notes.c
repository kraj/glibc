#include <link.h>

const ElfW(Dyn) __dynamic_note __attribute__ ((section (".dynamic"))) =
  { .d_tag = DT_GNU_FLAGS_1, .d_un.d_val = DF_GNU_1_UNIQUE };
