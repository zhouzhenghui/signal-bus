/*
 * Copyright 2013, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

/**
 * @file
 * @ingroup continuation
 * @brief Replacement of inline keyword/specifier for legacy platforms.
 * @details The inline is defined appropriately when it is not supported or specified.
 */

/** @cond */
#if !(defined(inline) || defined(__cplusplus))
# if defined(__GNUC__) || defined(_MSC_VER) || defined(__INTEL_COMPILER) \
        || defined(__DMC__) || defined(__WATCOMC__) || defined(__DECC)
#   define inline __inline
# elif !defined(__MWERKS__) && __STDC_VERSION__ < 199901L
#   if defined(__STDC__)
#     define inline __inline
#   else
#     define inline
#   endif
# endif
#endif
/** @endcond */
