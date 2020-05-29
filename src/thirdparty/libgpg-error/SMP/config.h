
/* Define to 1 if translation of program messages to the user's native
   language is requested. */
/* #undef ENABLE_NLS */

/* Define to use the GNU C visibility attribute. */
/* #undef GPGRT_USE_VISIBILITY */

/* Define to 1 if you have the Mac OS X function CFLocaleCopyCurrent in the
   CoreFoundation framework. */
/* #undef HAVE_CFLOCALECOPYCURRENT */

/* Define to 1 if you have the Mac OS X function CFPreferencesCopyAppValue in
   the CoreFoundation framework. */
/* #undef HAVE_CFPREFERENCESCOPYAPPVALUE */

/* Define if the GNU dcgettext() function is already present or preinstalled.
   */
/* #undef HAVE_DCGETTEXT */

/* Define to 1 if you have the declaration of `strerror_r', and to 0 if you
   don't. */
#define HAVE_DECL_STRERROR_R 0

/* Define to 1 if you have the <dlfcn.h> header file. */
/* #undef HAVE_DLFCN_H */

/* Define to 1 if you have the `flockfile' function. */
/* #undef HAVE_FLOCKFILE */

/* Defined if a GCC style "__attribute__ ((aligned (n))" is supported */
/* #undef HAVE_GCC_ATTRIBUTE_ALIGNED */

/* Define if the GNU gettext() function is already present or preinstalled. */
/* #undef HAVE_GETTEXT */

/* Define if you have the iconv() function and it works. */
#define HAVE_ICONV 1

/* Define to 1 if the system has the type `intmax_t'. */
#define HAVE_INTMAX_T 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define if you have <langinfo.h> and nl_langinfo(THOUSANDS_SEP). */
/* #undef HAVE_LANGINFO_THOUSANDS_SEP */

/* Define to 1 if you have the <locale.h> header file. */
#define HAVE_LOCALE_H 1

/* Define to 1 if the system has the type `long double'. */
#define HAVE_LONG_DOUBLE 1

/* Define to 1 if the system has the type 'long long int'. */
#define HAVE_LONG_LONG_INT 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

#if defined(ANDROID)
/* Define to 1 if you have the `memrchr' function. */
#define HAVE_MEMRCHR  1
#endif

/* Define if the <pthread.h> defines PTHREAD_MUTEX_RECURSIVE. */
/* #undef HAVE_PTHREAD_MUTEX_RECURSIVE */

/* Define if the POSIX multithreading library has read/write locks. */
/* #undef HAVE_PTHREAD_RWLOCK */

/* Define to 1 if the system has the type `ptrdiff_t'. */
#define HAVE_PTRDIFF_T 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strerror_r' function. */
/* #undef HAVE_STRERROR_R */

/* Define to 1 if you have the <strings.h> header file. */
/* #undef HAVE_STRINGS_H */

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/select.h> header file. */
/* #undef HAVE_SYS_SELECT_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
/* #undef HAVE_SYS_TIME_H */

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if the system has the type `uintmax_t'. */
#define HAVE_UINTMAX_T 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if the system has the type 'unsigned long long int'. */
#define HAVE_UNSIGNED_LONG_LONG_INT 1

/* Define to 1 if you have the `vasprintf' function. */
#define HAVE_VASPRINTF 1

/* Defined if we run on WindowsCE */
/* #undef HAVE_W32CE_SYSTEM */

#if defined(WIN32)
/* Defined if we run on a W32 API based system */
#define HAVE_W32_SYSTEM 1
#endif

/* Defined if we run on 64 bit W32 API system */
#if defined(__x86_64) || defined(_M_X64)
#define HAVE_W64_SYSTEM 1
#endif

/* Define to 1 if you have the <winsock2.h> header file. */
#define HAVE_WINSOCK2_H 1;

/* Name of package */
#define PACKAGE "libgpg-error"

/* Define to the full name of this package. */
#define PACKAGE_NAME "libgpg-error"

#include "version.h"

/* Define if the pthread_in_use() detection is hard. */
/* #undef PTHREAD_IN_USE_DETECTION_HARD */

/* Used by mkheader to insert the replacement type. */
#define REPLACEMENT_FOR_OFF_T "int64_t"

/* The size of `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG 4

/* The size of `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG 8

/* The size of `pthread_mutex_t', as computed by sizeof. */
/* #undef SIZEOF_PTHREAD_MUTEX_T */

/* The size of `unsigned long', as computed by sizeof. */
#define SIZEOF_UNSIGNED_LONG 4

/* The size of `void *', as computed by sizeof. */
#if defined(__x86_64) || defined(_M_X64)
#define SIZEOF_VOID_P 8
#else
#define SIZEOF_VOID_P 4
#endif

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if strerror_r returns char *. */
/* #undef STRERROR_R_CHAR_P */

#if !defined(WIN32)
/* Define if the POSIX multithreading library can be used. */
#define USE_POSIX_THREADS 1
#endif

#if !defined(WIN32)
/* Define if references to the POSIX multithreading library should be made
   weak. */
#define USE_POSIX_THREADS_WEAK 1
#endif

/* Define if the old Solaris multithreading library can be used. */
/* #undef USE_SOLARIS_THREADS */

/* Define if references to the old Solaris multithreading library should be
   made weak. */
/* #undef USE_SOLARIS_THREADS_WEAK */


/* Define if the native Windows multithreading API can be used. */
#define USE_WINDOWS_THREADS 1


/* Force using of NLS for W32 even if no libintl has been found.  This is
   okay because we have our own gettext implementation for W32.  */
#if defined(HAVE_W32_SYSTEM) && !defined(ENABLE_NLS)
#define ENABLE_NLS 1
#endif

/* Connect the generic estream-printf.c to our framework.  */
#define _ESTREAM_PRINTF_REALLOC _gpgrt_realloc
#define _ESTREAM_PRINTF_EXTRA_INCLUDE "gpgrt-int.h"

/* For building we need to define these macro.  */
#define GPG_ERR_ENABLE_GETTEXT_MACROS 1
#define GPG_ERR_ENABLE_ERRNO_MACROS 1
#define GPGRT_ENABLE_ES_MACROS 1

#if defined(_MSC_VER)
#include "compat.h"
#endif
