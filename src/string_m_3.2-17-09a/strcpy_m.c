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

// strcpy_m
//   Inputs
//      s2 - struct of source string
//          s->size - Any nonzero size is valid
//          s->str  - Any string is valid ("" is valid, NULL is not valid)
//   Outputs
//      s1 - struct of destination string
//          s->size - Any nonzero size is valid
//          s->str  - Any string is valid ("" is valid, NULL is not valid)
errno_t
strcpy_m(string_m s1, const string_m s2) {
  //validate arguments
  if (!s1) {
    ErrorHandler("strcpy_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }
  if (!s2){
    ErrorHandler("strcpy_m: 2nd Argument NULL pointer", s2, EINVAL);
    ERROR(EINVAL);
  }

  if (s2->strtype == STRTYPE_NTBS)
    return cstrcpy_m(s1, s2->str.cstr);
  else if (s2->strtype == STRTYPE_WSTR)
    return wstrcpy_m(s1, s2->str.wstr);
  else{
    ErrorHandler("strcpy_m: 2nd Argument Invalid String Type", s2, EINVAL);
    ERROR(EINVAL);
  }

} // end strcpy_ml

// cstrcpy_l
//   Inputs
//      s2 - Literal string char *
//   Outputs
//      s1 - struct of destination string
//          s->size - Any nonzero size is valid
//          s->str  - Any string is valid ("" is valid, NULL is not valid)
errno_t
cstrcpy_m(string_m s1, const char *s2) {
  size_t s2_size;
	

  //validate arguments
  if (!s1){
    ErrorHandler("cstrcpy_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }

  if (s1->strtype == STRTYPE_WSTR) {   	
    free(s1->str.wstr);
    s1->str.cstr = NULL;
    s1->size = 0;
    s1->strtype = STRTYPE_NTBS;
    if (s1->charset.wstr){
      wchar_t *t = (wchar_t *)malloc((wcslen(s1->charset.wstr) + 1) * sizeof(wchar_t));
      if (!t) ERROR(ENOMEM);
      (void) wcscpy(t, s1->charset.wstr);
      free(s1->charset.wstr);
      s1->charset.cstr = (char *)malloc(wcslen(t) * sizeof(wchar_t) + 1);
      if (!(s1->charset.cstr)) ERROR(ENOMEM);
      if (wcstombs(s1->charset.cstr, t, wcslen(t) * sizeof(wchar_t) + 1) == (size_t) -1) ERROR(EINVAL);
    }
  }
  if (s1->strtype != STRTYPE_NTBS){
    ErrorHandler("cstrcpy_m: 1st Argument Invalid String Type", s1, EINVAL);
    ERROR(EINVAL);
  }

  if (s2 == NULL) {
    if (s1->str.cstr) {
      free(s1->str.cstr);
      s1->str.cstr = NULL;
      s1->size = 0;
    }
    return 0;
  }

  s2_size=strlen(s2)+1;

  if (s1->maxsize && s1->maxsize < s2_size) {
    ErrorHandler("cstrcpy_m: String Too Large", s2, EINVAL);
    ERROR(EINVAL);
  }

  // if destination string s1 has charset defined make sure
  // all characters in source are valid
  if ( (s1->charset.cstr) && (strspn(s2, s1->charset.cstr) != (s2_size-1)) ){
    ErrorHandler("cstrcpy_m: Charset Invalidates String", s2, EINVAL);
    ERROR(EINVAL);
  }

  // else if the existing string is too small, reallocate
  else if (s2_size > s1->size) {
    REALLOC_C(s1, s2_size);
  }

  // the string exists and is large enough
  memcpy(s1->str.cstr, s2, s2_size);
    
  return 0;
} // end strcpy_m

// cstrcpy_l
//   Inputs
//      s2 - Literal string wchar_t *
//   Outputs
//      s1 - struct of destination string
//          s->size - Any nonzero size is valid
//          s->str  - Any string is valid ("" is valid, NULL is not valid)
errno_t
wstrcpy_m(string_m s1, const wchar_t *s2) {
  size_t s2_size;
  errno_t rv;
  //validate arguments
  if (!s1){
    ErrorHandler("wstrcpy_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }

  if (s1->strtype == STRTYPE_NTBS) {
    char *ostr = s1->str.cstr;
    const size_t osize = s1->size;

    s1->str.cstr = NULL;
    s1->size = 0;
    
    rv = str2wstr_m(s1);
    if (rv) {
      s1->str.cstr = ostr;
      s1->size = osize;
      ERROR(rv);
    }

    free(ostr);
  }
  if (s1->strtype != STRTYPE_WSTR){
    ErrorHandler("wstrcpy_m: 1st Argument Invalid String Type", s1, EINVAL);
    ERROR(EINVAL);
  }

  if (s2 == NULL) {
    if (s1->str.wstr) {
      free(s1->str.wstr);
      s1->str.wstr = NULL;
      s1->size = 0;
    }
    return 0;
  }

  s2_size=wcslen(s2)+1;

  if (s1->maxsize && s1->maxsize < s2_size) {
    ErrorHandler("wstrcpy_m: String Too Large", s2, EINVAL);
    ERROR(EINVAL);
  }

  // if destination string s1 has charset defined make sure
  // all characters in source are valid
  if ( (s1->charset.wstr) && (wcsspn(s2, s1->charset.wstr) != (s2_size-1)) ){
    ErrorHandler("wstrcpy_m: Charest Invalidates String", s2, EINVAL);
    ERROR(EINVAL);
  }

  if (s1->maxsize && s1->maxsize < s2_size) {
    ErrorHandler("wstrcpy_m: String Too Large", s2, EINVAL);
    ERROR(EINVAL);
  }

  // else if the existing string is too small, reallocate
  if (s2_size > s1->size) {
    REALLOC_W(s1, s2_size);
  }

  // the string exists and is large enough
  memcpy(s1->str.wstr, s2, s2_size * sizeof(wchar_t));
    
  return 0;

} // end strcpy_m

// strncpy_m
//   Inputs
//      s2 - struct of source string
//          s->size - Any nonzero size is valid
//          s->str  - Any string is valid ("" is valid, NULL is not valid)
//      nchar - number of characters to copy
//   Outputs
//      s1 - struct of destination string
//          s->size - Any nonzero size is valid
//          s->str  - Any string is valid ("" is valid, NULL is not valid)
errno_t
strncpy_m(string_m s1, const string_m s2, size_t nchar) {
  //validate arguments
  if (!s1) {
    ErrorHandler("strncpy_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }
  if (!s2){
    ErrorHandler("strncpy_m: 2nd Argument NULL Pointer", s2, EINVAL);
    ERROR(EINVAL);
  }

  if (s2->strtype == STRTYPE_NTBS)
    return cstrncpy_m(s1, s2->str.cstr, nchar);
  else if (s2->strtype == STRTYPE_WSTR)
    return wstrncpy_m(s1, s2->str.wstr, nchar);
  else{
    ErrorHandler("strncpy_m: 2nd Argument Invalid String Type", s2, EINVAL);     
    ERROR(EINVAL);
  }
} // end strcpy_ml

// cstrncpy_m
//   Inputs
//      s2 - Literal string char *
//	nchar - maximum number of characters to copy
//   Outputs
//      s1 - struct of destination string
//          s->size - Any nonzero size is valid
//          s->str  - Any string is valid ("" is valid, NULL is not valid)
errno_t
cstrncpy_m(string_m s1, const char *s2, size_t nchar) {
  size_t s2_size;

  //validate arguments
  if (!s1){
    ErrorHandler("cstrncpy_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }

  if (s1->strtype == STRTYPE_WSTR) {
   	
    free(s1->str.wstr);
    s1->str.cstr = NULL;
    s1->size = 0;
    s1->strtype = STRTYPE_NTBS;
    if (s1->charset.wstr){
      wchar_t *t = (wchar_t *)malloc((wcslen(s1->charset.wstr) + 1) * sizeof(wchar_t));
      if (!t) ERROR(ENOMEM);
      (void) wcscpy(t, s1->charset.wstr);
      free(s1->charset.wstr);
      s1->charset.cstr = (char *)malloc(wcslen(t) * sizeof(wchar_t) + 1);
      if (!(s1->charset.cstr)) ERROR(ENOMEM);
      if (wcstombs(s1->charset.cstr, t, wcslen(t) * sizeof(wchar_t) + 1) == (size_t) -1) ERROR(EINVAL);
    }
  }
  if (s1->strtype != STRTYPE_NTBS){
    ErrorHandler("cstrncpy_m: 1st Argument Invalid String Type", s1, EINVAL);
    ERROR(EINVAL);
  }

  if (s2 == NULL) {
    if (s1->str.cstr) {
      free(s1->str.cstr);
      s1->str.cstr = NULL;
      s1->size = 0;
    }
    return 0;
  }

  s2_size=strlen(s2)+1;
  if (s2_size > nchar) s2_size = nchar+1;

  if (s1->maxsize && s1->maxsize < s2_size) {
    ErrorHandler("cstrncpy_m: String Too Large", s2, EINVAL);
    ERROR(EINVAL);
  }

  // if destination string s1 has charset defined make sure
  // all characters in source are valid
  if ( (s1->charset.cstr) && (strspn(s2, s1->charset.cstr) < (s2_size-1)) ) {
    ErrorHandler("cstrncpy_m: Charset Invalidates String To Be Copied", s2, EINVAL);	
    ERROR(EINVAL);
  }

  // else if the existing string is too small, reallocate
  else if (s2_size > s1->size) {
    REALLOC_C(s1, s2_size);
  }

  // the string exists and is large enough
  memcpy(s1->str.cstr, s2, s2_size-1);
  s1->str.cstr[s2_size-1] = '\0';
    
  return 0;
} // end cstrncpy_m

// wstrncpy_m
//   Inputs
//      s2 - Literal string wchar_t *
//      nchar - maximum number of characters to copy
//   Outputs
//      s1 - struct of destination string
//          s->size - Any nonzero size is valid
//          s->str  - Any string is valid ("" is valid, NULL is not valid)
errno_t
wstrncpy_m(string_m s1, const wchar_t *s2, size_t nchar) {
  size_t s2_size;
  errno_t rv;
  //validate arguments
  if (!s1){
    ErrorHandler("wstrncpy_m: 1st Argument NULL Pointer", s1, EINVAL);
    ERROR(EINVAL);
  }
  if (!s2){
    ErrorHandler("wstrncpy_m: 2nd Arguemnt NULL Pointer", s2, EINVAL);
    ERROR(EINVAL);
  }

  if (s1->strtype == STRTYPE_NTBS) {
    char *ostr = s1->str.cstr;
    const size_t osize = s1->size;

    s1->str.cstr = NULL;
    s1->size = 0;

    rv = str2wstr_m(s1);
    if (rv) {
      s1->str.cstr = ostr;
      s1->size = osize;
      ERROR(rv);
    }

    free(ostr);
  }
  if (s1->strtype != STRTYPE_WSTR){
    ErrorHandler("wstrncpy_m: 1st Argument Invalid String Type", s1, EINVAL);	
    ERROR(EINVAL);
  }

  s2_size=wcslen(s2)+1;
  if (s2_size > nchar) s2_size = nchar+1;

  if (s1->maxsize && s1->maxsize < s2_size) {
    ErrorHandler("wstrncpy_m: String Too Large", s2, EINVAL);
    ERROR(EINVAL);
  }

  // if destination string s1 has charset defined make sure
  // all characters in source are valid
  if ( (s1->charset.wstr) && (wcsspn(s2, s1->charset.wstr) < (s2_size-1)) ){
    ErrorHandler("wstrncpy_m: Charset Invalidates String To Be Copied", s2, EINVAL);
    ERROR(EINVAL);
  }

  // else if the existing string is too small, reallocate
  if (s2_size > s1->size) {
    REALLOC_W(s1, s2_size);
  }

  // the string exists and is large enough
  memcpy(s1->str.wstr, s2, (s2_size-1) * sizeof(wchar_t));
  s1->str.wstr[s2_size-1] = L'\0';
    
  return 0;

} // end strncpy_m

