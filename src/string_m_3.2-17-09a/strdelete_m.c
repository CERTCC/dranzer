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
#ifdef unix
#include <string.h>
#endif

#include "string_m.h"
#include "string_m_internals.h"

// strdelete_m
//   Inputs
//      s - input string ("" is valid)
//          s->size - Any size is valid
//          s->str  - Any string is valid ("" is valid, NULL is valid)
//   Outputs
//      none
//   Return
//      status - 0=OK, -1=error
int strdelete_m(string_m *s) {
  // validate s
  if (s == NULL){
    ErrorHandler("strdelete_m: 1st Argument NULL Pointer", s, EINVAL);
    ERROR(EINVAL);
  }
  if (*s == NULL) return 0;

  // if valid, free s->str 
  // NOTE: Could zero out the memory before a free
  if ((*s)->strtype == STRTYPE_NTBS) {
    if ((*s)->str.cstr != NULL) {
      free((*s)->str.cstr);
      if (errno != 0) {
	ErrorHandler("strdelete_m: Free Error", s, errno);	
	ERROR(errno);
      }
      (*s)->str.cstr = NULL;
      (*s)->size = 0;
    }
    if ((*s)->charset.cstr != NULL) {
      free((*s)->charset.cstr);
      if (errno != 0){
	ErrorHandler("strdelete_m: Free Error", s, errno);	
	ERROR(errno);
      }
      (*s)->charset.cstr = NULL;
      (*s)->size = 0;
    }
  } else if ((*s)->strtype == STRTYPE_WSTR) {
    if ((*s)->str.wstr != NULL) {
      free((*s)->str.wstr);
      if (errno != 0){
	ErrorHandler("strdelete_m: Free Error", s, errno);	
	ERROR(errno);
      }
      (*s)->str.wstr = NULL;
      (*s)->size = 0;
    }
    if ((*s)->charset.wstr != NULL) {
      free((*s)->charset.wstr);
      if (errno != 0){
	ErrorHandler("strdelete_m: Free Error", s, errno);	
	ERROR(errno);
      }
      (*s)->charset.wstr = NULL;
      (*s)->size = 0;
    }
  } else{
    ErrorHandler("strdelete_m: 1st Argument Invalid String Type", s, EINVAL);
    ERROR(EINVAL);
  }

  // Free the memory
  // NOTE: Could zero out the memory before a free
  free(*s);
  *s = NULL;

  return 0;
} // end strdelete_m
