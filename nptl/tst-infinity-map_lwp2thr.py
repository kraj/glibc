from i8c.runtime import TestCase
import struct

TestCase.import_builtin_constants()
TestCase.import_constants_from("infinity-nptl-constants.h")
TestCase.import_constants_from("infinity-nptl_db-constants.h")

class TestMapLwp2Thr(TestCase):
    TESTFUNC = "libpthread::map_lwp2thr(i)ip"
    MAIN_PID = 30000

    def setUp(self):
        # Set up the test address space for the __stack_user.next
        # dereference.
        with self.memory.builder() as mem:
            stack_user = mem.alloc("__stack_user")
            if self.STACK_USER_SETUP:
                stack_user_next = mem.alloc()
            else:
                stack_user_next = NULL
            stack_user.store_ptr(LIST_T_NEXT_OFFSET, stack_user_next)
        # Initialize flags so we can see what was called.
        self.ps_get_register_called = False
        self.ps_get_thread_area_called = False

    def call_procservice_getpid(self):
        """Implementation of procservice::getpid."""
        return self.MAIN_PID

    def call_procservice_get_register(self, lwpid, offset, size):
        """Implementation of procservice::get_register."""
        self.assertFalse(self.ps_get_register_called)
        result = getattr(self, "PS_GETREG_RESULT", None)
        if result is None:
            self.fail("unexpected ps_get_register")
        self.assertEqual(lwpid, self.lwpid)
        self.assertNotEqual(offset, self.lwpid)
        self.assertGreaterEqual(offset, 0)
        # We can't really say much about offset.  It's an offset into
        # a prgregset_t structure, so it's probably not huge and it's
        # probably aligned to the machine's wordsize.
        self.assertLess(offset, 128 * 8) # =128 64-bit registers (IA-64)
        bytes_per_word, check = divmod(self.i8ctx.wordsize, 8)
        self.assertNotEqual(bytes_per_word, 0)
        self.assertEqual(check, 0)
        self.assertEqual(offset % bytes_per_word, 0)
        self.assertIn(size, (8, 16, 32, 64))
        self.assertLessEqual(size, self.i8ctx.wordsize)
        self.ps_get_register_called = True
        return result

    def call_procservice_get_thread_area(self, lwpid, idx):
        """Implementation of procservice::get_thread_area."""
        self.assertFalse(self.ps_get_thread_area_called)
        result = getattr(self, "PS_GET_TA_RESULT", None)
        if result is None:
            self.fail("unexpected ps_get_thread_area")
        self.assertEqual(lwpid, self.lwpid)
        self.assertNotEqual(idx, self.lwpid)
        self.ps_get_thread_area_called = True
        return result

    def check_I8_TS_CONST_THREAD_AREA_result(self, result):
        # The result is whatever ps_get_thread_area returned.
        self.assertTrue(self.ps_get_thread_area_called)
        self.assertEqual(result[0], TD_OK)
        self.assertNotEqual(result[1], 0)
        self.assertEqual(result[1], self.PS_GET_TA_RESULT[1])

    def check_I8_TS_REGISTER_result(self, result):
        # The result is what ps_get_register returned with some
        # bias added.  We'll assume the bias is fairly small.
        self.assertTrue(self.ps_get_register_called)
        self.assertEqual(result[0], TD_OK)
        self.assertNotEqual(result[1], 0)
        bias = result[1] - self.PS_GETREG_RESULT[1]
        self.assertLess(abs(bias), 0x10000)

    def check_I8_TS_REGISTER_THREAD_AREA_result(self, result):
        # The result is whatever ps_get_thread_area returned.
        self.assertTrue(self.ps_get_register_called)
        self.assertTrue(self.ps_get_thread_area_called)
        self.assertEqual(result[0], TD_OK)
        self.assertNotEqual(result[1], 0)
        self.assertEqual(result[1], self.PS_GET_TA_RESULT[1])

class TestMapLwp2Thr_uninit(TestMapLwp2Thr):
    STACK_USER_SETUP = False

    def test_uninit(self):
        """Test map_lwp2thr with NPTL uninitialized"""
        result = self.i8ctx.call(self.TESTFUNC, self.MAIN_PID)
        self.assertEqual(len(result), 2)
        self.assertEqual(result[0], TD_OK)
        self.assertEqual(result[1], NULL)

class TestMapLwp2Thr_uninit_wrongpid(TestMapLwp2Thr):
    STACK_USER_SETUP = False

    def test_uninit_wrongpid(self):
        """Test map_lwp2thr with NPTL uninitialized and lwpid != main pid"""
        result = self.i8ctx.call(self.TESTFUNC, self.MAIN_PID + 1)
        self.assertEqual(len(result), 2)
        self.assertEqual(result[0], TD_ERR)

class TestMapLwp2Thr_getreg_fail(TestMapLwp2Thr):
    STACK_USER_SETUP = True
    PS_GETREG_RESULT = PS_ERR, 0x23ff00fa
    PS_GET_TA_RESULT = PS_OK, 0x89ab1234

    def test_ps_getreg_fail(self):
        """Check map_lwp2thr handles ps_get_register failures"""
        self.lwpid = self.MAIN_PID + 1
        result = self.i8ctx.call(self.TESTFUNC, self.lwpid)
        self.assertEqual(len(result), 2)
        if self.ps_get_register_called:
            self.assertEqual(result[0], TD_ERR)
        else:
            # This failure isn't a problem for this platform.
            self.check_I8_TS_CONST_THREAD_AREA_result(result)

class TestMapLwp2Thr_gta_fail(TestMapLwp2Thr):
    STACK_USER_SETUP = True
    PS_GETREG_RESULT = PS_OK, 0x23ff00fa
    PS_GET_TA_RESULT = PS_ERR, 0x89ab1234

    def test_ps_gta_fail(self):
        """Check map_lwp2thr handles ps_get_thread_area failures"""
        self.lwpid = self.MAIN_PID + 1
        result = self.i8ctx.call(self.TESTFUNC, self.lwpid)
        self.assertEqual(len(result), 2)
        if self.ps_get_thread_area_called:
            self.assertEqual(result[0], TD_ERR)
        else:
            # This failure isn't a problem for this platform.
            self.check_I8_TS_REGISTER_result(result)

class TestMapLwp2Thr_mainpath(TestMapLwp2Thr):
    STACK_USER_SETUP = True
    PS_GETREG_RESULT = PS_OK, 0x23ff00fa
    PS_GET_TA_RESULT = PS_OK, 0x89ab1234

    def test_mainpath(self):
        """Test the main path through map_lwp2thr"""
        self.lwpid = self.MAIN_PID + 1
        result = self.i8ctx.call(self.TESTFUNC, self.lwpid)
        self.assertEqual(len(result), 2)
        if self.ps_get_thread_area_called:
            if self.ps_get_register_called:
                self.check_I8_TS_REGISTER_THREAD_AREA_result(result)
            else:
                self.check_I8_TS_CONST_THREAD_AREA_result(result)
        else:
            self.check_I8_TS_REGISTER_result(result)
