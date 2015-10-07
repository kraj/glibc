from i8c.runtime import TestCase

TestCase.import_builtin_constants()
TestCase.import_constants_from("infinity-nptl-constants.h")
TestCase.import_constants_from("infinity-nptl_db-constants.h")

class TestThread(object):
    def __init__(self, pid, policy=SCHED_OTHER, priority=0, tid=None):
        self.pid = pid
        self.policy = policy
        self.priority = priority
        self.tid = tid

    def write_into(self, buf):
        self.__buf = buf
        buf.store_s32(PTHREAD_PID_OFFSET, self.pid)
        buf.store_s32(PTHREAD_SCHEDPOLICY_OFFSET, self.policy)
        buf.store_s32(PTHREAD_SCHEDPARAM_SCHED_PRIORITY_OFFSET,
                      self.priority)
        if self.tid is not None:
            buf.store_s32(PTHREAD_TID_OFFSET, self.tid)

    @property
    def handle(self):
        return self.__buf.location

    def matches(self, test):
        if self.pid < 0:
            if not (self.pid == -test.MAIN_PID
                    or self.tid == test.MAIN_PID):
                return False
        elif self.pid != test.MAIN_PID:
            return False
        if self.priority < test.TI_PRIORITY:
            return False
        return True

class TestThrIter(TestCase):
    TESTFUNC = "libpthread::thr_iter(Fi(po)oiipi)i"
    MAIN_PID = 30000

    # Arguments call_thr_iter uses.
    TI_CALLBACK_ARG = lambda x: x + 3
    TI_STATE = TD_THR_ANY_STATE
    TI_PRIORITY = TD_THR_LOWEST_PRIORITY
    TI_SIGNO_MASK = TD_SIGNO_MASK
    TI_USER_FLAGS = TD_THR_ANY_USER_FLAGS

    def setUp(self):
        # Set up the address space.
        with self.memory.builder() as mem:
            self.__setup_threads(mem, "__stack_user", self.STACK_USER)
            self.__setup_threads(mem, "__stack_used", self.STACK_USED)

    def __setup_threads(self, mem, symname, threads):
        head = mem.alloc(symname)
        if threads is None:
            # This thread list is uninitialized.
            head.store_ptr(LIST_T_NEXT_OFFSET, NULL)
            return

        prev = head
        for src in threads:
            dst = mem.alloc()
            src.write_into(dst)

            list = dst + PTHREAD_LIST_OFFSET
            prev.store_ptr(LIST_T_NEXT_OFFSET, list)
            prev = list
        prev.store_ptr(LIST_T_NEXT_OFFSET, head)

    def call_procservice_getpid(self):
        """Implementation of procservice::getpid."""
        return self.MAIN_PID

    def recording_callback(self, handle, arg):
        self.assertEqual(arg, self.TI_CALLBACK_ARG)
        self.calls.append(handle)
        return 0

    def failing_callback(self, handle, arg):
        self.assertEqual(arg, self.TI_CALLBACK_ARG)
        return 1

    def call_thr_iter(self, callback):
        return self.i8ctx.call(self.TESTFUNC,
                               callback,
                               self.TI_CALLBACK_ARG,
                               self.TI_STATE,
                               self.TI_PRIORITY,
			       self.TI_SIGNO_MASK,
			       self.TI_USER_FLAGS)

    def run_standard_test(self, expect_ncalls, null_ok=False):
        # Check callback is called for the expected threads.
        self.calls = []
        result = self.call_thr_iter(self.recording_callback)
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0], TD_OK)
        self.check_calls(expect_ncalls, null_ok)
        # Check that callback errors are handled.
        if expect_ncalls != 0:
            result = self.call_thr_iter(self.failing_callback)
            self.assertEqual(len(result), 1)
            self.assertEqual(result[0], TD_DBERR)

    def check_calls(self, expect_ncalls, null_ok):
        expect, empty_count = [], 0
        for list in self.STACK_USER, self.STACK_USED:
            if not list:
                empty_count += 1
                continue
            for thread in list:
                if thread.matches(self):
                    expect.append(thread.handle)
        if empty_count == 2:
            expect.append(NULL) # faked main process
        # Check the list we've built seems right.
        self.assertEqual(len(expect), expect_ncalls)
        if not null_ok:
            self.assertNotIn(NULL, expect)
        # Now check our list matches what happened.
        self.assertEqual(self.calls, expect)

# Tests for unhandled filters.  libthread_db specifies these, but
# glibc's td_ta_thr_iter doesn't implement them.  The thr_iter
# note accepts the parameters but returns TD_NOCAPAB if anything
# other than a pass-through filter is specified.

class TestThrIter_unhandled(TestThrIter):
    STACK_USER = None
    STACK_USED = None

    def run_nocapab_test(self):
        result = self.call_thr_iter(self.failing_callback)
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0], TD_NOCAPAB)

class TestThrIter_state_unhandled(TestThrIter_unhandled):
    TI_STATE = TD_THR_ANY_STATE + 1

    def test_unhandled_state(self):
        """Test thr_iter with state != TD_THR_ANY_STATE"""
        self.run_nocapab_test()

