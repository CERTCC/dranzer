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
#include "stdbool.h"

#include "string_m.h"
#include "string_m_internals.h"

// isnull_m
errno_t
isnull_m(const string_m s, _Bool *nullstr) {
  if (!nullstr){
    ErrorHandler("isnull_m: 2nd Argument NULL Pointer", nullstr, EINVAL);	
    ERROR(EINVAL);
  }
  *nullstr = false;

  // validate s
  if (s == NULL){
    ErrorHandler("isnull_m: 1st Argument NULL Pointer", s, EINVAL);	
    ERROR(EINVAL);
  }
  if (s->strtype == STRTYPE_NTBS) {
    if ((s->str.cstr && !s->size) || (!s->str.cstr && s->size)){
      ErrorHandler("isnull_m: 1st Argument Invalid String size", s, EINVAL);
      ERROR(EINVAL);
    }

    if (!s->str.cstr) *nullstr = true;
    return 0;
  }

  if (s->strtype == STRTYPE_WSTR) {
    if ((s->str.wstr && !s->size) || (!s->str.wstr && s->size)){
      ErrorHandler("isnull_m: 1st Argument Invalid String Size", s, EINVAL);
      ERROR(EINVAL);
    }
    *nullstr = false;
    if (!s->str.wstr) *nullstr = true;
    return 0;
  }

  ErrorHandler("isnull_m: 1st Argument Invalid String Type", s, EINVAL);
  ERROR(EINVAL);
}

// isempty_m
errno_t
isempty_m(string_m const s, _Bool *emptystr) {
  if (!emptystr){
    ErrorHandler("isempty_m: 2nd Argument NULL Pointer", emptystr, EINVAL);	
    ERROR(EINVAL);
  }
  *emptystr = false;
  // validate s
  if (s == NULL) {
    ErrorHandler("isempty_m: 1st Argument NULL Pointer", emptystr, EINVAL);	
    ERROR(EINVAL);
  }
  if (s->strtype == STRTYPE_NTBS) {
    if ((s->str.cstr && !s->size) || (!s->str.cstr && s->size)){
      ErrorHandler("isempty_m: 1st Argument Invalid String Size", s, EINVAL);
      ERROR(EINVAL);
    }

    if (!s->str.cstr) *emptystr = true;
    else if (!*s->str.cstr) *emptystr = true;
    return 0;
  }

  if (s->strtype == STRTYPE_WSTR) {
    if ((s->str.wstr && !s->size) || (!s->str.wstr && s->size)){
      ErrorHandler("isempty_m: 1st Argument Invalid String Size", s, EINVAL);
      ERROR(EINVAL);
    }

    if (!s->str.wstr) *emptystr = true;
    else if (!*s->str.wstr) *emptystr = true;
    return 0;
  }

  ErrorHandler("isempty_m: 1st Argument Invalid String Type", s, EINVAL);
  ERROR(EINVAL);
}

/* isntbs_m */
errno_t 
isntbs_m(const string_m s, _Bool *ntbsr){
  if(!ntbsr){
    ErrorHandler("isntbs_m: 2nd Argument NULL Pointer", ntbsr, EINVAL);
    ERROR(EINVAL);
  }
  *ntbsr = false;

  /* validate s */
  if(s == NULL) {
    ErrorHandler("isntbs_m: 1st Argument NULL Pointer", s, EINVAL);
    ERROR(EINVAL);
  }

  if(s->strtype == STRTYPE_NTBS)
    *ntbsr = true;
  else if(s->strtype == STRTYPE_WSTR)
    *ntbsr = false;
  else{
    ErrorHandler("isntbs_m: 1st Argument Invalid String Type", s, EINVAL);
    ERROR(EINVAL);
  }
  return 0;
}

/* iswide_m */
errno_t
iswide_m(const string_m s, _Bool *widestr){
  if(!widestr){
    ErrorHandler("iswide_m: 2nd Argument NULL Pointer", widestr, EINVAL);	
    ERROR(EINVAL);
  }
  *widestr = false;

  /* validate s */
  if(s == NULL){
    ErrorHandler("iswide_m: 1st Argumetn NULL Pointer", s, EINVAL);	
    ERROR(EINVAL);
  }
  if(s->strtype == STRTYPE_NTBS)
    *widestr = false;
  else if (s->strtype == STRTYPE_WSTR)
    *widestr = true;
  else {
    ErrorHandler("iswide_m: 1st Argument Invalid String Type", s, EINVAL);
    ERROR(EINVAL);
  }
  return 0;
}
