/* Test for char_array.
   Copyright (C) 2017 Free Software Foundation, Inc.
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

#include <string.h>

#include <malloc/char_array-skeleton.c>

#include <malloc.h>
#include <mcheck.h>
#include <stdint.h>
#include <support/check.h>
#include <support/support.h>

static int
do_test (void)
{
  mtrace ();

  {
    struct char_array str;
    TEST_VERIFY_EXIT (char_array_init_empty (&str) == true);
    TEST_VERIFY_EXIT (char_array_length (&str) == 0);
    TEST_VERIFY_EXIT (char_array_is_empty (&str) == true);
    TEST_VERIFY_EXIT (strcmp (char_array_str (&str), "") == 0);
    char_array_free (&str);
  }

  {
    struct char_array str;
    TEST_VERIFY_EXIT (char_array_init_str (&str, "testing"));
    TEST_VERIFY_EXIT (char_array_length (&str) == strlen ("testing"));
    TEST_VERIFY_EXIT (char_array_pos (&str, 2) == 's');
    TEST_VERIFY_EXIT (char_array_is_empty (&str) == false);
    TEST_VERIFY_EXIT (strcmp (char_array_str (&str), "testing") == 0);
    char_array_free (&str);
  }

  {
    struct char_array str;
    TEST_VERIFY_EXIT (char_array_init_str_size (&str, "testing", 4));
    TEST_VERIFY_EXIT (char_array_length (&str) == 4);
    TEST_VERIFY_EXIT (char_array_pos (&str, 2) == 's');
    TEST_VERIFY_EXIT (char_array_is_empty (&str) == false);
    TEST_VERIFY_EXIT (strcmp (char_array_str (&str), "test") == 0);
    char_array_free (&str);
  }

  {
    struct char_array str;
    TEST_VERIFY_EXIT (char_array_init_str (&str, "testing"));
    TEST_VERIFY_EXIT (char_array_set_str (&str, "abcdef"));
    TEST_VERIFY_EXIT (strcmp (char_array_str (&str), "abcdef") == 0);
    TEST_VERIFY_EXIT (char_array_set_str_size (&str, "abcdef", 4));
    TEST_VERIFY_EXIT (strcmp (char_array_str (&str), "abcd") == 0);
    char_array_free (&str);
  }

  {
    struct char_array str;
    TEST_VERIFY_EXIT (char_array_init_str (&str, "testing"));
    TEST_VERIFY_EXIT (char_array_erase (&str, 4) == true);
    TEST_VERIFY_EXIT (char_array_length (&str) == strlen ("testing") - 1);
    TEST_VERIFY_EXIT (strcmp (char_array_str (&str), "testng") == 0);
    TEST_VERIFY_EXIT (char_array_erase (&str, char_array_length (&str))
		      == false);
    TEST_VERIFY_EXIT (char_array_length (&str) == strlen ("testing") - 1);
    TEST_VERIFY_EXIT (char_array_erase (&str, char_array_length (&str) - 1)
		      == true);
    TEST_VERIFY_EXIT (char_array_length (&str) == strlen ("testing") - 2);
    TEST_VERIFY_EXIT (strcmp (char_array_str (&str), "testn") == 0);
    char_array_free (&str);
  }

  {
    struct char_array str;
    TEST_VERIFY_EXIT (char_array_init_str (&str, "test"));
    TEST_VERIFY_EXIT (char_array_prepend_str (&str, "123"));
    TEST_VERIFY_EXIT (strcmp (char_array_str (&str), "123test") == 0);
    TEST_VERIFY_EXIT (char_array_length (&str) == strlen ("123test"));
    TEST_VERIFY_EXIT (char_array_append_str (&str, "456"));
    TEST_VERIFY_EXIT (strcmp (char_array_str (&str), "123test456") == 0);
    TEST_VERIFY_EXIT (char_array_length (&str) == strlen ("123test456"));
    TEST_VERIFY_EXIT (char_array_replace_str_pos (&str, 7, "789", 3));
    TEST_VERIFY_EXIT (strcmp (char_array_str (&str), "123test789") == 0);
    TEST_VERIFY_EXIT (char_array_length (&str) == strlen ("123test789"));
    TEST_VERIFY_EXIT (char_array_crop (&str, 7));
    TEST_VERIFY_EXIT (char_array_length (&str) == 7);
    TEST_VERIFY_EXIT (strcmp (char_array_str (&str), "123test") == 0);
    char_array_free (&str);
  }

  return 0;
}

#include <support/test-driver.c>
