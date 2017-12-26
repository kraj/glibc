#ifndef _BITS_LIBIO_H
# include <libio/bits/libio.h>
# ifndef _ISOMAC

libc_hidden_proto (__woverflow)
libc_hidden_proto (__wunderflow)
libc_hidden_proto (__wuflow)
libc_hidden_proto (_IO_flockfile)
libc_hidden_proto (_IO_free_backup_area)
libc_hidden_proto (_IO_free_wbackup_area)
libc_hidden_proto (_IO_ftrylockfile)
libc_hidden_proto (_IO_funlockfile)
libc_hidden_proto (_IO_padn)
libc_hidden_proto (_IO_putc)
libc_hidden_proto (_IO_sgetn)
libc_hidden_proto (_IO_vfprintf)
libc_hidden_proto (_IO_vfscanf)

# endif
#endif
