from i8c.runtime import TestCase
import struct

TestCase.import_builtin_constants()
TestCase.import_constants_from("infinity-nptl-constants.h")
TestCase.import_constants_from("infinity-nptl_db-constants.h")

class TestMapLwp2Thr(TestCase):
    TESTFUNC = "libpthread::map_lwp2thr(i)ip"
    MAIN_PID = 30000

    def setUp(self):
        # Create flags
        self.ps_get_register_called = False
        self.ps_get_thread_area_called = False
        # Store the address of __stack_user
        note = self.i8ctx.get_function(self.TESTFUNC)
        symbols = note.external_pointers
        self.assertEqual(len(symbols), 1)
        self.stack_user_p = symbols[0]

    def read_memory(self, fmt, addr):
        # The only dereference we do is __stack_user.next
        self.assertEqual(addr, self.stack_user_p + LIST_T_NEXT_OFFSET)
        return struct.pack(fmt, self.STACK_USER_NEXT)

    def call_i8core_getpid(self):
        """Implementation of i8core::getpid."""
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
        # The result is whatever ps_get_thread_area returned
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
        self.assertLess(abs(bias), 16384)

    def check_I8_TS_REGISTER_THREAD_AREA_result(self, result):
        # The result is whatever ps_get_thread_area returned
        self.assertTrue(self.ps_get_register_called)
        self.assertTrue(self.ps_get_thread_area_called)
        self.assertEqual(result[0], TD_OK)
        self.assertNotEqual(result[1], 0)
        self.assertEqual(result[1], self.PS_GET_TA_RESULT[1])

class TestMapLwp2Thr_uninit(TestMapLwp2Thr):
    STACK_USER_NEXT = NULL

    def test_map_lwp2thr(self):
        """map_lwp2thr (nptl uninitialized, lwpid == main PID)"""
        result = self.i8ctx.call(self.TESTFUNC, self.MAIN_PID)
        self.assertEqual(len(result), 2)
        self.assertEqual(result[0], TD_OK)
        self.assertEqual(result[1], NULL)

class TestMapLwp2Thr_uninit_wrongpid(TestMapLwp2Thr):
    STACK_USER_NEXT = NULL

    def test_map_lwp2thr(self):
        """map_lwp2thr (nptl uninitialized, lwpid != main PID)"""
        result = self.i8ctx.call(self.TESTFUNC, self.MAIN_PID + 1)
        self.assertEqual(len(result), 2)
        self.assertEqual(result[0], TD_ERR)

class TestMapLwp2Thr_init_getreg_fail(TestMapLwp2Thr):
    STACK_USER_NEXT = 0x1fff
    PS_GETREG_RESULT = PS_ERR, 0x23ff00fa
    PS_GET_TA_RESULT = PS_OK, 0x89ab1234

    def test_map_lwp2thr(self):
        """map_lwp2thr (nptl initialized, ps_get_register fails)"""
        self.lwpid = self.MAIN_PID + 1
        result = self.i8ctx.call(self.TESTFUNC, self.lwpid)
        self.assertEqual(len(result), 2)
        if self.ps_get_register_called:
            self.assertEqual(result[0], TD_ERR)
        else:
            # This failure isn't a problem for this platform
            self.check_I8_TS_CONST_THREAD_AREA_result(result)

class TestMapLwp2Thr_init_gta_fail(TestMapLwp2Thr):
    STACK_USER_NEXT = 0x1fff
    PS_GETREG_RESULT = PS_OK, 0x23ff00fa
    PS_GET_TA_RESULT = PS_ERR, 0x89ab1234

    def test_map_lwp2thr(self):
        """map_lwp2thr (nptl initialized, ps_get_thread_area fails)"""
        self.lwpid = self.MAIN_PID + 1
        result = self.i8ctx.call(self.TESTFUNC, self.lwpid)
        self.assertEqual(len(result), 2)
        if self.ps_get_thread_area_called:
            self.assertEqual(result[0], TD_ERR)
        else:
            # This failure isn't a problem for this platform
            self.check_I8_TS_REGISTER_result(result)

class TestMapLwp2Thr_init_gta_ok(TestMapLwp2Thr):
    STACK_USER_NEXT = 0x1fff
    PS_GETREG_RESULT = PS_OK, 0x23ff00fa
    PS_GET_TA_RESULT = PS_OK, 0x89ab1234

    def test_map_lwp2thr(self):
        """map_lwp2thr (nptl initialized, everything worked)"""
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