class TestThrIter_signo_mask_unhandled(TestThrIter_unhandled):
    TI_SIGNO_MASK = TD_SIGNO_MASK + 1

    def test_unhandled_signo_mask(self):
        """Test thr_iter with signo_mask != TD_SIGNO_MASK"""
        self.run_nocapab_test()

class TestThrIter_user_flags_unhandled(TestThrIter_unhandled):
    TI_USER_FLAGS = TD_THR_ANY_USER_FLAGS + 1

    def test_unhandled_user_flags(self):
        """Test thr_iter with user_flags != TD_THR_ANY_USER_FLAGS"""
        self.run_nocapab_test()

# Tests with uninitialized and partly initialized thread lists.

class TestThrIter_both_uninit(TestThrIter):
    STACK_USER = None
    STACK_USED = None

    def test_both_uninit(self):
       """Test thr_iter with both lists uninitialized"""
       self.run_standard_test(1, True)

class TestThrIter_stack_user_uninit(TestThrIter):
    STACK_USER = None
    STACK_USED = []

    def test_stack_user_uninit(self):
        """Test thr_iter with __stack_user uninitialized"""
        # There is a tiny window in glibc where this setup can happen.
        self.run_standard_test(1, True)

class TestThrIter_both_empty(TestThrIter):
    STACK_USER = []
    STACK_USED = []

    def test_stack_user_uninit(self):
        """Test thr_iter with both lists initialized but empty"""
        # There is a tiny window in glibc where this setup can happen.
        self.run_standard_test(1, True)

class TestThrIter_stack_used_uninit_1(TestThrIter):
    STACK_USER = []
    STACK_USED = None

    def test_stack_used_uninit_1(self):
        """Test thr_iter with __stack_used uninitialized (1)"""
        # This should never happen in glibc (__stack_used is
        # initialized first) but we test it anyway.
        self.run_standard_test(1, True)

class TestThrIter_stack_used_uninit_2(TestThrIter):
    STACK_USER = [TestThread(TestThrIter.MAIN_PID)]
    STACK_USED = None

    def test_stack_used_uninit_2(self):
        """Test thr_iter with __stack_used uninitialized (2)"""
        # This should never happen in glibc (__stack_used is
        # initialized first) but we test it anyway.
        self.run_standard_test(1)

# Test with threads on both lists.

class TestThrIter_regular(TestThrIter):
    STACK_USER = [TestThread(TestThrIter.MAIN_PID),
                  TestThread(TestThrIter.MAIN_PID, SCHED_FIFO, 5),
                  TestThread(TestThrIter.MAIN_PID, SCHED_RR, -14),
                  TestThread(TestThrIter.MAIN_PID),
                  TestThread(TestThrIter.MAIN_PID + 1),
                  TestThread(TestThrIter.MAIN_PID + 2),
                  TestThread(TestThrIter.MAIN_PID)]
    STACK_USED = [TestThread(TestThrIter.MAIN_PID + 4, SCHED_FIFO, -3),
                  TestThread(TestThrIter.MAIN_PID),
                  TestThread(TestThrIter.MAIN_PID, SCHED_RR, -5),
                  TestThread(TestThrIter.MAIN_PID + 2, SCHED_RR, -3),
                  TestThread(TestThrIter.MAIN_PID + 1),
                  TestThread(TestThrIter.MAIN_PID),
                  TestThread(TestThrIter.MAIN_PID + 2),
                  TestThread(TestThrIter.MAIN_PID + 1),
                  TestThread(TestThrIter.MAIN_PID - 1),
                  # Threads which are about to fork.
                  TestThread(-TestThrIter.MAIN_PID),
                  TestThread(-TestThrIter.MAIN_PID, SCHED_FIFO, -3),
                  # Threads which are fork children.
                  TestThread(-(TestThrIter.MAIN_PID + 1),
                             tid=TestThrIter.MAIN_PID),
                  TestThread(-(TestThrIter.MAIN_PID + 2),
                             SCHED_RR, -5,
                             tid=TestThrIter.MAIN_PID),
                  TestThread(-(TestThrIter.MAIN_PID + 2),
                             tid=TestThrIter.MAIN_PID + 4),
    ]

    def test_thr_iter(self):
        """Test thr_iter with both lists initialized"""
        self.run_standard_test(12)

    def test_by_priority(self):
        """Test thr_iter priority filtering works."""
        counts = {}
        for list in self.STACK_USER, self.STACK_USED:
            for thread in list:
                if thread.matches(self):
                    priority = thread.priority
                    counts[priority] = counts.get(priority, 0) + 1

        pstart = TD_THR_LOWEST_PRIORITY
        plimit = pstart + 32 # POSIX
        self.assertLessEqual(pstart, min(counts.keys()))
        self.assertGreater(plimit, max(counts.keys()))

        nthreads = 0
        for priority in reversed(range(pstart, plimit)):
            nthreads += counts.get(priority, 0)
            self.TI_PRIORITY = priority
            self.run_standard_test(nthreads)
