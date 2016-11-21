#ifndef ____mbstate_t_defined
#define ____mbstate_t_defined 1

/* Conversion state information.  */
typedef struct
{
  int __count;
  union
  {
# ifdef __WINT_TYPE__
    __WINT_TYPE__ __wch;
# else
    wint_t __wch;
# endif
    char __wchb[4];
  } __value;		/* Value so far.  */
} __mbstate_t;

#endif
