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
strspn_m(const string_m str, const string_m accept, rsize_t *len){
  if(!str){
    ErrorHandler("strspn_m: 1st Argument NULL Pointer", str, EINVAL);
    ERROR(EINVAL);
  }

  if(!accept){
    ErrorHandler("strspn_m: 2nd Argument NULL Pointer", accept, EINVAL);
    ERROR(EINVAL);
  }
  if(!len){
    ErrorHandler("strspn_m: 3rd Argument NULL Pointer", len, EINVAL);
    ERROR(EINVAL);
  }


  if(accept->strtype == STRTYPE_NTBS){
    return cstrspn_m(str, accept->str.cstr, len);
  } else if(accept->strtype == STRTYPE_WSTR){
    return wstrspn_m(str, accept->str.wstr, len);
  } else {
    ErrorHandler("strspn_m: 2nd Argument Invalid String Type", accept, EINVAL);
    ERROR(EINVAL);
  }
}

errno_t
cstrspn_m(const string_m str, const char *accept, rsize_t *len){
  errno_t rv;
  size_t rz;
  char *s1, *ostr;
  const char *s2;
  if(!str){
    ErrorHandler("cstrspn_m: 1st Argument NULL Pointer", str, EINVAL);
    ERROR(EINVAL);
  }
  if(!len){
    ErrorHandler("cstrspn_m: 3rd Argument NULL Pointer", len, EINVAL);
    ERROR(EINVAL);
  }

  if(!str->size){
    *len = 0;
    return 0;
  }
  if(!accept){
    return strlen_m(str, len);
  }

  if(str->strtype == STRTYPE_WSTR){
    wchar_t *t = (wchar_t *)malloc((strlen(accept) + 1)*sizeof(wchar_t));
    if (!t) ERROR(ENOMEM);
    rz = mbstowcs(t, accept, strlen(accept)+1);
    if (rz == (size_t) -1) ERROR(EINVAL);
    rv = wstrspn_m(str, t, len);
    free(t);
    return rv;
  }


  s1 = str->str.cstr;
  ostr = s1;
	

  while(*s1){
    for(s2 = accept;*s2;s2++)
      if(*s1 == *s2) break;
    if(*s2 == '\0') break;
    s1++;
  }

  *len = s1 - ostr;
  return 0;
}

errno_t
wstrspn_m(const string_m str, const wchar_t *accept, rsize_t *len){
  errno_t rv;
  wchar_t *s1, *ostr;
  const wchar_t *s2;
  if(!str){
    ErrorHandler("cstrspn_m: 1st Argument NULL Pointer", str, EINVAL);
    ERROR(EINVAL);
  }
  if(!len){
    ErrorHandler("cstrspn_m: 3rd Argument NULL Pointer", len, EINVAL);
    ERROR(EINVAL);
  }

  if(!str->size){
    *len = 0;
    return 0;
  }

  if(!accept){
    return strlen_m(str, len);
  }

  if(str->strtype == STRTYPE_NTBS){
    rv = str2wstr_m(str);
    if (rv) ERROR(rv);
  }

  s1 = str->str.wstr;
  ostr = s1;
	

  while(*s1){
    for(s2 = accept;*s2;s2++)
      if(*s1 == *s2) break;
    if(*s2 == '\0') break;
    s1++;
  }

  *len = (s1 - ostr);
  return 0;
}


// strncspn_m
errno_t
strcspn_m(const string_m str, const string_m reject, rsize_t *len){
  if(!str){
    ErrorHandler("strspn_m: 1st Argument NULL Pointer", str, EINVAL);
    ERROR(EINVAL);
  }

  if(!reject){
    ErrorHandler("strspn_m: 2nd Argument NULL Pointer", reject, EINVAL);
    ERROR(EINVAL);
  }
  if(!len){
    ErrorHandler("strspn_m: 3rd Argument NULL Pointer", len, EINVAL);
    ERROR(EINVAL);
  }


  if(reject->strtype == STRTYPE_NTBS){
    return cstrcspn_m(str, reject->str.cstr, len);
  } else if(reject->strtype == STRTYPE_WSTR){
    return wstrcspn_m(str, reject->str.wstr, len);
  } else {
    ErrorHandler("strspn_m: 2nd Argument Invalid String Type", reject, EINVAL);
    ERROR(EINVAL);
  }
}

errno_t
cstrcspn_m(const string_m str, const char *reject, rsize_t *len){
  errno_t rv;
  size_t rz;
  char *s1, *ostr;
  const char *s2;
  if(!str){
    ErrorHandler("cstrspn_m: 1st Argument NULL Pointer", str, EINVAL);
    ERROR(EINVAL);
  }
  if(!len){
    ErrorHandler("cstrspn_m: 3rd Argument NULL Pointer", len, EINVAL);
    ERROR(EINVAL);
  }

  if(!str->size) {
    *len = 0;
    return 0;
  }

  if(!reject){
    return strlen_m(str, len);
  }

  if(str->strtype == STRTYPE_WSTR){
    wchar_t *t = (wchar_t *)malloc((strlen(reject)+1)*sizeof(wchar_t));
    if (!t) ERROR(ENOMEM);
    rz = mbstowcs(t, reject, strlen(reject)+1);
    if (rz == (size_t) -1) ERROR(EINVAL);
    rv = wstrcspn_m(str, t, len);
    free(t);
    return rv;
  }

  s1 = str->str.cstr;
  ostr = s1;
	

  while(*s1){
    for(s2 = reject;*s2;s2++)
      if(*s1 == *s2) break;
    if(*s2) break;
    s1++;
  }

  *len = s1 - ostr;
  return 0;
}

errno_t
wstrcspn_m(const string_m str, const wchar_t *reject, rsize_t *len){
  errno_t rv;
  wchar_t *s1, *ostr;
  const wchar_t *s2;
  if(!str){
    ErrorHandler("cstrspn_m: 1st Argument NULL Pointer", str, EINVAL);
    ERROR(EINVAL);
  }
  if(!len){
    ErrorHandler("cstrspn_m: 3rd Argument NULL Pointer", len, EINVAL);
    ERROR(EINVAL);
  }

  if(!str->size){
    *len = 0;
    return 0;
  }

  if(!reject){
    return strlen_m(str, len);
  }
  if(str->strtype == STRTYPE_NTBS){
    rv = str2wstr_m(str);
    if (rv) ERROR(rv);
  }

  s1 = str->str.wstr;
  ostr = s1;
	

  while(*s1){
    for(s2 = reject;*s2;s2++)
      if(*s1 == *s2) break;
    if(*s2) break;
    s1++;
  }

  *len = (s1 - ostr);
  return 0;
}
