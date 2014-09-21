/*
 * Copyright 2014, Zhou Zhenghui <zhouzhenghui@gmail.com>
 */

#ifndef __CONTINUATION_COMPILER_MSVC_H
#define __CONTINUATION_COMPILER_MSVC_H

#pragma optimize("y", off)
#if _MSC_VER >= 1400 /* Visual Studio 2005 and later */
# pragma strict_gs_check(on)
#endif

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
// #pragma hdrstop

#if defined (_M_IX86)

struct __ExceptionRegistrationRecord
{
  struct __ExceptionRegistrationRecord *prev;
  FARPROC handler;
};

struct __ScopeTableEntry
{
  DWORD previousTryLevel;
  FARPROC lpfnFilter;
  FARPROC lpfnHandler;
};

struct __VCExceptionRegistrationRecord
{
  struct __ExceptionRegistrationRecord exception_registration;
  struct __ScopeTableEntry *scope_table;
  int trylevel;
  int _ebp;
};

# define CONTINUATION_CONSTRUCT(cont) \
  __try { \
    struct __VCExceptionRegistrationRecord *pVCExcRec; \
    { \
      __asm mov eax, FS:[0] \
      __asm mov [pVCExcRec], eax \
    } \
    (cont)->stack_frame_addr = (char *)pVCExcRec + sizeof(struct __VCExceptionRegistrationRecord); \
  } __except(EXCEPTION_EXECUTE_HANDLER) { \
  }

#endif /* defined (_M_IX86) */

#if CONTINUATION_USE_C99_VLA
# error "MSVC C99 VLA implementation is uncompatible"
#endif

#if !defined(CONTINUATION_USE_ALLOCA) && !defined(alloca)
# define alloca _alloca
# define CONTINUATION_USE_ALLOCA 1
#endif

#endif /* __CONTINUATION_COMPILER_MSVC_H */
