/* Test the comma-separated tunable string iterator.
   Copyright (C) 2026 Free Software Foundation, Inc.
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

#include <dl-tunables.h>
#include <dl-tunables-parse.h>

#include <array_length.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <support/check.h>
#include <support/next_to_fault.h>

struct token
{
  const char *str;
  bool disable;
};

struct test_case
{
  /* The suboptions string.  It is copied to a guarded buffer without the null
     terminator, so the iterator must honor the supplied length instead of
     relying on the terminator.  */
  const char *input;
  /* The expected suboptions.  */
  const struct token *tokens;
  size_t ntokens;
};

#define TOKENS(...) \
  (const struct token[]) { __VA_ARGS__ }, \
  array_length (((const struct token[]) { __VA_ARGS__ }))

static const struct test_case tests[] =
{
  { "", NULL, 0 },
  { "abc", TOKENS ({ "abc", false }) },
  { "a,b", TOKENS ({ "a", false }, { "b", false }) },
  { "a,-b,c",
    TOKENS ({ "a", false }, { "b", true }, { "c", false }) },
  { "-", TOKENS ({ "", true }) },
  { "--a", TOKENS ({ "-a", true }) },
  { "-a,-", TOKENS ({ "a", true }, { "", true }) },
  /* Empty suboptions are returned as empty strings, not disabled.  */
  { ",", TOKENS ({ "", false }) },
  { ",,", TOKENS ({ "", false }, { "", false }) },
  { ",a", TOKENS ({ "", false }, { "a", false }) },
  { "a,", TOKENS ({ "a", false }) },
  { "a,,b", TOKENS ({ "a", false }, { "", false }, { "b", false }) },
  { "-avx2,,-,avx512f,",
    TOKENS ({ "avx2", true }, { "", false }, { "", true },
            { "avx512f", false }) },
};

static void
check_tokens (const char *input, tunable_val_t *valp,
              const struct token *tokens, size_t ntokens)
{
  struct tunable_str_comma_state_t state;
  tunable_str_comma_init (&state, valp);

  size_t i = 0;
  while (true)
    {
      /* Poison the result so an uninitialized field does not pass by
         accident.  */
      struct tunable_str_comma_t t;
      memset (&t, 0xff, sizeof (t));

      if (!tunable_str_comma_next (&state, &t))
        break;

      if (i >= ntokens)
        FAIL_EXIT1 ("\"%s\": unexpected suboption %zu \"%.*s\"",
                    input, i, (int) t.len, t.str);

      printf ("info: \"%s\": suboption %zu \"%.*s\" (disable=%d)\n",
              input, i, (int) t.len, t.str, (int) t.disable);
      TEST_COMPARE_BLOB (t.str, t.len, tokens[i].str,
                         strlen (tokens[i].str));
      TEST_COMPARE (t.disable, tokens[i].disable);
      i++;
    }

  TEST_COMPARE (i, ntokens);
}

static void
check_guarded (const char *input, const struct token *tokens, size_t ntokens)
{
  size_t len = strlen (input);
  struct support_next_to_fault ntf = support_next_to_fault_allocate (len);
  memcpy (ntf.buffer, input, len);

  tunable_val_t val = { .strval = { ntf.buffer, len } };
  check_tokens (input, &val, tokens, ntokens);

  support_next_to_fault_free (&ntf);
}

static void
check_length (const char *input, size_t len, const struct token *tokens,
              size_t ntokens)
{
  tunable_val_t val = { .strval = { input, len } };
  check_tokens (input, &val, tokens, ntokens);
}

static int
do_test (void)
{
  for (size_t i = 0; i < array_length (tests); i++)
    check_guarded (tests[i].input, tests[i].tokens, tests[i].ntokens);

  /* The suboptions after the supplied length are not visible.  */
  check_length ("abcdef,gh", 4, TOKENS ({ "abcd", false }));
  check_length ("abc,def", 3, TOKENS ({ "abc", false }));
  check_length ("abc,def", 4, TOKENS ({ "abc", false }));
  check_length ("-abc,def", 4, TOKENS ({ "abc", true }));

  /* The null terminator stops the iteration before the supplied length.  */
  check_length ("a,b", 10, TOKENS ({ "a", false }, { "b", false }));
  check_length ("a,", 10, TOKENS ({ "a", false }));
  check_length (",", 10, TOKENS ({ "", false }));

  return 0;
}

#include <support/test-driver.c>
