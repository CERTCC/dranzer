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

size_t __MAX_SYS_DEFAULT_STR_SIZE = 2048;

// strcreate_m
//   Inputs
//      s1 - pointer to store returned string_m
//      s2 - string to duplicate
//   Outputs
//      *s1 - new string-m
//   Return
//      string_m - NULL=error, other=OK
errno_t strdup_m(string_m *s1, const string_m s2){
  errno_t rv;

  if (s2 == NULL){
    ErrorHandler("strdup_m: 2nd Argument NULL Pointer", s2, EINVAL);	
    ERROR(EINVAL);
  }

  if (s2->strtype != STRTYPE_NTBS && s2->strtype != STRTYPE_WSTR){
    ErrorHandler("strdup_m: 2nd Argument Invalid String Type", s2, EINVAL);
    ERROR(EINVAL);
  }

  *s1 = malloc(sizeof(struct string_mx));
  if (!(*s1)){
    ErrorHandler("strdup_m: Malloc Error", s1, ENOMEM);	
    ERROR(ENOMEM);
  }

  if (s2->strtype == STRTYPE_NTBS){
    rv = makestr(*s1, s2->str.cstr);
  } else {
    rv = wmakestr(*s1, s2->str.wstr);
  }
  if (rv) {
    free(*s1);
    ERROR(rv);
  }

  /* copy charset */
  if(s2->strtype == STRTYPE_NTBS){
    if(s2->charset.cstr)
      (*s1)->charset.cstr = strdup(s2->charset.cstr);
    else
      (*s1)->charset.cstr = NULL;
  } else {
    if(s2->charset.wstr)
      (*s1)->charset.wstr = wcsdup(s2->charset.wstr);	
    else
      (*s1)->charset.wstr = NULL;
  }

  /* copy the maxsize */
  (*s1)->maxsize = s2->maxsize;
  return 0;
}

// strcreate_m
//   Arguments
//      s - address of new string
//      cstr - c style initialization string (NULL and "" are valid)
//      size_t - specifies the maximum size the string can obtain.  0 means system default
//      charset - set of valid characters.  NULL means all characters valid.
//   Return
//      string_m - NULL=error, other=OK
errno_t strcreate_m(string_m *s, const char *cstr, const size_t maxsize, const char *charset) {
  if (!s){
    ErrorHandler("strcreate_m: 1st Argument NULL Pointer", s, EINVAL);	
    ERROR(EINVAL);
  }

  if(charset && strlen(charset) == 0){ 
    ErrorHandler("strcreate_m: 4th Argument May Not Be An Empty String", charset, EINVAL);
    ERROR(EINVAL);
  }
  *s = (string_m)calloc(sizeof(struct string_mx), sizeof(char));
  if (!(*s)){
    ErrorHandler("strcreate_m: Calloc Error", s, ENOMEM);	
    ERROR(ENOMEM);
  }

  // Input string null
  if (cstr == NULL) {
    (*s)->size = 0;
    (*s)->strtype = STRTYPE_NTBS;
    (*s)->str.cstr = NULL;

  }
  // else input string not null allocate memory
  else {
    // if a valid set of chars is provided, make sure input is valid
    if (charset) {

      // if initialization string contains invalid characters exit
      if (strspn(cstr, charset) != strlen(cstr)) {
	free(*s);
	*s = NULL;
	ErrorHandler("strcreate_m: Charset Invalidates String", s, EINVAL);
	return EINVAL;
      }
    } // end if charset not null    
    if (makestr(*s, cstr) != 0) {
      ErrorHandler("strcreate_m: String Creation Failed", s, EINVAL);
      free(*s);
      ERROR(EINVAL);
    }
  }

  if (maxsize && maxsize < (*s)->size) {
    ErrorHandler("cstrcreate_m: String Too Large", s, EINVAL);
    ERROR(EINVAL);
  }

  (*s)->maxsize = maxsize;
  if (charset) (*s)->charset.cstr = strdup(charset);
  else (*s)->charset.cstr = NULL;
       
  return 0;
}

// wstrcreate_m
//   Arguments
//      s - address of new string
//      cstr - c style initialization string (NULL and "" are valid)
//      size_t - specifies the maximum size the string can obtain.  0 means system default
//      charset - set of valid characters.  NULL means all characters valid.
//   Return
//      string_m - NULL=error, other=OK
errno_t wstrcreate_m(string_m *s, const wchar_t *wstr, const size_t maxsize, const wchar_t *charset) {
  errno_t rv;

  if (!s){
    ErrorHandler("wstrcreate_m: 1st Argument NULL Pointer", s, EINVAL);
    ERROR(EINVAL);
  }

  if(charset && wcslen(charset) == 0){
    ErrorHandler("wstrcreate_m: 4th Argument May Not Be An Empty String", charset, EINVAL);
    ERROR(EINVAL);
  }
  *s = (string_m)calloc(1, sizeof(struct string_mx));
  if (!(*s)){
    ErrorHandler("wstrcreate_m: Calloc Error", s, ENOMEM);	
    ERROR(ENOMEM);
  }

  // Input string null
  if (wstr == NULL) {
    (*s)->size = 0;
    (*s)->strtype = STRTYPE_WSTR;
    (*s)->str.wstr = NULL;
  }
  else {

    // if a valid set of chars is provided, make sure input is valid
    if (charset) {
      // if initialization string contains invalid characters exit
      if (wcsspn(wstr, charset) != wcslen(wstr)) {
	free(*s);
	*s = NULL;
	ErrorHandler("wstrcreate_m: Charset Invalidates String", s, EINVAL);
	ERROR(EINVAL);
      }
    } // end if charset not null

    rv = wmakestr(*s, wstr);
    if (rv) {
      free(*s);
      ERROR(rv);
    }
  }
  if (maxsize && maxsize < (*s)->size) {
    ErrorHandler("wstrcreate_m: String Too Large", s, EINVAL);
    ERROR(EINVAL);
  }

  (*s)->maxsize = maxsize;
  if (charset) {
    const size_t nchar = wcslen(charset);
    (*s)->charset.wstr = (wchar_t*) malloc((nchar+1)*sizeof(wchar_t));
    if (!((*s)->charset.wstr)) {
      free((*s)->str.wstr);
      free(*s);
      ErrorHandler("wstrcreate_m: Malloc Error", s, ENOMEM);
      ERROR(ENOMEM);
    }
    (void) wcsncpy((*s)->charset.wstr, charset, nchar+1);
    (*s)->charset.wstr[nchar] = L'\0';
  } else (*s)->charset.wstr = NULL;
       
  return 0;
}
