#ifndef _TIME_H
#include <time/time.h>

#ifndef _ISOMAC
# include <bits/types/locale_t.h>
# include <stdbool.h>

#include <endian.h>
#include <stdbool.h>

extern __typeof (strftime_l) __strftime_l;
libc_hidden_proto (__strftime_l)
extern __typeof (strptime_l) __strptime_l;

extern struct tm *__localtime64 (const __time64_t *__timer);

libc_hidden_proto (time)
libc_hidden_proto (asctime)
libc_hidden_proto (mktime)
libc_hidden_proto (timelocal)
libc_hidden_proto (localtime)
libc_hidden_proto (strftime)
libc_hidden_proto (strptime)

libc_hidden_proto (__localtime64)

/* Indicates whether the underlying kernel has 64-bit time support.
   This is required for e.g. librt, which cannot directly check the
   flag variable that init-first.c sets when detecting support. */
extern int __y2038_kernel_support (void);

#if BYTE_ORDER == BIG_ENDIAN
struct __timespec64
{
  __time64_t tv_sec;		/* Seconds */
  int tv_pad: 32;		/* Padding named for checking/setting */
  __syscall_slong_t tv_nsec;	/* Nanoseconds */
};
#else
struct __timespec64
{
  __time64_t tv_sec;		/* Seconds */
  __syscall_slong_t tv_nsec;	/* Nanoseconds */
  int tv_pad: 32;		/* Padding named for checking/setting */
};
#endif

struct __timeval64
{
  __time64_t tv_sec;		/* Seconds */
  __int64_t tv_usec;		/* Microseconds */
};

extern __typeof (clock_getres) __clock_getres;
extern __typeof (clock_gettime) __clock_gettime;
libc_hidden_proto (__clock_gettime)
extern __typeof (clock_settime) __clock_settime;
extern __typeof (clock_nanosleep) __clock_nanosleep;
extern __typeof (clock_getcpuclockid) __clock_getcpuclockid;

extern int __clock_gettime64 (clockid_t __clock_id,
			      struct __timespec64 *__tp) __THROW;
extern int __clock_settime64 (clockid_t __clock_id,
			       const struct __timespec64 *__tp) __THROW;
extern int __clock_getres64 (clockid_t __clock_id,
			      struct __timespec64 *__res) __THROW;
extern int __clock_nanosleep64 (clockid_t __clock_id, int __flags,
				const struct __timespec64 *__req,
				struct __timespec64 *__rem);

/* Now define the internal interfaces.  */
struct tm;

/* Defined in mktime.c.  */
extern const unsigned short int __mon_yday[2][13] attribute_hidden;

/* Defined in localtime.c.  */
extern struct tm _tmbuf attribute_hidden;

/* Defined in tzset.c.  */
extern char *__tzstring (const char *string) attribute_hidden;

extern int __use_tzfile attribute_hidden;

extern void __tzfile_read (const char *file, size_t extra,
			   char **extrap) attribute_hidden;
extern void __tzfile_compute (__time64_t timer, int use_localtime,
			      long int *leap_correct, int *leap_hit,
			      struct tm *tp) attribute_hidden;
extern void __tzfile_default (const char *std, const char *dst,
			      long int stdoff, long int dstoff)
  attribute_hidden;
extern void __tzset_parse_tz (const char *tz) attribute_hidden;
extern void __tz_compute (__time64_t timer, struct tm *tm, int use_localtime)
  __THROW attribute_hidden;

/* Subroutine of `mktime'.  Return the `time_t' representation of TP and
   normalize TP, given that a `struct tm *' maps to a `time_t' as performed
   by FUNC.  Keep track of next guess for time_t offset in *OFFSET.  */
extern time_t __mktime_internal (struct tm *__tp,
				 struct tm *(*__func) (const time_t *,
						       struct tm *),
				 time_t *__offset) attribute_hidden;

/* Subroutine of `__mktime64'.  Return the `__time64_t' representation of TP and
   normalize TP, given that a `struct tm *' maps to a `__time64_t' as performed
   by FUNC.  Keep track of next guess for __time64_t offset in *OFFSET.  */
extern __time64_t __mktime64_internal (struct tm *__tp,
				 struct tm *(*__func) (const __time64_t *,
						       struct tm *),
				 __time64_t *__offset) attribute_hidden;

extern struct tm *__localtime_r (const time_t *__timer,
				 struct tm *__tp) attribute_hidden;

extern struct tm *__localtime64_r (const __time64_t *__timer,
				   struct tm *__tp) attribute_hidden;

extern struct tm *__gmtime_r (const __time_t *__restrict __timer,
			      struct tm *__restrict __tp);
libc_hidden_proto (__gmtime_r)

extern struct tm *__gmtime64_r (const __time64_t *__restrict __timer,
			        struct tm *__restrict __tp);

/* Compute the `struct tm' representation of *T,
   offset OFFSET seconds east of UTC,
   and store year, yday, mon, mday, wday, hour, min, sec into *TP.
   Return nonzero if successful.  */
extern int __offtime (const __time_t *__timer,
		      long int __offset,
		      struct tm *__tp) attribute_hidden;

extern char *__asctime_r (const struct tm *__tp, char *__buf)
  attribute_hidden;
extern void __tzset (void) attribute_hidden;

extern int __offtime64 (const __time64_t *__timer,
		        long int __offset,
		        struct tm *__tp)
  attribute_hidden;

