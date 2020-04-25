#ifndef ERR_MAP
#define ERR_MAP(value) value
#endif
_S(ERR_MAP(0), N_("Success"))
#ifdef EPERM
_S(ERR_MAP(EPERM), N_("Operation not permitted"))
#endif
#ifdef ENOENT
_S(ERR_MAP(ENOENT), N_("No such file or directory"))
#endif
#ifdef ESRCH
_S(ERR_MAP(ESRCH), N_("No such process"))
#endif
#ifdef EINTR
_S(ERR_MAP(EINTR), N_("Interrupted system call"))
#endif
#ifdef EIO
_S(ERR_MAP(EIO), N_("Input/output error"))
#endif
#ifdef ENXIO
_S(ERR_MAP(ENXIO), N_("No such device or address"))
#endif
#ifdef E2BIG
_S(ERR_MAP(E2BIG), N_("Argument list too long"))
#endif
#ifdef ENOEXEC
_S(ERR_MAP(ENOEXEC), N_("Exec format error"))
#endif
#ifdef EBADF
_S(ERR_MAP(EBADF), N_("Bad file descriptor"))
#endif
#ifdef ECHILD
_S(ERR_MAP(ECHILD), N_("No child processes"))
#endif
#ifdef EDEADLK
_S(ERR_MAP(EDEADLK), N_("Resource deadlock avoided"))
#endif
#ifdef ENOMEM
_S(ERR_MAP(ENOMEM), N_("Cannot allocate memory"))
#endif
#ifdef EACCES
_S(ERR_MAP(EACCES), N_("Permission denied"))
#endif
#ifdef EFAULT
_S(ERR_MAP(EFAULT), N_("Bad address"))
#endif
#ifdef ENOTBLK
_S(ERR_MAP(ENOTBLK), N_("Block device required"))
#endif
#ifdef EBUSY
_S(ERR_MAP(EBUSY), N_("Device or resource busy"))
#endif
#ifdef EEXIST
_S(ERR_MAP(EEXIST), N_("File exists"))
#endif
#ifdef EXDEV
_S(ERR_MAP(EXDEV), N_("Invalid cross-device link"))
#endif
#ifdef ENODEV
_S(ERR_MAP(ENODEV), N_("No such device"))
#endif
#ifdef ENOTDIR
_S(ERR_MAP(ENOTDIR), N_("Not a directory"))
#endif
#ifdef EISDIR
_S(ERR_MAP(EISDIR), N_("Is a directory"))
#endif
#ifdef EINVAL
_S(ERR_MAP(EINVAL), N_("Invalid argument"))
#endif
#ifdef EMFILE
_S(ERR_MAP(EMFILE), N_("Too many open files"))
#endif
#ifdef ENFILE
_S(ERR_MAP(ENFILE), N_("Too many open files in system"))
#endif
#ifdef ENOTTY
_S(ERR_MAP(ENOTTY), N_("Inappropriate ioctl for device"))
#endif
#ifdef ETXTBSY
_S(ERR_MAP(ETXTBSY), N_("Text file busy"))
#endif
#ifdef EFBIG
_S(ERR_MAP(EFBIG), N_("File too large"))
#endif
#ifdef ENOSPC
_S(ERR_MAP(ENOSPC), N_("No space left on device"))
#endif
#ifdef ESPIPE
_S(ERR_MAP(ESPIPE), N_("Illegal seek"))
#endif
#ifdef EROFS
_S(ERR_MAP(EROFS), N_("Read-only file system"))
#endif
#ifdef EMLINK
_S(ERR_MAP(EMLINK), N_("Too many links"))
#endif
#ifdef EPIPE
_S(ERR_MAP(EPIPE), N_("Broken pipe"))
#endif
#ifdef EDOM
_S(ERR_MAP(EDOM), N_("Numerical argument out of domain"))
#endif
#ifdef ERANGE
_S(ERR_MAP(ERANGE), N_("Numerical result out of range"))
#endif
#ifdef EAGAIN
_S(ERR_MAP(EAGAIN), N_("Resource temporarily unavailable"))
#endif
#ifdef EINPROGRESS
_S(ERR_MAP(EINPROGRESS), N_("Operation now in progress"))
#endif
#ifdef EALREADY
_S(ERR_MAP(EALREADY), N_("Operation already in progress"))
#endif
#ifdef ENOTSOCK
_S(ERR_MAP(ENOTSOCK), N_("Socket operation on non-socket"))
#endif
#ifdef EMSGSIZE
_S(ERR_MAP(EMSGSIZE), N_("Message too long"))
#endif
#ifdef EPROTOTYPE
_S(ERR_MAP(EPROTOTYPE), N_("Protocol wrong type for socket"))
#endif
#ifdef ENOPROTOOPT
_S(ERR_MAP(ENOPROTOOPT), N_("Protocol not available"))
#endif
#ifdef EPROTONOSUPPORT
_S(ERR_MAP(EPROTONOSUPPORT), N_("Protocol not supported"))
#endif
#ifdef ESOCKTNOSUPPORT
_S(ERR_MAP(ESOCKTNOSUPPORT), N_("Socket type not supported"))
#endif
#ifdef EOPNOTSUPP
_S(ERR_MAP(EOPNOTSUPP), N_("Operation not supported"))
#endif
#ifdef EPFNOSUPPORT
_S(ERR_MAP(EPFNOSUPPORT), N_("Protocol family not supported"))
#endif
#ifdef EAFNOSUPPORT
_S(ERR_MAP(EAFNOSUPPORT), N_("Address family not supported by protocol"))
#endif
#ifdef EADDRINUSE
_S(ERR_MAP(EADDRINUSE), N_("Address already in use"))
#endif
#ifdef EADDRNOTAVAIL
_S(ERR_MAP(EADDRNOTAVAIL), N_("Cannot assign requested address"))
#endif
#ifdef ENETDOWN
_S(ERR_MAP(ENETDOWN), N_("Network is down"))
#endif
#ifdef ENETUNREACH
_S(ERR_MAP(ENETUNREACH), N_("Network is unreachable"))
#endif
#ifdef ENETRESET
_S(ERR_MAP(ENETRESET), N_("Network dropped connection on reset"))
#endif
#ifdef ECONNABORTED
_S(ERR_MAP(ECONNABORTED), N_("Software caused connection abort"))
#endif
#ifdef ECONNRESET
_S(ERR_MAP(ECONNRESET), N_("Connection reset by peer"))
#endif
#ifdef ENOBUFS
_S(ERR_MAP(ENOBUFS), N_("No buffer space available"))
#endif
#ifdef EISCONN
_S(ERR_MAP(EISCONN), N_("Transport endpoint is already connected"))
#endif
#ifdef ENOTCONN
_S(ERR_MAP(ENOTCONN), N_("Transport endpoint is not connected"))
#endif
#ifdef EDESTADDRREQ
_S(ERR_MAP(EDESTADDRREQ), N_("Destination address required"))
#endif
#ifdef ESHUTDOWN
_S(ERR_MAP(ESHUTDOWN), N_("Cannot send after transport endpoint shutdown"))
#endif
#ifdef ETOOMANYREFS
_S(ERR_MAP(ETOOMANYREFS), N_("Too many references: cannot splice"))
#endif
#ifdef ETIMEDOUT
_S(ERR_MAP(ETIMEDOUT), N_("Connection timed out"))
#endif
#ifdef ECONNREFUSED
_S(ERR_MAP(ECONNREFUSED), N_("Connection refused"))
#endif
#ifdef ELOOP
_S(ERR_MAP(ELOOP), N_("Too many levels of symbolic links"))
#endif
#ifdef ENAMETOOLONG
_S(ERR_MAP(ENAMETOOLONG), N_("File name too long"))
#endif
#ifdef EHOSTDOWN
_S(ERR_MAP(EHOSTDOWN), N_("Host is down"))
#endif
#ifdef EHOSTUNREACH
_S(ERR_MAP(EHOSTUNREACH), N_("No route to host"))
#endif
#ifdef ENOTEMPTY
_S(ERR_MAP(ENOTEMPTY), N_("Directory not empty"))
#endif
#ifdef EUSERS
_S(ERR_MAP(EUSERS), N_("Too many users"))
#endif
#ifdef EDQUOT
_S(ERR_MAP(EDQUOT), N_("Disk quota exceeded"))
#endif
#ifdef ESTALE
_S(ERR_MAP(ESTALE), N_("Stale file handle"))
#endif
#ifdef EREMOTE
_S(ERR_MAP(EREMOTE), N_("Object is remote"))
#endif
#ifdef ENOLCK
_S(ERR_MAP(ENOLCK), N_("No locks available"))
#endif
#ifdef ENOSYS
_S(ERR_MAP(ENOSYS), N_("Function not implemented"))
#endif
#ifdef EILSEQ
_S(ERR_MAP(EILSEQ), N_("Invalid or incomplete multibyte or wide character"))
#endif
#ifdef EBADMSG
_S(ERR_MAP(EBADMSG), N_("Bad message"))
#endif
#ifdef EIDRM
_S(ERR_MAP(EIDRM), N_("Identifier removed"))
#endif
#ifdef EMULTIHOP
_S(ERR_MAP(EMULTIHOP), N_("Multihop attempted"))
#endif
#ifdef ENODATA
_S(ERR_MAP(ENODATA), N_("No data available"))
#endif
#ifdef ENOLINK
_S(ERR_MAP(ENOLINK), N_("Link has been severed"))
#endif
#ifdef ENOMSG
_S(ERR_MAP(ENOMSG), N_("No message of desired type"))
#endif
#ifdef ENOSR
_S(ERR_MAP(ENOSR), N_("Out of streams resources"))
#endif
#ifdef ENOSTR
_S(ERR_MAP(ENOSTR), N_("Device not a stream"))
#endif
#ifdef EOVERFLOW
_S(ERR_MAP(EOVERFLOW), N_("Value too large for defined data type"))
#endif
#ifdef EPROTO
_S(ERR_MAP(EPROTO), N_("Protocol error"))
#endif
#ifdef ETIME
_S(ERR_MAP(ETIME), N_("Timer expired"))
#endif
#ifdef ECANCELED
_S(ERR_MAP(ECANCELED), N_("Operation canceled"))
#endif
#ifdef EOWNERDEAD
_S(ERR_MAP(EOWNERDEAD), N_("Owner died"))
#endif
#ifdef ENOTRECOVERABLE
_S(ERR_MAP(ENOTRECOVERABLE), N_("State not recoverable"))
#endif
#ifdef ERESTART
_S(ERR_MAP(ERESTART), N_("Interrupted system call should be restarted"))
#endif
#ifdef ECHRNG
_S(ERR_MAP(ECHRNG), N_("Channel number out of range"))
#endif
#ifdef EL2NSYNC
_S(ERR_MAP(EL2NSYNC), N_("Level 2 not synchronized"))
#endif
#ifdef EL3HLT
_S(ERR_MAP(EL3HLT), N_("Level 3 halted"))
#endif
#ifdef EL3RST
_S(ERR_MAP(EL3RST), N_("Level 3 reset"))
#endif
#ifdef ELNRNG
_S(ERR_MAP(ELNRNG), N_("Link number out of range"))
#endif
#ifdef EUNATCH
_S(ERR_MAP(EUNATCH), N_("Protocol driver not attached"))
#endif
#ifdef ENOCSI
_S(ERR_MAP(ENOCSI), N_("No CSI structure available"))
#endif
#ifdef EL2HLT
_S(ERR_MAP(EL2HLT), N_("Level 2 halted"))
#endif
#ifdef EBADE
_S(ERR_MAP(EBADE), N_("Invalid exchange"))
#endif
#ifdef EBADR
_S(ERR_MAP(EBADR), N_("Invalid request descriptor"))
#endif
#ifdef EXFULL
_S(ERR_MAP(EXFULL), N_("Exchange full"))
#endif
#ifdef ENOANO
_S(ERR_MAP(ENOANO), N_("No anode"))
#endif
#ifdef EBADRQC
_S(ERR_MAP(EBADRQC), N_("Invalid request code"))
#endif
#ifdef EBADSLT
_S(ERR_MAP(EBADSLT), N_("Invalid slot"))
#endif
#ifdef EBFONT
_S(ERR_MAP(EBFONT), N_("Bad font file format"))
#endif
#ifdef ENONET
_S(ERR_MAP(ENONET), N_("Machine is not on the network"))
#endif
#ifdef ENOPKG
_S(ERR_MAP(ENOPKG), N_("Package not installed"))
#endif
#ifdef EADV
_S(ERR_MAP(EADV), N_("Advertise error"))
#endif
#ifdef ESRMNT
_S(ERR_MAP(ESRMNT), N_("Srmount error"))
#endif
#ifdef ECOMM
_S(ERR_MAP(ECOMM), N_("Communication error on send"))
#endif
#ifdef EDOTDOT
_S(ERR_MAP(EDOTDOT), N_("RFS specific error"))
#endif
#ifdef ENOTUNIQ
_S(ERR_MAP(ENOTUNIQ), N_("Name not unique on network"))
#endif
#ifdef EBADFD
_S(ERR_MAP(EBADFD), N_("File descriptor in bad state"))
#endif
#ifdef EREMCHG
_S(ERR_MAP(EREMCHG), N_("Remote address changed"))
#endif
#ifdef ELIBACC
_S(ERR_MAP(ELIBACC), N_("Can not access a needed shared library"))
#endif
#ifdef ELIBBAD
_S(ERR_MAP(ELIBBAD), N_("Accessing a corrupted shared library"))
#endif
#ifdef ELIBSCN
_S(ERR_MAP(ELIBSCN), N_(".lib section in a.out corrupted"))
#endif
#ifdef ELIBMAX
_S(ERR_MAP(ELIBMAX), N_("Attempting to link in too many shared libraries"))
#endif
#ifdef ELIBEXEC
_S(ERR_MAP(ELIBEXEC), N_("Cannot exec a shared library directly"))
#endif
#ifdef ESTRPIPE
_S(ERR_MAP(ESTRPIPE), N_("Streams pipe error"))
#endif
#ifdef EUCLEAN
_S(ERR_MAP(EUCLEAN), N_("Structure needs cleaning"))
#endif
#ifdef ENOTNAM
_S(ERR_MAP(ENOTNAM), N_("Not a XENIX named type file"))
#endif
#ifdef ENAVAIL
_S(ERR_MAP(ENAVAIL), N_("No XENIX semaphores available"))
#endif
#ifdef EISNAM
_S(ERR_MAP(EISNAM), N_("Is a named type file"))
#endif
#ifdef EREMOTEIO
_S(ERR_MAP(EREMOTEIO), N_("Remote I/O error"))
#endif
#ifdef ENOMEDIUM
_S(ERR_MAP(ENOMEDIUM), N_("No medium found"))
#endif
#ifdef EMEDIUMTYPE
_S(ERR_MAP(EMEDIUMTYPE), N_("Wrong medium type"))
#endif
#ifdef ENOKEY
_S(ERR_MAP(ENOKEY), N_("Required key not available"))
#endif
#ifdef EKEYEXPIRED
_S(ERR_MAP(EKEYEXPIRED), N_("Key has expired"))
#endif
#ifdef EKEYREVOKED
_S(ERR_MAP(EKEYREVOKED), N_("Key has been revoked"))
#endif
#ifdef EKEYREJECTED
_S(ERR_MAP(EKEYREJECTED), N_("Key was rejected by service"))
#endif
#ifdef ERFKILL
_S(ERR_MAP(ERFKILL), N_("Operation not possible due to RF-kill"))
#endif
#ifdef EHWPOISON
_S(ERR_MAP(EHWPOISON), N_("Memory page has hardware error"))
#endif
#ifdef EBADRPC
_S(ERR_MAP(EBADRPC), N_("RPC struct is bad"))
#endif
#ifdef EFTYPE
_S(ERR_MAP(EFTYPE), N_("Inappropriate file type or format"))
#endif
#ifdef EPROCUNAVAIL
_S(ERR_MAP(EPROCUNAVAIL), N_("RPC bad procedure for program"))
#endif
#ifdef EAUTH
_S(ERR_MAP(EAUTH), N_("Authentication error"))
#endif
#ifdef EDIED
_S(ERR_MAP(EDIED), N_("Translator died"))
#endif
#ifdef ERPCMISMATCH
_S(ERR_MAP(ERPCMISMATCH), N_("RPC version wrong"))
#endif
#ifdef EGREGIOUS
_S(ERR_MAP(EGREGIOUS), N_("You really blew it this time"))
#endif
#ifdef EPROCLIM
_S(ERR_MAP(EPROCLIM), N_("Too many processes"))
#endif
#ifdef EGRATUITOUS
_S(ERR_MAP(EGRATUITOUS), N_("Gratuitous error"))
#endif
#if defined (ENOTSUP) && ENOTSUP != EOPNOTSUPP
_S(ERR_MAP(ENOTSUP), N_("Not supported"))
#endif
#ifdef EPROGMISMATCH
_S(ERR_MAP(EPROGMISMATCH), N_("RPC program version wrong"))
#endif
#ifdef EBACKGROUND
_S(ERR_MAP(EBACKGROUND), N_("Inappropriate operation for background process"))
#endif
#ifdef EIEIO
_S(ERR_MAP(EIEIO), N_("Computer bought the farm"))
#endif
#if defined (EWOULDBLOCK) && EWOULDBLOCK != EAGAIN
_S(ERR_MAP(EWOULDBLOCK), N_("Operation would block"))
#endif
#ifdef ENEEDAUTH
_S(ERR_MAP(ENEEDAUTH), N_("Need authenticator"))
#endif
#ifdef ED
_S(ERR_MAP(ED), N_("?"))
#endif
#ifdef EPROGUNAVAIL
_S(ERR_MAP(EPROGUNAVAIL), N_("RPC program not available"))
#endif
