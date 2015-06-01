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

errno_t wstrslice_m(string_m s1, wchar_t *wstr, rsize_t offset, rsize_t len);
errno_t wstrright_m(string_m s1, wchar_t *wstr, rsize_t len);

errno_t
cstrslice_m(string_m s1, char *cstr, rsize_t offset, rsize_t len){
  size_t ilen, s1_size, s2_size;
  errno_t rv;
  size_t rz;

  if(!s1){
    ErrorHandler("cstrclice_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }

  if(s1->strtype == STRTYPE_WSTR){
    wchar_t * t;
    if(!cstr) return wstrslice_m(s1, NULL, offset, len);
    t = (wchar_t *)calloc(strlen(cstr)+1, sizeof(wchar_t));
    if (!t) ERROR(ENOMEM);
    rz = mbstowcs( t,cstr, strlen(cstr));
    if (rz == (size_t) -1) ERROR(EINVAL);
    rv = wstrslice_m(s1, t, offset, len);
    free(t);
    return rv;
  }

  if(s1->strtype != STRTYPE_NTBS){
    ErrorHandler("cstrslice_m: 1st Argument Invalid String type", s1, EINVAL);
    ERROR(EINVAL);
  }
  if((rv = strlen_m(s1, &s1_size)) != 0) ERROR(rv);
	
  if(!cstr || offset > (s2_size = strlen(cstr))){
    return cstrcpy_m(s1, NULL);
  }

  if(offset == s2_size || len == 0){
    return cstrcpy_m(s1, "");
  }

  if(len > s2_size - offset + 1)
    ilen = s2_size - offset + 1;
  else
    ilen = len + 1;

  if(s1->size < ilen + s1_size){
    ErrorHandler("cstrslice_m: Not Enough Memory Allocated", s1, ENOMEM);
    ERROR(ENOMEM);
  }

  // if destination string s1 has charset defined make sure
  // all characters in source are valid
  if ( (s1->charset.cstr) && (strspn(cstr+offset, s1->charset.cstr) < (ilen-1)) ){
    ErrorHandler("cstrslice_m: Charset Invalidates string", cstr, EINVAL);
    ERROR(EINVAL);
  }
	
  memcpy(s1->str.cstr+s1_size, cstr+offset, ilen-1);
  s1->str.cstr[s1_size+ilen-1] = '\0';
  return 0;
}

errno_t
wstrslice_m(string_m s1, wchar_t *wstr, rsize_t offset, rsize_t len){
  size_t ilen, s1_size, s2_size;
  errno_t rv;

  if(!s1){
    ErrorHandler("wstrclice_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }

  if(s1->strtype == STRTYPE_NTBS){
    if((rv = str2wstr_m(s1)) != 0) ERROR(rv);
  }

  if(s1->strtype != STRTYPE_WSTR){
    ErrorHandler("wstrslice_m: 1st Argument Invalid String Type", s1, EINVAL);
    ERROR(EINVAL);
  }

  if((rv = strlen_m(s1, &s1_size)) != 0) ERROR(rv);
	
  if(!wstr || offset > (s2_size = wcslen(wstr))){
    return wstrcpy_m(s1, NULL);
  }

  if(offset == s2_size || len == 0){
    return wstrcpy_m(s1, L"");
  }

  if(len > s2_size - offset + 1)
    ilen = s2_size - offset + 1;
  else
    ilen = len + 1;

  if(s1->size < ilen + s1_size){
    ErrorHandler("wstrslice_m: Not Enough Memory Allocated", s1, ENOMEM);
    ERROR(ENOMEM);
  }

  // if destination string s1 has charset defined make sure
  // all characters in source are valid
  if ( (s1->charset.wstr) && (wcsspn(wstr+offset, s1->charset.wstr) < (ilen-1)) ){
    ErrorHandler("wstrslice_m: Charset Invalidates string", wstr, EINVAL);
    ERROR(EINVAL);
  }
	
  memcpy(s1->str.wstr+s1_size, wstr+offset, (ilen-1)*sizeof(wchar_t));
  s1->str.wstr[s1_size+ilen-1] = '\0';
  return 0;
}

errno_t
strslice_m(string_m s1, string_m s2, rsize_t offset, rsize_t len){
  if(!s1) {
    ErrorHandler("strslice_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }

  if(!s2){
    ErrorHandler("strslice_m: 2nd Argument NULL Pointer", s2, EINVAL);
    ERROR(EINVAL);
  }

  if(s2->strtype == STRTYPE_NTBS){
    return cstrslice_m(s1, s2->str.cstr, offset, len);
  } else if(s2->strtype == STRTYPE_WSTR){
    return wstrslice_m(s1, s2->str.wstr, offset, len);
  }	

  ErrorHandler("strslice_m: 2nd Argument Invalid Sting Type", s2, EINVAL);
  ERROR(EINVAL);
}

errno_t
wstrleft_m(string_m s1, wchar_t *wstr, rsize_t len){
  size_t ilen, s1_size, s2_size;
  errno_t rv;

  if(!s1){
    ErrorHandler("wstrleft_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }

  if(s1->strtype == STRTYPE_NTBS){
    if((rv = str2wstr_m(s1)) != 0) ERROR(rv);
  }

  if(s1->strtype != STRTYPE_WSTR){
    ErrorHandler("wstrleft_m: 1st Argument Invalid String Type", s1, EINVAL);
    ERROR(EINVAL);
  }

  if((rv = strlen_m(s1, &s1_size)) != 0) ERROR(rv);
	
  if(!wstr ){
    return wstrcpy_m(s1, NULL);
  }

  s2_size = wcslen(wstr);
  if(len > s2_size + 1)
    ilen = s2_size + 1;
  else
    ilen = len + 1;

  if(s1->size < ilen + s1_size){
    ErrorHandler("wstrslice_m: Not Enough Memory Allocated", s1, ENOMEM);
    ERROR(ENOMEM);
  }

  // if destination string s1 has charset defined make sure
  // all characters in source are valid
  if ( (s1->charset.wstr) && (wcsspn(wstr, s1->charset.wstr) < (ilen-1)) ){
    ErrorHandler("wstrleft_m: Charset Invalidates string", wstr, EINVAL);
    ERROR(EINVAL);
  }
	
  memcpy(s1->str.wstr+s1_size, wstr, (ilen-1)*sizeof(wchar_t));
  s1->str.wstr[s1_size+ilen-1] = '\0';
  return 0;

}

errno_t
cstrleft_m(string_m s1, char *cstr, rsize_t len){
  size_t ilen, s1_size, s2_size;
  errno_t rv;
  size_t rz;

  if(!s1){
    ErrorHandler("cstrleft_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }

  if(!cstr){
    return cstrcpy_m(s1, NULL);
  }

  if((rv = strlen_m(s1, &s1_size)) != 0) ERROR(rv);

  s2_size = strlen(cstr);

  if(s1->strtype == STRTYPE_WSTR){
    wchar_t *t = (wchar_t *)calloc(s2_size + 1, sizeof(wchar_t));
    if (!t) ERROR(ENOMEM);
    rz = mbstowcs(t, cstr, s2_size);
    if (rz == (size_t) -1) ERROR(EINVAL);
    rv = wstrleft_m(s1, t, len);
    free(t);
    return rv;
  }

  if(s1->strtype != STRTYPE_NTBS){
    ErrorHandler("cstrleft_m: 1st Argument Invalid String Type", s1, EINVAL);
    ERROR(EINVAL);
  }

  if(len > s2_size) ilen = s2_size + 1;
  else ilen = len + 1;

  if(s1->size < ilen + s1_size){
    ErrorHandler("cstrslice_m: Not Enough Memory Allocated", s1, ENOMEM);
    ERROR(ENOMEM);
  }

  // if destination string s1 has charset defined make sure
  // all characters in source are valid
  if ( (s1->charset.cstr) && (strspn(cstr, s1->charset.cstr) < (ilen-1)) ){
    ErrorHandler("cstrleft_m: Charset Invalidates string", cstr, EINVAL);
    ERROR(EINVAL);
  }
	
  memcpy(s1->str.cstr+s1_size, cstr, ilen-1);
  s1->str.cstr[s1_size+ilen-1] = '\0';
  return 0;
}




errno_t
strleft_m(string_m s1, string_m s2, rsize_t len){
  if(!s1){
    ErrorHandler("strleft_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }

  if(!s2){
    ErrorHandler("strleft_m: 2nd Argument NULL Pointer", s2, EINVAL);
    ERROR(EINVAL);
  }	

  if(s2->strtype == STRTYPE_NTBS){
    return cstrleft_m(s1, s2->str.cstr, len);
  } else if(s2->strtype == STRTYPE_WSTR){
    return wstrleft_m(s1, s2->str.wstr, len);
  }

  ErrorHandler("strleft_m: 2nd Argument Invalid String Type", s2, EINVAL);
  ERROR(EINVAL);
}

errno_t
cstrright_m(string_m s1, char *cstr, rsize_t len){
  size_t ilen, s1_size, s2_size;
  errno_t rv;
  size_t rz;

  if(!s1){
    ErrorHandler("cstrright_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }

  if(!cstr){
    return cstrcpy_m(s1, NULL);
  }

  s2_size = strlen(cstr);
  if(s1->strtype == STRTYPE_WSTR){
    wchar_t *t = (wchar_t *)calloc(s2_size + 1, sizeof(wchar_t));
    if (!t) ERROR(ENOMEM);
    rz = mbstowcs(t,cstr, s2_size);
    if (rz == (size_t) -1) ERROR(EINVAL);
    rv = wstrright_m(s1, t, len);
    free(t);
    return rv;
  }

  if((rv = strlen_m(s1, &s1_size)) != 0) ERROR(rv);

  if(len > s2_size+1)
    ilen = s2_size+1;
  else
    ilen = len+1;

  if(s1->size < ilen + s1_size){
    ErrorHandler("cstrright_m: Not Enough Memory Allocated", s1, ENOMEM);
    ERROR(ENOMEM);
  }

  // if destination string s1 has charset defined make sure
  // all characters in source are valid
  if ( (s1->charset.cstr) && (strspn(cstr+s2_size - ilen +1, s1->charset.cstr) < (ilen-1)) ){
    ErrorHandler("cstrright_m: Charset Invalidates string", cstr, EINVAL);
    ERROR(EINVAL);
  }

  memcpy(s1->str.cstr+s1_size, cstr + (s2_size - ilen + 1), (ilen-1) );
  s1->str.cstr[s1_size+ilen-1] = L'\0';
  return 0;
}

errno_t
wstrright_m(string_m s1, wchar_t *wstr, rsize_t len){
  size_t ilen, s1_size, s2_size;
  errno_t rv;

  if(!s1){
    ErrorHandler("wstrright_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }

  if(!wstr){
    return wstrcpy_m(s1, NULL);
  }

  if(s1->strtype == STRTYPE_NTBS){
    if((rv = str2wstr_m(s1)) != 0) ERROR(rv);
  }

  if((rv = strlen_m(s1, &s1_size)) != 0) ERROR(rv);

  s2_size = wcslen(wstr);

  if(len > s2_size+1)
    ilen = s2_size+1;
  else
    ilen = len +1;

  if(s1->size < ilen + s1_size){
    ErrorHandler("wstrright_m: Not Enough Memory Allocated", s1, ENOMEM);
    ERROR(ENOMEM);
  }

  // if destination string s1 has charset defined make sure
  // all characters in source are valid
  if ( (s1->charset.wstr) && (wcsspn(wstr+s2_size-ilen +1, s1->charset.wstr) < (ilen-1)) ){
    ErrorHandler("wstrleft_m: Charset Invalidates string", wstr, EINVAL);
    ERROR(EINVAL);
  }

  memcpy(s1->str.wstr+s1_size, wstr+s2_size-ilen+1, (ilen-1)*sizeof(wchar_t));
  s1->str.wstr[s1_size+ilen -1] = L'\0';
  return 0;
}

errno_t
strright_m(string_m s1, string_m s2, rsize_t len){
  if(!s1){
    ErrorHandler("strright_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }

  if(!s2){
    ErrorHandler("strright_m: 2nd Arugment NULL Pointer", s2, EINVAL);
    ERROR(EINVAL);
  }

  if(s2->strtype == STRTYPE_NTBS)
    return cstrright_m(s1, s2->str.cstr, len);
  else
    return wstrright_m(s1, s2->str.wstr, len);

}

errno_t
cchar_m(string_m s, rsize_t offset, char *c){
  size_t olen;

  errno_t rv;

  if(!s){
    ErrorHandler("cchar_m: 1st Argument NULL Pointer", s, EINVAL);
    ERROR(EINVAL);
  }
  if(!c){
    ErrorHandler("cchar_m: 3rd Argument NULL Pointer", c, EINVAL);
    ERROR(EINVAL);
  }

  if((rv = strlen_m(s, &olen)) != 0) ERROR(rv);

  if(s->strtype == STRTYPE_WSTR){
    rv = wctomb(c,s->str.wstr[offset]);
    if (rv) ERROR(rv);
  } else
    *c = s->str.cstr[offset];

  return 0;
}

errno_t 
wchar_m(string_m s, rsize_t offset, wchar_t *w){
  size_t olen;
  errno_t rv;

  if(!s){
    ErrorHandler("wchar_m: 1st Argument NULL Pointer", s, EINVAL);
    ERROR(EINVAL);
  }
  if(!w){
    ErrorHandler("wchar_m: 3rd Argument NULL Pointer", w, EINVAL);
    ERROR(EINVAL);
  }
  if((rv = strlen_m(s, &olen)) != 0) ERROR(rv);

  if(s->strtype == STRTYPE_NTBS){
    if((rv = str2wstr_m(s)) != 0) ERROR(rv);
  }

  *w = s->str.wstr[offset];
  return 0;
}