extern char *__asctime_r (const struct tm *__tp, char *__buf);
extern void __tzset (void);

/* Prototype for the internal function to get information based on TZ.  */
extern struct tm *__tz_convert (const __time_t *timer, int use_localtime,
			        struct tm *tp) attribute_hidden;

extern struct tm *__tz_convert64 (const __time64_t *timer,
				  int use_localtime, struct tm *tp)
  attribute_hidden;

extern int __nanosleep (const struct timespec *__requested_time,
			struct timespec *__remaining);
hidden_proto (__nanosleep)
extern int __getdate_r (const char *__string, struct tm *__resbufp)
  attribute_hidden;


/* Determine CLK_TCK value.  */
extern int __getclktck (void) attribute_hidden;


/* strptime support.  */
extern char * __strptime_internal (const char *rp, const char *fmt,
				   struct tm *tm, void *statep,
				   locale_t locparam) attribute_hidden;

extern double __difftime (time_t time1, time_t time0);

/* Use in the clock_* functions.  Size of the field representing the
   actual clock ID.  */
#define CLOCK_IDFIELD_SIZE	3

/* check whether a time64_t value fits in a time_t */
static inline bool
fits_in_time_t (__time64_t t)
{
  return t == (time_t) t;
}

/* convert a known valid struct timespec into a struct __timespec64 */
static inline void
valid_timespec_to_timespec64(const struct timespec *ts32,
			     struct __timespec64 *ts64)
{
  ts64->tv_sec = ts32->tv_sec;
  ts64->tv_nsec = ts32->tv_nsec;
  /* we only need to zero ts64->tv_pad if we pass it to the kernel */
}

/* convert a known valid struct timespec into a struct __timespec64 */
static inline void
valid_timespec64_to_timespec(const struct __timespec64 *ts64,
			     struct timespec *ts32)
{
  ts32->tv_sec = (time_t) ts64->tv_sec;
  ts32->tv_nsec = ts64->tv_nsec;
}

/* check if a value lies with the valid nanoseconds range */
#define IS_VALID_NANOSECONDS(ns) (ns >= 0 && ns <= 999999999)

/* check and convert a struct timespec into a struct __timespec64 */
static inline bool timespec_to_timespec64(const struct timespec *ts32,
					  struct __timespec64 *ts64)
{
  /* check that ts32 holds a valid count of nanoseconds */
  if (! IS_VALID_NANOSECONDS(ts32->tv_nsec))
    return false;
  /* all ts32 fields can fit in ts64, so copy them */
  valid_timespec_to_timespec64(ts32, ts64);
  /* we only need to zero ts64->tv_pad if we pass it to the kernel */
  return true;
}

/* check and convert a struct __timespec64 into a struct timespec */
static inline bool timespec64_to_timespec(const struct __timespec64 *ts64,
					  struct timespec *ts32)
{
  /* check that tv_nsec holds a valid count of nanoseconds */
  if (! IS_VALID_NANOSECONDS(ts64->tv_nsec))
    return false;
  /* check that tv_sec can fit in a __time_t */
  if (! fits_in_time_t(ts64->tv_sec))
    return false;
  /* all ts64 fields can fit in ts32, so copy them */
  valid_timespec64_to_timespec(ts64, ts32);
  return true;
}

/* convert a known valid struct timeval into a struct __timeval64 */
static inline void valid_timeval_to_timeval64(const struct timeval *tv32,
                                              struct __timeval64 *tv64)
{
  tv64->tv_sec = tv32->tv_sec;
  tv64->tv_usec = tv32->tv_usec;
}

/* convert a known valid struct timeval into a struct __timeval64 */
static inline void valid_timeval64_to_timeval(const struct __timeval64 *tv64,
					      struct timeval *tv32)
{
  tv32->tv_sec = (time_t) tv64->tv_sec;
  tv32->tv_usec = tv64->tv_usec;
}

/* check if a struct timeval/__timeval64 is valid */
#define IS_VALID_TIMEVAL(ts) \
  ((ts).tv_usec >= 0 && (ts).tv_usec <= 999999)

/* check if a struct timeval/__timeval64 is a valid 32-bit timeval */
#define IS_VALID_TIMEVAL32(ts) \
  (fits_in_time_t((ts).tv_sec) && (ts).tv_usec >= 0 && (ts).tv_usec <= 999999)

/* check and convert a struct timeval into a struct __timeval64 */
static inline bool timeval_to_timeval64(const struct timeval *tv32,
                                        struct __timeval64 *tv64)
{
  /* check that tv_usec holds a valid count of nanoseconds */
  if (! IS_VALID_TIMEVAL(*tv32))
    return false;
  /* all ts32 fields can fit in ts64, so copy them */
  valid_timeval_to_timeval64(tv32, tv64);
  /* we will only zero ts64->tv_pad if we pass it to the kernel */
  return true;
}

/* check and convert a struct __timeval64 into a struct timeval */
static inline bool timeval64_to_timeval(const struct __timeval64 *tv64,
                                        struct timeval *tv32)
{
  /* check that tv_usec holds a valid count of nanoseconds */
  if (! IS_VALID_TIMEVAL32(*tv64))
    return false;
  /* all ts64 fields can fit in ts32, so copy them */
  valid_timeval64_to_timeval(tv64, tv32);
  return true;
}

#endif
#endif
