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

#include <wchar.h> 
#include "string_m.h"
#include "string_m_internals.h"
#include "scanf_internals.h"
#ifndef fwide
extern int fwide(FILE *stream,int mode);
#endif
errno_t vfscanf_m(FILE *file, const string_m format, int *count, va_list args){
  int orientation;
  errno_t rv;

  if(!format){
    ErrorHandler("vfscanf_m: 2nd Argument NULL Pointer", format, EINVAL);
    ERROR(EINVAL);
  }

  orientation = fwide(file, 0);

  rv = wvfscanf_m(file, format, count, args);
  if (rv) ERROR(rv);

  if(orientation > 0)
    return wvfscanf_m(file, format, count, args);
  else
    return cvfscanf_m(file, format, count, args);
}

errno_t
fscanf_m(FILE *file, const string_m format, int *count, ...){
  va_list ap;
  errno_t rv;

  if(!format){
    ErrorHandler("fscanf_m: 2nd Argument NULL Pointer", format, EINVAL);
    ERROR(EINVAL);
  }

  va_start(ap, count);
  rv = vfscanf_m(file, format, count, ap);
  va_end(ap);
  return rv;
}

errno_t
vscanf_m(const string_m format, int *count, va_list args){
  if(!format){
    ErrorHandler("vscanf_m: 1st Argument NULL Pointer", format, EINVAL);
    ERROR(EINVAL);
  }

  return vfscanf_m(stdin, format, count, args);
}

errno_t
scanf_m(const string_m format, int *count, ...){
  va_list ap;
  errno_t rv;

  if(!format){
    ErrorHandler("scanf_m: 1st Argument NULL Pointer", format, EINVAL);
    ERROR(EINVAL);
  }

  va_start(ap, count);
  rv = vfscanf_m(stdin, format, count, ap);
  va_end(ap);
  return rv;
}
