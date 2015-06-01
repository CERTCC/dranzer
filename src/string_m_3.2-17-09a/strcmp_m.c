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
#include <wchar.h>

#include "string_m.h"
#include "string_m_internals.h"

errno_t
strcmp_m(const string_m s1, const string_m s2, int *cmp){
  /* validate Arguments */
  if (!s1){
    ErrorHandler("strcmp_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }

  if (!s2){
    ErrorHandler("strcmp_m: 2nd Argument NULL Pointer", s2, EINVAL);
    ERROR(EINVAL);
  }

  if (!cmp){
    ErrorHandler("strcmp_m: 3rd Argument NULL Pointer", cmp, EINVAL);
    ERROR(EINVAL);
  }
  *cmp = 0;

  if( s2->strtype == STRTYPE_NTBS){
    return cstrcmp_m(s1, s2->str.cstr, cmp);
  } else if (s2->strtype == STRTYPE_WSTR){
    return wstrcmp_m(s1, s2->str.wstr, cmp);
  } else {
    ErrorHandler("strcmp_m: 2nd Argument Invalid String Type", s2, EINVAL);
    ERROR(EINVAL);
  }
}

errno_t
cstrcmp_m(const string_m s1, const char *cstr, int *cmp){
  errno_t rv;
  size_t rz;
  const char *ptr1, *ptr2;

  if (!s1) {
    ErrorHandler("cstrcmp_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }
  if (!cmp){
    ErrorHandler("cstrcmp_m: 3rd Argument NULL Pointer", cmp, EINVAL);
    ERROR(EINVAL);
  }
  *cmp = 0;

  if (s1->strtype == STRTYPE_WSTR){
    wchar_t *t;
    if (!cstr) return wstrcmp_m(s1, NULL, cmp);
    t = (wchar_t *)calloc(strlen(cstr) + 1,sizeof(wchar_t));
    if (!t) ERROR(ENOMEM);
    rz = mbstowcs(t, cstr, strlen(cstr));
    if (rz == (size_t) -1) ERROR(EINVAL);
    rv = wstrcmp_m(s1, t, cmp);
    free(t);
    return rv;
  }

  if (s1->strtype != STRTYPE_NTBS){
    ErrorHandler("cstrcmp_m: 1st Argument Invalid String Type", s1, EINVAL);
    ERROR(EINVAL);
  }

  if(!s1->str.cstr){
    if(!cstr) *cmp = 0;
    else *cmp = -1;
  } else if(!cstr){
    *cmp = 1;
  } else {
    ptr1 = s1->str.cstr;
    ptr2 = cstr;
    for(;*ptr1 && (*ptr1 == *ptr2); ptr1++, ptr2++)
      ;
    *cmp = (unsigned char)*ptr1 - (unsigned char)*ptr2;
  }

  return 0;
}

errno_t
wstrcmp_m(const string_m s1, const wchar_t *wstr, int *cmp){
  errno_t rv;
  const wchar_t *ptr1, *ptr2;

  if(!s1){
    ErrorHandler("wstrcmp_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }

  if(!cmp){
    ErrorHandler("wstrcmp_m: 3rd Argument NULL Pointer", cmp, EINVAL);
    ERROR(EINVAL);
  }
  *cmp = 0;

  if (s1->strtype == STRTYPE_NTBS){
    rv = str2wstr_m(s1);
    if (rv) ERROR(rv);
  }
  if(!s1->str.wstr){
    if(!wstr) *cmp = 0;
    else *cmp = -1;
  } else if(!wstr){
    *cmp = 1;
  } else {
    ptr1 = s1->str.wstr;
    ptr2 = wstr;
    for(;*ptr1 && (*ptr1 == *ptr2); ptr1++, ptr2++)
      ;
    *cmp = *ptr1 - *ptr2;
  }

  return 0;
}

errno_t
strncmp_m(const string_m s1, const string_m s2, rsize_t nchar, int *cmp){
  /* validate arguments */
  if(!s1){
    ErrorHandler("strncmp_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }

  if(!s2){
    ErrorHandler("strncmp_m: 2nd Argument NULL Pointer", s2, EINVAL);
    ERROR(EINVAL);
  }

  if(!cmp){
    ErrorHandler("strncmp_m: 4th Argument NULL Pointer", cmp, EINVAL);
    ERROR(EINVAL);
  }
  *cmp = 0;

	

  if(s2->strtype == STRTYPE_NTBS){
    return cstrncmp_m(s1, s2->str.cstr, nchar, cmp);
  } else if(s2->strtype == STRTYPE_WSTR){
    return wstrncmp_m(s1, s2->str.wstr, nchar, cmp);
  } else{
    ErrorHandler("strncmp_m: 2nd Argument Invalid String Type", s2, EINVAL);
    ERROR(EINVAL);
  }
}

errno_t
cstrncmp_m(const string_m s1, const char *cstr, rsize_t nchar, int *cmp){
  errno_t rv;
  size_t rz;
  //size_t olen, ilen;
  const char *ptr1, *ptr2;

  if (!s1) {
    ErrorHandler("cstrcmp_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }
  if (!cmp){
    ErrorHandler("cstrcmp_m: 3rd Argument NULL Pointer", cmp, EINVAL);
    ERROR(EINVAL);
  }
  *cmp = 0;

  if(nchar == 0) return 0;

  if (s1->strtype == STRTYPE_WSTR){
    wchar_t *t;
    if (!cstr) return wstrncmp_m(s1, NULL, nchar, cmp);
    t = (wchar_t *)calloc(strlen(cstr) + 1,sizeof(wchar_t));
    if (!t) ERROR(ENOMEM);
    rz = mbstowcs(t,cstr, strlen(cstr));
    if (rz == (size_t) -1) ERROR(EINVAL);
    rv = wstrncmp_m(s1, t, nchar, cmp);
    free(t);
    return rv;
  }

  if (s1->strtype != STRTYPE_NTBS){
    ErrorHandler("cstrcmp_m: 1st Argument Invalid String Type", s1, EINVAL);
    ERROR(EINVAL);
  }

  if(!s1->str.cstr){
    if(!cstr) *cmp = 0;
    else *cmp = -1;
  } else if(!cstr){
    *cmp = 1;
  } else {
    size_t n = 0;
    ptr1 = s1->str.cstr;
    ptr2 = cstr;
    for(;*ptr1 && (*ptr1 == *ptr2) && (n < nchar); ptr1++, ptr2++, n++)
      ;
    *cmp = (unsigned char)*ptr1 - (unsigned char)*ptr2;
  }

  return 0;
}

errno_t
wstrncmp_m(const string_m s1, const wchar_t *wstr, rsize_t nchar, int *cmp){
  errno_t rv;
  //size_t ilen, olen;
  const wchar_t *ptr1, *ptr2;

  if(!s1){
    ErrorHandler("wstrcmp_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }

  if(!cmp){
    ErrorHandler("wstrcmp_m: 3rd Argument NULL Pointer", cmp, EINVAL);
    ERROR(EINVAL);
  }
  *cmp = 0;

  if(nchar == 0) return 0;

  if (s1->strtype == STRTYPE_NTBS){
    rv = str2wstr_m(s1);
    if (rv) ERROR(rv);
  }
  if(!s1->str.wstr){
    if(!wstr) *cmp = 0;
    else *cmp = -1;
  } else if(!wstr){
    *cmp = 1;
  } else {
    size_t n = 0;
    ptr1 = s1->str.wstr;
    ptr2 = wstr;
    for(;*ptr1 && (*ptr1 == *ptr2) && (n < nchar); ptr1++, ptr2++, n++)
      ;
    *cmp = *ptr1 - *ptr2;
  }

  return 0;

}
