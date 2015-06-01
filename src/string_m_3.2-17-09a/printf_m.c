/*
 *
 * Copyright (c) 2005 Carnegie Mellon University.
 * All rights reserved.

 * Permission to use this software and its documentation for any purpose is hereby granted, 
 * provided that the above copyright notice appear and that both that copyright notice and 
 * this permission notice appear in supporting documentation, and that the name of CMU not 
 * be used in advertising or publicity pertaining to distribution of the software without 
 * specific, written prior permission.
 * 
 * CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES 
 * OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, 
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, RISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <string.h>
#include <wchar.h>

#include "string_m.h"
#include "string_m_internals.h"

errno_t
vprintf_m(const string_m fmt, int *count, va_list args){
  if(!fmt){
    ErrorHandler("vprintf_m: 1st Argument NULL Pointer", fmt, EINVAL);
    ERROR(EINVAL);
  }

  return vfprintf_m(stdout, fmt, count, args);
}

/* variadic function helper macros */
/* "struct Qdmy" swallows the semicolon after VA_OPEN/VA_FIXEDARG's
   use without inhibiting further decls and without declaring an
   actual variable.  */
#define VA_OPEN(AP, VAR)	{ va_list AP; va_start(AP, VAR); { struct Qdmy
#define VA_CLOSE(AP)		} va_end(AP); }
#define VA_FIXEDARG(AP, T, N)	struct Qdmy

errno_t
printf_m(const string_m fmt, int *count, ...){
  errno_t rv;

  if (!fmt){
    ErrorHandler("printf_m: 1st Argument NULL pointer", fmt, EINVAL);
    ERROR(EINVAL);
  }
  VA_OPEN(ap, count);
  VA_FIXEDARG(ap, const string_m, fmt);
  VA_FIXEDARG(ap, int *, count);
  rv = vprintf_m(fmt, count, ap);
  VA_CLOSE(ap);
  return rv;
}
