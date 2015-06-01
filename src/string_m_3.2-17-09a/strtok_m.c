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

#ifdef __MSVC__
extern _CRTIMP char *  __cdecl strtok_s(char *, const char *, char **);
#endif

// this function is really messed up--need to redesign
errno_t strtok_m(string_m token, string_m str, const string_m delim, string_m ptr) {
  errno_t rv;
  size_t slen;
  //validate arguments
  if (!token){
    ErrorHandler("strtok_m: 1st Argument NULL Pointer", token, EINVAL);
    ERROR(EINVAL);
  }
  if(!str){
    ErrorHandler("strtok_m: 2nd Argument NULL Pointer", str, EINVAL);
    ERROR(EINVAL);
  }
  if(!delim){
    ErrorHandler("strtok_m: 3rd Argument NULL Pointer", delim, EINVAL);
    ERROR(EINVAL);
  }
  if(!ptr){
    ErrorHandler("strtok_m: 4th Argument NULL Pointer", ptr, EINVAL);
    ERROR(EINVAL);
  }
  rv = strlen_m(str, &slen);
  if (rv) ERROR(rv);

  //validate token
  if (str->strtype != delim->strtype) {
    ErrorHandler("strtok_m: 2nd and 3rd Argument String Types Don't Match", delim, EINVAL);	
    ERROR(EINVAL);
  }

  if (str->strtype == STRTYPE_NTBS) {
    size_t c;
    rv = cstrcpy_m(token, NULL);
    if (rv) ERROR(rv);
    rv = cstrcpy_m(ptr, NULL);
    if (rv) ERROR(rv);
    if (str->str.cstr == NULL) return 0;
    if (delim->str.cstr != NULL) c = strcspn(str->str.cstr, delim->str.cstr);
    else c = slen;
    rv = cstrncpy_m(token, str->str.cstr, c);
    if (rv) ERROR(rv);
    if (c < slen) {
      rv = cstrncpy_m(ptr, str->str.cstr + c+1, slen-c-1);
      if (rv) ERROR(rv);
    }
  } else if (str->strtype == STRTYPE_WSTR) {
    size_t c;
    rv = wstrcpy_m(token, NULL);
    if (rv) ERROR(rv);
    rv = wstrcpy_m(ptr, NULL);
    if (rv) ERROR(rv);
    if (str->str.wstr == NULL) return 0;
    if (delim->str.wstr != NULL) c = wcscspn(str->str.wstr, delim->str.wstr);
    else c = slen;
    rv = wstrncpy_m(token, str->str.wstr, c);
    if (rv) ERROR(rv);
    if (c < slen) {
      rv = wstrncpy_m(ptr, str->str.wstr + c+1, slen-c-1);
      if (rv) ERROR(rv);
    }
  } else ERROR(EINVAL);

  return 0;
}


errno_t
cstrchr_m(string_m out, const string_m str, char c){
  errno_t rv;

  if(!str){
    ErrorHandler("cstrchr_m: 2nd Argument NULL Pointer", str, EINVAL);
    ERROR(EINVAL);
  }	
  if(!out){
    ErrorHandler("cstrchr_m: 1st Argument NULL Pointer", out, EINVAL);
    ERROR(EINVAL);
  }

  if(!str->size){
    return strcpy_m(out, str);
  }

  if(str->strtype == STRTYPE_WSTR){
    wchar_t *t = (wchar_t *)malloc(sizeof(wchar_t));
    if (!t) ERROR(ENOMEM);
    if (mbtowc(t, &c, 1) == -1) ERROR(EINVAL);
    rv = wstrchr_m(out, str, *t);
    free(t);
    return rv;
  } else if(str->strtype == STRTYPE_NTBS){
    char *t = str->str.cstr;
    for(;*t && (*t != c); t++)
      ;
    if(!(*t)) return cstrcpy_m(out, NULL);
    else return cstrcpy_m(out, t);
  }
  ErrorHandler("cstrchr_m: 2nd Argument Invalid String Type", str, EINVAL);
  ERROR(EINVAL);
}

errno_t
wstrchr_m(string_m out, const string_m str, wchar_t c){
  errno_t rv;

  if(!str){
    ErrorHandler("cstrchr_m: 2nd Argument NULL Pointer", str, EINVAL);
    ERROR(EINVAL);
  }	
  if(!out){
    ErrorHandler("cstrchr_m: 1st Argument NULL Pointer", out, EINVAL);
    ERROR(EINVAL);
  }

  if(!str->size){
    return strcpy_m(out, str);
  }

  if(str->strtype == STRTYPE_WSTR){
    wchar_t *t = str->str.wstr;
    for(;*t && (*t != c); t++)
      ;
    if(!(*t)) return wstrcpy_m(out, NULL);
    else return wstrcpy_m(out, t);
  } else if(str->strtype == STRTYPE_NTBS){
    char *t = (char *)malloc(1);
    if (!t) ERROR(ENOMEM);
    if (wctomb(t, c) == -1) ERROR(EINVAL);
    rv = cstrchr_m(out, str, *t);
    free(t);
    return rv;
  } else {
    ErrorHandler("cstrchr_m: 2nd Argument Invalid String Type", str, EINVAL);
    ERROR(EINVAL);
  }
}

