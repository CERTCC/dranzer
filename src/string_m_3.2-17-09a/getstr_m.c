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

// returns a copy of the string from s.  the user must free this storage
errno_t cgetstr_m(const string_m s, char **string) {
#undef FUNC
#define FUNC "getstr_m"

  size_t len;

  if (string == NULL){
    ErrorHandler("cgetstr_m: 2nd Argument NULL ptr", string, EINVAL);	
    ERROR(EINVAL);
  }
  *string = NULL;

  // validate s
  if (s == NULL){
    ErrorHandler("cgetstr_m: 1st Argument NULL ptr", s, EINVAL);	
    ERROR(EINVAL);
  }
  if (s->strtype != STRTYPE_NTBS){
    ErrorHandler("cgetstr_m: 1st Argument Invalid String Type", s, EINVAL);	
    ERROR(EINVAL);
  }

  // either string has zero size and null pointer or it doesn't
  if ( (!s->str.cstr && s->size) && (s->str.cstr && !s->size) ){
    ErrorHandler("cgetstr_m: 1st Argument size mismatch", s, EINVAL);
    ERROR(EINVAL);
  }


  // if null string return NULL
  if (!s->str.cstr) {
    *string = NULL;
    return 0;
  }
  // string is not null
  else {
    // get the length of the string to copy
    if (strlen_m(s, &len)){	
      ERROR(EINVAL);
    }

    *string = (char *)calloc(len + 1, sizeof(char));
    if (!(*string)){
      ErrorHandler("cgetstr_m: Calloc Error", string, ENOMEM);	
      ERROR(ENOMEM);
    }
    memcpy(*string, s->str.cstr, len); 
  }
  return 0;    
}

// returns a copy of the string from s.  the user must free this storage
errno_t wgetstr_m(const string_m s, wchar_t **string) {
#undef FUNC
#define FUNC "wgetstr_m"

  size_t len;


  if (string == NULL) {
    ErrorHandler("wgetstr_m: 2nd Argument NULL pointer", string, EINVAL);
    ERROR(EINVAL);
  }
  *string = NULL;

  // validate s
  if (s == NULL){
    ErrorHandler("wgetstr_m: 1st Argument NULL pointer", s, EINVAL);	
    ERROR(EINVAL);
  }

  if (s->strtype == STRTYPE_WSTR) {
    // either string has zero size and null pointer or it doesn't
    if ( (!s->str.wstr && s->size) && (s->str.wstr && !s->size) ){
      ErrorHandler("wgetstr_m: 1st Argument size mismatch", s, EINVAL);
      ERROR(EINVAL);
    }

    // if null string return NULL
    if (!s->str.wstr) {
      *string = NULL;
      return 0;
    }

    // string is not null

    // get the length of the string to copy
    if (strlen_m(s, &len)){	
      ERROR(EINVAL);
    }

    *string = (wchar_t *)calloc(len + 1, sizeof(wchar_t));
    if (!(*string)){
      ErrorHandler("wgetstr_m: Calloc Error", string, ENOMEM);	
      ERROR(ENOMEM);
    }

    memcpy(*string, s->str.wstr, len * sizeof(wchar_t)); 
    return 0;    
  } else if (s->strtype == STRTYPE_NTBS) {
    // either string has zero size and null pointer or it doesn't
    if ( (!s->str.cstr && s->size) && (s->str.cstr && !s->size) ){
      ErrorHandler("wgetstr_m: 1st Argument Size Mismatch", s, EINVAL);
      ERROR(EINVAL);
    }

    // if null string return NULL
    if (!s->str.cstr) {
      *string = NULL;
      return 0;
    }
    // string is not null

    // get the length of the string to copy
    if (strlen_m(s, &len)) ERROR(EINVAL);

    *string = (wchar_t *)calloc(len + 1, sizeof(wchar_t));
    if (!(*string)){
      ErrorHandler("wgetstr_m: Calloc Error", string, ENOMEM);	
      ERROR(ENOMEM);
    }


    if (mbstowcs(*string, s->str.cstr, len) == (size_t) -1) ERROR(EINVAL);

    return 0;    
  } else {
    ErrorHandler("wgetstr_m: 1st Argument Invalid String Type", s, EINVAL);
    ERROR(EINVAL);
  }
}

char * getstrptr_m(const string_m s) 
{
  if (s==NULL) return NULL;
  else if ( (!s->str.cstr && s->size) && (s->str.cstr && !s->size) ) return(NULL);
  else return(s->str.cstr);
}
