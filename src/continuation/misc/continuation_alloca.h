/*
 * Copyright 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __CONTINUATION_ALLOCA_H
#define __CONTINUATION_ALLOCA_H

/* user defined alloca */
#if defined(alloca)
# if !defined(HAVE_ALLOCA)
#   define HAVE_ALLOCA 1
# endif
#elif defined(__GNUC__)
/* When using gcc, make sure to use its builtin alloca. */
# define alloca __builtin_alloca
# if !defined(HAVE_ALLOCA)
#   define HAVE_ALLOCA 1
# endif
#elif defined(_MSC_VER)
/* msvc */
# include <malloc.h>
# define alloca _alloca
# if !defined(HAVE_ALLOCA)
#   define HAVE_ALLOCA 1
# endif
#elif defined(_IBMR2) || defined(_AIX)
/* IBM's compilers need a #pragma
in "each module that needs to use alloca". */
# pragma alloca
# if !defined(HAVE_ALLOCA)
#   define HAVE_ALLOCA 1
# endif
#elif defined(__DECC)
# define alloca(x) __ALLOCA(x)
# if !defined(HAVE_ALLOCA)
#   define HAVE_ALLOCA 1
# endif
#endif /* !defined(alloca) */

/* When using cc, do whatever necessary to allow use of alloca. For many
machines, this means including alloca.h. */
#if !defined(HAVE_ALLOCA_H)
/* We need lots of variants for MIPS, to cover all versions and perversions
of OSes for MIPS. */
# if defined (__mips) || defined (MIPSEL) || defined (MIPSEB) \
    || defined (_MIPSEL) || defined (_MIPSEB) || defined (__sgi) \
    || defined (__alpha) || defined (__sparc) || defined (sparc) \
    || defined (__ksr__)
#   define HAVE_ALLOCA_H
# endif
#elif defined (sparc) && defined (sun)
# define HAVE_ALLOCA_H
#endif /* !defined(HAVE_ALLOCA_H) */

/* include header file if needed */
#if defined(HAVE_ALLOCA_H)
# if !defined(HAVE_ALLOCA)
#   define HAVE_ALLOCA 1
# endif

# if defined (IBMESA)
#   include <malloc.h>
# else /** !IBMESA */
#   include <alloca.h>
# endif /** !IBMESA */

#endif /** HAVE_ALLOCA_H */

#endif /* __CONTINUATION_ALLOCA_H */
