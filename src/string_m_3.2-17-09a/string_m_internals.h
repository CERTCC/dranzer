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
#ifndef __STRING_M_INTERNAL_
#define __STRING_M_INTERNAL_ 1

#define STRTYPE_NTBS	1
#define STRTYPE_WSTR	2

union str_union_t {
  char *cstr;
  wchar_t *wstr;
};

struct string_mx {
  size_t size;  // This is the size of the string allocation (includes any NULL)
  // Note that size is the number of characters, not the number of bytes
  // For strtype == STRTYPE_WSTR, you must multiply size by sizeof(wchar_t)
  // to compute the number of bytes

  size_t maxsize;  // maximum size of string
  unsigned char strtype;

  union str_union_t charset; // set of valid characters
  union str_union_t str; // the real string
};

#define REALLOC_C(S,L) {						\
    (S)->size = (L);							\
    (S)->str.cstr = (char*)realloc((S)->str.cstr, sizeof(char)*((L)+1)); \
    if (!(S)->str.cstr) { (S)->size = 0; ERROR(ENOMEM); }		\
  }

#define REALLOC_W(S,L) {						\
    (S)->size = (L);							\
    (S)->str.wstr = (wchar_t*)realloc((S)->str.wstr, sizeof(wchar_t)*((L)+1)); \
    if (!(S)->str.wstr) { (S)->size = 0; ERROR(ENOMEM); }		\
  }


#define ERROR(C) return (C)

#ifdef _MSC_VER
#define VA_COPY( dest, src) memcpy( &dest, &src, sizeof( va_list))
#else
#define VA_COPY( dest, src) va_copy( dest, src)
#endif // unix


extern int makestr(string_m s, const char *cstr);
extern int wmakestr(string_m s, const wchar_t *cstr);

/* str2wstr_m is guaranteed to make no revision to the string unless it
   succeeds */
extern int str2wstr_m(string_m s);

#endif /* __STRING_M_INTERNAL_ */
