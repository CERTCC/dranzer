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

// strcat_m
//   Inputs
//      s2 - struct of source string
//          s->size - Any nonzero size is valid
//          s->str  - Any string is valid ("" is valid, NULL is not valid)
//   Outputs
//      s1 - struct of destination string
//          s->size - Any nonzero size is valid
//          s->str  - Any string is valid ("" is valid, NULL is not valid)
//   Return
//      status - 0=OK, -1=error
errno_t
strcat_m(string_m s1, const string_m s2) {
  if (!s1){
    ErrorHandler("strcat_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }
  if (!s2){
    ErrorHandler("strcat_m: 2nd Argument NULL Pointer", s2, EINVAL);
    ERROR(EINVAL);
  }

  if (s2->strtype == STRTYPE_NTBS)
    return cstrcat_m(s1, s2->str.cstr);
  else if (s2->strtype == STRTYPE_WSTR)
    return wstrcat_m(s1, s2->str.wstr);
  else{
    ErrorHandler("strcat_m: 2nd Argument Invalid String Type", s2, EINVAL);
    ERROR(EINVAL);
  }
} // end strcat_m

// cstrcat_m
//   Inputs
//      s2 - Literal string char *
//   Outputs
//      s1 - struct of destination string
//          s->size - Any nonzero size is valid
//          s->str  - Any string is valid ("" is valid, NULL is not valid)
errno_t 
cstrcat_m(string_m s1, const char *s2) {
  string_m const ostr = s1;
  errno_t rv;
  size_t olen, ilen;


  //validate arguments
  if (!s1) {
    ErrorHandler("cstrcat_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }
  if (!s2){
    ErrorHandler("cstrcat_m: 2nd Argument NULL Pointer", s2, EINVAL);
    ERROR(EINVAL);
  }

  ilen = strlen(s2);
  if (!ilen) return 0;

  if (s1->strtype == STRTYPE_WSTR) {
    wchar_t *t = (wchar_t*)calloc(ilen+1, sizeof(wchar_t));
    if (!t) {
      ErrorHandler("cstrcat_m: Calloc Error", t, ENOMEM);
      ERROR(ENOMEM);
    }
    rv = (errno_t)mbstowcs(t, s2, ilen+1);
    if (rv < 0) ERROR(errno);
    rv = wstrcat_m(s1, t);
    free(t);
    return rv;
  }
  if (s1->strtype != STRTYPE_NTBS){
    ErrorHandler("cstrcat_m: 1st Argument Invalid String Type", s1, EINVAL);
    ERROR(EINVAL);
  }

  if (!ostr->size) return cstrcpy_m(s1, s2);

  rv = strlen_m(s1, &olen);
  if (rv) ERROR(rv);

  // if destination string s1 has charset defined make sure
  // all characters in source are valid
  if ( (s1->charset.cstr) && (strspn(s2, s1->charset.cstr) != ilen) ){
    ErrorHandler("cstrcat_m: Charset Invalidates string", s2, EINVAL);
    ERROR(EINVAL);
  }

  if (ostr->maxsize && ostr->maxsize < ilen+olen+1) {
    ErrorHandler("cstrcat_m: String Too Large", s2, EINVAL);
    ERROR(EINVAL);
  }

  if (ostr->size < ilen+olen+1)
    REALLOC_C(ostr, ilen+olen+1);

  memcpy(olen + ostr->str.cstr, s2, ilen+1);
  return 0;

} // end cstrcat_m

// wstrcat_m
//   Inputs
//      s2 - Literal string wchar_t *
//   Outputs
//      s1 - struct of destination string
//          s->size - Any nonzero size is valid
//          s->str  - Any string is valid ("" is valid, NULL is not valid)
errno_t 
wstrcat_m(string_m s1, const wchar_t *s2) {
  string_m const ostr = s1;
  errno_t rv;
  size_t olen, ilen;

  //validate arguments
  if (!s1) {
    ErrorHandler("wstrcat_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }
  if (!s2) {
    ErrorHandler("wstrcat_m: 2nd Argument NULL Pointer", s2, EINVAL);
    ERROR(EINVAL);
  }

  if (!ostr->size) return wstrcpy_m(s1, s2);

  if (s1->strtype != STRTYPE_WSTR) {
    rv = str2wstr_m(s1);
    if (rv) ERROR(rv);
  }

  ilen = wcslen(s2);
  if (ilen == 0) return 0;

  rv = strlen_m(s1, &olen);
  if (rv) ERROR(rv);

  // if destination string s1 has charset defined make sure
  // all characters in source are valid
  if ( (s1->charset.wstr) && (wcsspn(s2, s1->charset.wstr) != ilen) ){
    ErrorHandler("wstrcat_m: Charset Invalidates String", s2, EINVAL);
    ERROR(EINVAL);
  }

  if (ostr->maxsize && ostr->maxsize < ilen+olen+1) {
    ErrorHandler("wstrcat_m: String Too Large", s2, EINVAL);
    ERROR(EINVAL);
  }

  if (ostr->size < ilen+olen+1)
    REALLOC_W(ostr, ilen+olen+1);

  memcpy(olen + ostr->str.wstr, s2, (ilen+1)*sizeof(wchar_t));
  return 0;

} // end wstrcat_m

// strncat_m
errno_t
strncat_m(string_m s1, const string_m s2, rsize_t nchar){
  /* Validate Arguments */
  if (!s1){
    ErrorHandler("strncat_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }
  if (!s2){
    ErrorHandler("strncat_m: 2nd Argument NULL Pointer", s2, EINVAL);
    ERROR(EINVAL);
  }

  if (s2->strtype == STRTYPE_NTBS)
    return cstrncat_m(s1, s2->str.cstr, nchar);
  else if (s2->strtype == STRTYPE_WSTR)
    return wstrncat_m(s1, s2->str.wstr, nchar);
  else{
    ErrorHandler("strcat_m: 2nd Argument Invalid String Type", s2, EINVAL);
    ERROR(EINVAL);
  }
} // end strncat_m

// cstrncat_m
errno_t
cstrncat_m(string_m s1, const char *s2, rsize_t nchar){
  string_m const ostr = s1;
  errno_t rv;
  size_t olen, ilen;

	
  if(!s1){
    ErrorHandler("cstrncat_m: 1st Arguemnt NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }

  if (!s2) return 0;
	
  if (s1->strtype == STRTYPE_WSTR){
    wchar_t *t = (wchar_t *)calloc(strlen(s2),sizeof(wchar_t));
    if (!t) ERROR(ENOMEM);
    if (mbstowcs(t, s2, strlen(s2)) == (size_t) -1) ERROR(EINVAL);
    rv = wstrncat_m(s1, t, nchar);
    free(t);
    return rv;
  }

  if (s1->strtype != STRTYPE_NTBS){
    ErrorHandler("cstrncat_m: 1st Argument Invalid String Type", s1, EINVAL);
    ERROR(EINVAL);
  }

  if (!ostr->size) return cstrncpy_m(s1, s2, nchar);

  rv = strlen_m(s1, &olen);
  if (rv) ERROR(rv);

  ilen = strlen(s2) + 1;
  if(ilen > nchar) ilen = nchar +1;

  // if destination string s1 has charset defined make sure
  // all characters in source are valid
  if ( (s1->charset.cstr) && (strspn(s2, s1->charset.cstr) < (ilen-1)) ){
    ErrorHandler("cstrcat_m: Charset Invalidates string", s2, EINVAL);
    ERROR(EINVAL);
  }

  if (ostr->maxsize && ostr->maxsize < ilen+olen+1) {
    ErrorHandler("cstrncat_m: String Too Large", s2, EINVAL);
    ERROR(EINVAL);
  }

  if (ostr->size < ilen+olen+1)
    REALLOC_C(ostr, ilen+olen+1);

  memcpy(olen + ostr->str.cstr, s2, (ilen-1)*sizeof(char));
  s1->str.cstr[olen+ilen-1] = '\0';
  return 0;
}

errno_t
wstrncat_m(string_m s1, const wchar_t *s2, rsize_t nchar){
  string_m const ostr = s1;
  errno_t rv;
  size_t olen, ilen;
	
  if(!s1){
    ErrorHandler("cstrncat_m: 1st Arguemnt NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }

  if (!s2) return 0;

  if (s1->strtype != STRTYPE_WSTR){
    rv = str2wstr_m(s1);
    if(rv != 0) return rv;
  }

  if (!ostr->size) return wstrncpy_m(s1, s2, nchar);

  rv = strlen_m(s1, &olen);
  if (rv) ERROR(rv);

  ilen = wcslen(s2) + 1;
  if(ilen > nchar) ilen = nchar +1;

  // if destination string s1 has charset defined make sure
  // all characters in source are valid
  if ( (s1->charset.wstr) && (wcsspn(s2, s1->charset.wstr) < (ilen-1)) ){
    ErrorHandler("cstrcat_m: Charset Invalidates string", s2, EINVAL);
    ERROR(EINVAL);
  }

  if (ostr->maxsize && ostr->maxsize < ilen+olen+1) {
    ErrorHandler("wstrncat_m: String Too Large", s2, EINVAL);
    ERROR(EINVAL);
  }

  if (ostr->size < (ilen+olen+1)*sizeof(wchar_t))
    REALLOC_W(ostr, ilen+olen+1);

  memmove(olen + ostr->str.wstr, s2, (ilen-1)*sizeof(wchar_t));
  s1->str.wstr[olen+ilen-1] = '\0';
  return 0;
}
