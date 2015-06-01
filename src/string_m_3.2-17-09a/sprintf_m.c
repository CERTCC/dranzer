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
#include <wctype.h>

#include "string_m.h"
#include "string_m_internals.h"


#define FLG_SIGN 0x01
#define FLG_LEFT 0x02
#define FLG_ALT 0x04
#define FLG_ZERO 0x08
#define FLG_SPACE 0x10
#define FLG_SIGNED 0x20
#define FLG_CAPS 0x40

typedef enum {
  NONE, h, hh, l, ll, L, q, j, z, t
} modifier_t;

#define THISTYPE STRTYPE_NTBS
#define PREFIX(A) c ## A
#define LETTER(A) A
#define STR(A) A
#define SFUNC(A) str ## A
#define CHECK(A) is ## A
#define TYPE char
#include "sprintf_internal.h"
#undef THISTYPE
#undef PREFIX
#undef LETTER
#undef STR
#undef SFUNC
#undef TYPE
#undef CHECK

#define THISTYPE STRTYPE_WSTR
#define PREFIX(A) w ## A
#define LETTER(A) L ## A
#define STR(A) L ## A
#define SFUNC(A) wcs ## A
#define CHECK(A) isw ## A
#define TYPE wchar_t
#include "sprintf_internal.h"


/*
 * vsprintf_m() - managed string version of C99 vsprintf() function.
 * ??? need to convert function so that string args are string_m
 */
errno_t vsprintf_m(string_m s, const string_m format, int *count, va_list args) {
#undef FUNC
#define FUNC vsprintf_m

  errno_t rv;
  size_t total_width = 0;
  va_list ap;
  int strtype = STRTYPE_NTBS;

  if (!s){
    ErrorHandler("vsprintf_m: 1st Argument NULL pointer", s, EINVAL);	
    ERROR(EINVAL);
  }
  if (!format){
    ErrorHandler("vsprintf_m: 2nd Argument NULL pointer", format, EINVAL);	
    ERROR(EINVAL);
  }

  VA_COPY( ap, args);

  if (format->strtype == STRTYPE_WSTR) {
    if (format->str.wstr == NULL) {
      return wstrcpy_m(s, NULL);
    }
    rv = wcomputeLen(format, ap, &total_width, &strtype);
  } else if (format->strtype == STRTYPE_NTBS) {
    if (format->str.cstr == NULL) {
      return cstrcpy_m(s, NULL);
    }
    rv = ccomputeLen(format, ap, &total_width, &strtype);
  } else {
    ErrorHandler("vsprintf_m: 2nd Argument Invalid String Type", format, EINVAL);	
    ERROR(EINVAL);
  }
  va_end(ap);
  if (rv) ERROR(rv);

  if (strtype != s->strtype) {
    if (s->strtype == STRTYPE_NTBS) {      
      rv = cstrcpy_m(s, NULL);
      if (rv) ERROR(rv);
    } else if (s->strtype == STRTYPE_WSTR) {
      rv = wstrcpy_m(s, NULL);
      if (rv) ERROR(rv);
      rv = str2wstr_m(s);
      if (rv) ERROR(rv);
    } else {
      ErrorHandler("vsprintf_m: 1st Argument Invalid String Type", s, EINVAL);	
      ERROR(EINVAL);
    }
  }

  if (total_width > s->size) {
    if (s->strtype == STRTYPE_NTBS) {
      REALLOC_C(s, total_width);
    } else if (s->strtype == STRTYPE_WSTR) {
      REALLOC_W(s, total_width);
    } else {
      ErrorHandler("vsprintf_m: 1st Argument Invalid String Type", s, EINVAL);
      ERROR(EINVAL);
    }
  }

  if (strtype == STRTYPE_NTBS) {
    rv = coutputString(s, format, args);
  } else if (strtype == STRTYPE_WSTR) {
    rv = woutputString(s, format, args);
  } else {	
    ERROR(EINVAL);
  }
  if (rv) {
    if (s->strtype == STRTYPE_NTBS) {
      if (s->str.cstr) {
	free(s->str.cstr);
	s->str.cstr= NULL;
	s->size = 0;
      }
    } else if (s->strtype == STRTYPE_WSTR) {
      if (s->str.wstr) {
	free(s->str.wstr);
	s->str.wstr= NULL;
	s->size = 0;
      }
    } else ERROR(EINVAL);
    ERROR(EINVAL);
  }

  rv = strlen_m(s, (size_t *)count);
  if (rv) ERROR(rv);

  return 0;
} // end vsprintf_m()



/* variadic function helper macros */
/* "struct Qdmy" swallows the semicolon after VA_OPEN/VA_FIXEDARG's
   use without inhibiting further decls and without declaring an
   actual variable.  */
#define VA_OPEN(AP, VAR)	{ va_list AP; va_start(AP, VAR); { struct Qdmy
#define VA_CLOSE(AP)		} va_end(AP); }
#define VA_FIXEDARG(AP, T, N)	struct Qdmy

int sprintf_m(string_m buf, const string_m fmt, int *count, ...) {
  int status;
  if (!buf){
    ErrorHandler("sprintf_m: 1st Argument NULL pointer", buf, EINVAL);
    ERROR(EINVAL);
  }
  if (!fmt){
    ErrorHandler("sprintf_m: 2nd Argument NULL pointer", fmt, EINVAL);
    ERROR(EINVAL);
  }
  VA_OPEN(ap, count);
  VA_FIXEDARG(ap, string_m, buf);
  VA_FIXEDARG(ap, const string_m, fmt);
  VA_FIXEDARG(ap, int *, count);
  status = vsprintf_m(buf, fmt, count, ap);
  VA_CLOSE(ap);
  return status;
}

