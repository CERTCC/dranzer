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
#include <string.h>

#include "string_m.h"
#include "string_m_internals.h"

// strlen_m
//   Inputs
//      s - managed string
//   Outputs
//      size - length of input string
//   Return
//      status - 0=OK, -1=error
errno_t strlen_m(const string_m s, size_t *size) {
  register size_t n;

  if (!size) ERROR(EINVAL);
  *size = 0;

  //validate s
  if (!s){
    ErrorHandler("strlen_m: 1st Argument NULL Pointer", s, EINVAL);	
    ERROR(EINVAL);
  }

  if (s->strtype == STRTYPE_WSTR) {
    wchar_t *lp;

    if (!s->str.wstr) { // Null string has length 0
      *size = 0;
      return 0;
    }
    n = 0;
    for (lp = s->str.wstr; n < s->size && *lp; lp++, n++)
      ;
    if (n >= s->size) ERROR(EINVAL);

    *size=n;
  } else if (s->strtype == STRTYPE_NTBS) {
    char *lp;

    if (!s->str.cstr) { // Null string has length 0
      *size = 0;
      return 0;
    }
    n = 0;
    for (lp = s->str.cstr; n < s->size && *lp; lp++, n++)
      ;
    if (n >= s->size) ERROR(EINVAL);

    *size=n;
  } else{
    ErrorHandler("strlen_m: 1st Argument Invalid String Type", s, EINVAL);	
    ERROR(EINVAL);
  }
  return 0;
} // end strlen_m
