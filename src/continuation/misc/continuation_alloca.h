/*
 * Copyright 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __CONTINUATION_ALLOCA_H
#define __CONTINUATION_ALLOCA_H

/* IBM's compilers need a #pragma
in "each module that needs to use alloca". */
# if defined(_IBMR2)
#  pragma alloca
#  if !defined(HAVE_ALLOCA)
#    define HAVE_ALLOCA 1
#  endif
# endif

/* user defined alloca */
#if defined(alloca) && !defined(HAVE_ALLOCA)
# define HAVE_ALLOCA 1
#endif

/* buildin alloca */
#if !defined(HAVE_ALLOCA)
/* When using gcc, make sure to use its builtin alloca. */
# if defined(__GNUC__)
#   define alloca __builtin_alloca
#   define HAVE_ALLOCA 1
# endif

# if defined(__DECC)
#   define alloca(x) __ALLOCA(x)
#   define HAVE_ALLOCA 1
# endif
#endif

/* When using cc, do whatever necessary to allow use of alloca. For many
machines, this means including alloca.h. */
#if !defined(HAVE_ALLOCA) && !defined(HAVE_ALLOCA_H)
/* We need lots of variants for MIPS, to cover all versions and perversions
of OSes for MIPS. */
# if defined (__mips) || defined (MIPSEL) || defined (MIPSEB) \
    || defined (_MIPSEL) || defined (_MIPSEB) || defined (__sgi) \
    || defined (__alpha) || defined (__sparc) || defined (sparc) \
    || defined (__ksr__)
#   define HAVE_ALLOCA_H
# endif

# if defined (sparc) && defined (sun)
#   define HAVE_ALLOCA_H
# endif

#endif /* !HAVE_ALLOCA_H && !HAVE_ALLOCA_H */

/* include header file if needed */
#if !defined(alloca) && defined(HAVE_ALLOCA_H)

# if !defined(HAVE_ALLOCA)
#   define HAVE_ALLOCA 1
# endif

# if defined (IBMESA)
#   include <malloc.h>
# else /** !IBMESA */
#   include <alloca.h>
# endif /** !IBMESA */

#endif /** !alloca */

#endif /* __CONTINUATION_ALLOCA_H */
