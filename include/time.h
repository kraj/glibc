#ifndef _TIME_H
#include <time/time.h>

#ifndef _ISOMAC
# include <xlocale.h>

#include <endian.h>

__BEGIN_DECLS

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

struct __itimerspec64
{
  struct __timespec64 it_interval;
  struct __timespec64 it_value;
};

extern __typeof (clock_getres) __clock_getres;
extern __typeof (clock_gettime) __clock_gettime;
extern int __clock_getres64 (clockid_t __clock_id, struct __timespec64 *__res) __THROW;
libc_hidden_proto (__clock_gettime)
extern int __clock_gettime64 (clockid_t __clock_id, struct __timespec64 *__tp) __THROW;
extern __typeof (clock_settime) __clock_settime;
extern int __clock_settime64 (clockid_t __clock_id, const struct __timespec64 *__tp) __THROW;
extern __typeof (clock_nanosleep) __clock_nanosleep;
extern int __clock_nanosleep64 (clockid_t __clock_id, int __flags,
			    const struct __timespec64 *__req,
			    struct __timespec64 *__rem);
extern __typeof (clock_getcpuclockid) __clock_getcpuclockid;

/* Now define the internal interfaces.  */
struct tm;

/* Defined in mktime.c.  */
extern const unsigned short int __mon_yday[2][13] attribute_hidden;

/* Defined in localtime.c.  */
extern struct tm _tmbuf attribute_hidden;

/* Defined in tzset.c.  */
extern char *__tzstring (const char *string);

/* Defined in tzset.c. */
extern size_t __tzname_cur_max attribute_hidden;


extern int __use_tzfile attribute_hidden;

extern void __tzfile_read (const char *file, size_t extra,
			   char **extrap);
extern void __tzfile_compute (__time64_t timer, int use_localtime,
			      long int *leap_correct, int *leap_hit,
			      struct tm *tp);
extern void __tzfile_default (const char *std, const char *dst,
			      long int stdoff, long int dstoff);
extern void __tzset_parse_tz (const char *tz);
extern void __tz_compute (__time64_t timer, struct tm *tm, int use_localtime)
     __THROW internal_function;

/* Subroutine of `mktime'.  Return the `time_t' representation of TP and
   normalize TP, given that a `struct tm *' maps to a `time_t' as performed
   by FUNC.  Keep track of next guess for time_t offset in *OFFSET.  */
extern time_t __mktime_internal (struct tm *__tp,
				 struct tm *(*__func) (const time_t *,
						       struct tm *),
				 time_t *__offset);

/* Subroutine of `__mktime64'.  Return the `__time64_t' representation of TP and
   normalize TP, given that a `struct tm *' maps to a `__time64_t' as performed
   by FUNC.  Keep track of next guess for __time64_t offset in *OFFSET.  */
extern __time64_t __mktime64_internal (struct tm *__tp,
				 struct tm *(*__func) (const __time64_t *,
						       struct tm *),
				 __time64_t *__offset);

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
		      struct tm *__tp);

extern int __offtime64 (const __time64_t *__timer,
		      long int __offset,
		      struct tm *__tp);

extern char *__asctime_r (const struct tm *__tp, char *__buf);
extern void __tzset (void);

/* Prototype for the internal function to get information based on TZ.  */
extern struct tm *__tz_convert (const __time_t *timer, int use_localtime, struct tm *tp);

extern struct tm *__tz_convert64 (const __time64_t *timer, int use_localtime, struct tm *tp);

/* Return the maximum length of a timezone name.
   This is what `sysconf (_SC_TZNAME_MAX)' does.  */
extern long int __tzname_max (void);

extern int __nanosleep (const struct timespec *__requested_time,
			struct timespec *__remaining);
libc_hidden_proto (__nanosleep)
extern int __nanosleep_nocancel (const struct timespec *__requested_time,
				 struct timespec *__remaining)
  attribute_hidden;
extern int __getdate_r (const char *__string, struct tm *__resbufp);


/* Determine CLK_TCK value.  */
extern int __getclktck (void);


/* strptime support.  */
extern char * __strptime_internal (const char *rp, const char *fmt,
				   struct tm *tm, void *statep,
				   __locale_t locparam)
     internal_function;

extern double __difftime (time_t time1, time_t time0);


/* Use in the clock_* functions.  Size of the field representing the
   actual clock ID.  */
#define CLOCK_IDFIELD_SIZE	3

__END_DECLS

#endif
#endif
