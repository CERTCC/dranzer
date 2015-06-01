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


#define FLG_SIGN 0x01
#define FLG_LEFT 0x02
#define FLG_ALT 0x04
#define FLG_ZERO 0x08
#define FLG_SPACE 0x10
#define FLG_SIGNED 0x20
#define FLG_CAPS 0x40

#include "fprintf_internal.h"

/* - vfprintf(FILE *file, const string_m fmt, int *count, va_list args)
 *  The vfprintf_m function formats its parameters args into a string according to the format 
 *  contained in the managed string fmt and outputs the result to file.
 *  If not a null pointer, *count is set to the number of characters transmitted.
 */

int _vfprintf_m(FILE *file, const string_m format, int *count, va_list args){
  errno_t rv;
  size_t total_width;
  va_list ap;
  int strtype = STRTYPE_NTBS;

  if(!file){
    ErrorHandler("vfprintf_m: 1st Argument NULL Pointer", file, EINVAL);
    ERROR(EINVAL);
  }

  if(!format){
    ErrorHandler("vfprintf_m: 2nd Argument NULL Pointer", format, EINVAL);
    ERROR(EINVAL);
  }

  VA_COPY( ap, args);
  if(format->strtype == STRTYPE_WSTR){
    if(format->str.wstr == NULL) return 0;
    rv = wcomputeLen(format, ap, &total_width, &strtype);
  } else if(format->strtype == STRTYPE_NTBS){
    if(format->str.cstr == NULL) return 0;
    rv = ccomputeLen(format, ap, &total_width, &strtype);
  } else {
    ErrorHandler("vfprintf_m: 2nd Argument Invalid String Type", format, EINVAL);
    ERROR(EINVAL);
  }


  va_end(ap);
  if(rv) return rv;

  // call outputString to write the string to file
  if(format->strtype == STRTYPE_NTBS)
    coutputString(file, format, count, args);
  else if(format->strtype == STRTYPE_WSTR)
    woutputString(file, format, count, args);

  return 0;
}

errno_t
vfprintf_m(FILE *f, const string_m fmt, int *count, va_list args){
  /*
    errno_t rv;

    char localbuf[BUFSIZ];

    if(f->_flag & _IONBF){
    f->_flag &= ~_IONBF;
    f->_ptr = f->_base = localbuf;
    f->_bufsiz = BUFSIZ;
    rv = _vfprintf_m(f, fmt, count, args);
    (void)fflush(f);
    f->_flag |= _IONBF;
    f->_base = NULL;
    f->_bufsiz = 0;
    f->_cnt = 0;
    } else
  */

  return _vfprintf_m(f,fmt, count, args);
}

/* variadic function helper macros */
/* "struct Qdmy" swallows the semicolon after VA_OPEN/VA_FIXEDARG's
   use without inhibiting further decls and without declaring an
   actual variable.  */
#define VA_OPEN(AP, VAR)	{ va_list AP; va_start(AP, VAR); { struct Qdmy
#define VA_CLOSE(AP)		} va_end(AP); }
#define VA_FIXEDARG(AP, T, N)	struct Qdmy

errno_t
fprintf_m(FILE *f, const string_m fmt, int *count, ...){
  int status;

  if (!fmt){
    ErrorHandler("sprintf_m: 2nd Argument NULL pointer", fmt, EINVAL);
    ERROR(EINVAL);
  }
  VA_OPEN(ap, count);
  VA_FIXEDARG(ap, FILE *, f);
  VA_FIXEDARG(ap, const string_m, fmt);
  VA_FIXEDARG(ap, int *, count);
  status = vfprintf_m(f, fmt, count, ap);
  VA_CLOSE(ap);
  return status;
}

