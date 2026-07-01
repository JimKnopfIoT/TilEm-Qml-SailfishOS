/* config.h - Generated for SailfishOS build */

#ifndef CONFIG_H
#define CONFIG_H

/* Package version */
#define PACKAGE_VERSION "2.0.0"

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the __sync_synchronize function. */
#define HAVE___SYNC_SYNCHRONIZE 1

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel).
   ARM64/x86 are little-endian. NOTE: tilem.h uses #ifdef WORDS_BIGENDIAN, so the
   macro must NOT be defined at all on little-endian (defining it to 0 still makes
   #ifdef true → wrong big-endian register union → scrambled Z80 byte registers). */
#undef WORDS_BIGENDIAN

/* Define if you have the lround function */
#define HAVE_LROUND 1

/* uintptr_t type - use system definition from stdint.h if available */
#ifndef HAVE_STDINT_H
#define uintptr_t unsigned long
#endif

/* C99 'restrict' keyword is not part of C++ - make it empty for C++ builds */
#ifdef __cplusplus
#ifndef restrict
#define restrict
#endif
#endif

#endif /* CONFIG_H */
