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


#include <wchar.h>

#ifdef _MSC_VER
#else
wchar_t *wcsdup(const wchar_t *str);
char *strdup(const char *str);
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "stdbool.h"
#include "constraint.h"

#ifdef  __cplusplus
extern "C" {
#endif


  struct string_mx;

  typedef struct string_mx *string_m;

  /* isnull_m */
  extern errno_t isnull_m(const string_m s, _Bool *nullstr);

  /* isempty_m */
  extern errno_t isempty_m(const string_m s, _Bool *emptystr);

  /* isntbs_m */
  extern errno_t isntbs_m(const string_m s, _Bool *isntbsr);

  /* iswide_m */
  extern errno_t iswide_m(const string_m s, _Bool *widestr);


  /* strcreate_m
   *   Arguments
   *       s - address of new string
   *       cstr - c style initialization string (NULL and "" are valid)
   *       size_t - specifies the maximum size the string can obtain.  0 means system default
   *       charset - set of valid characters.  NULL means all characters valid.
   *   Return
   *      string_m - NULL=error, other=OK
   */

  extern errno_t strcreate_m(string_m *s, const char *cstr, const size_t maxsize, const char *charset);
  extern errno_t wstrcreate_m(string_m *s, const wchar_t *cstr, const size_t maxsize, const wchar_t *charset);

  /* strdelete_m
   *   Inputs
   *      s - input string ("" is valid)
   *          s->size - Any size is valid
   *          s->str  - Any string is valid ("" is valid, NULL is valid)
   *   Outputs
   *      none
   *   Return
   *      status - 0=OK, -1=error
   */
  extern errno_t strdelete_m(string_m *s);

  /* strlen_m
   *   Inputs
   *      s - struct of string
   *          s->size - Any nonzero size is valid
   *          s->str  - Any string is valid ("" is valid, NULL is not valid)
   *   Outputs
   *      size - length of input string
   *   Return
   *      status - 0=OK, -1=error
   */
  extern errno_t strlen_m(const string_m s, size_t *size);

  /* returns a copy of the string from s.  the user must free this storage */
  extern int cgetstr_m(const string_m s, char **string);

  /* returns a copy of the string from s.  the user must free this storage */
  /* Note: if s is a NTBS, string will be the conversion of that into a
     wchar_t.  s will remain a wchar */
  extern int wgetstr_m(const string_m s, wchar_t **string);

  extern char * getstrptr_m(const string_m s);

  /* strcpy_m
   *   Inputs
   *      s2 - struct of source string
   *          s->size - Any nonzero size is valid
   *          s->str  - Any string is valid ("" is valid, NULL is not valid)
   *   Outputs
   *      s1 - struct of destination string
   *          s->size - Any nonzero size is valid
   *          s->str  - Any string is valid ("" is valid, NULL is not valid)
   *   Return
   *      status - 0=OK, -1=error
   */
  extern errno_t strcpy_m(string_m s1, const string_m s2) ;

  extern errno_t cstrcpy_m(string_m s1, const char *s2);
  extern errno_t wstrcpy_m(string_m s1, const wchar_t *s2);

  extern errno_t strncpy_m(string_m s1, const string_m s2, size_t nchar);
  extern errno_t cstrncpy_m(string_m s1, const char *s2, size_t nchar);
  extern errno_t wstrncpy_m(string_m s1, const wchar_t *s2, size_t nchar);

  /* setcharset_m */

  extern errno_t setcharset_m(string_m s, const string_m charset) ;
  extern errno_t strtok_m(string_m token, string_m str, const string_m delim, string_m ptr);
  extern errno_t cstrchr_m(string_m out, const string_m str, char c);
  extern errno_t wstrchr_m(string_m out, const string_m str, wchar_t c);

  extern errno_t strspn_m(const string_m str, const string_m accept, rsize_t *len);
  extern errno_t cstrspn_m(const string_m str, const char *accept, rsize_t *len);
  extern errno_t wstrspn_m(const string_m str, const wchar_t *accept, rsize_t *len);

  extern errno_t strcspn_m(const string_m str, const string_m accept, rsize_t *len);
  extern errno_t cstrcspn_m(const string_m str, const char *accept, rsize_t *len);
  extern errno_t wstrcspn_m(const string_m str, const wchar_t *accept, rsize_t *len);

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
  extern errno_t strcat_m(string_m s1, const string_m s2) ;

  extern errno_t cstrcat_m(string_m s1, const char *s2);
  extern errno_t wstrcat_m(string_m s1, const wchar_t *s2);

  extern errno_t strncat_m(string_m s1, const string_m s2, rsize_t nchar);

  extern errno_t cstrncat_m(string_m s1, const char *s2, rsize_t nchar);
  extern errno_t wstrncat_m(string_m s1, const wchar_t *s2, rsize_t nchar);

  extern errno_t sprintf_m(string_m buf, const string_m fmt, int *, ...);

  /*
   * vsprintf_m() - managed string version of C99 vsprintf() function.
   * ??? need to convert function so that string args are string_m
   */
  extern errno_t vsprintf_m(string_m s, const string_m format, int *, va_list args);

  extern errno_t fprintf_m(FILE *file, const string_m format, int *count, ...);
  extern errno_t vfprintf_m(FILE *f, const string_m fmt, int *count, va_list args);

  extern errno_t printf_m(const string_m format, int *count, ...);
  extern errno_t vprintf_m(const string_m format, int *count, va_list args);

  extern errno_t sscanf_m(string_m buf, const string_m fmt, int *count, ...);
  extern errno_t vsscanf_m(string_m buf, const string_m fmt, int *count, va_list args);

  extern errno_t fscanf_m(FILE *file, const string_m format, int *count, ...);
  extern errno_t vfscanf_m(FILE *file, const string_m format, int *count, va_list args);

  extern errno_t scanf_m(const string_m format, int *count, ...);
  extern errno_t vscanf_m(const string_m format, int *count, va_list args);

  extern errno_t strdup_m(string_m *o, const string_m i);

  extern errno_t strcmp_m(const string_m s1, const string_m s2, int *cmp);
  extern errno_t cstrcmp_m(const string_m s1, const char *cstr, int *cmp);
  extern errno_t wstrcmp_m(const string_m s1, const wchar_t *wstr, int *cmp);

  extern errno_t strncmp_m(const string_m s1, const string_m s2, rsize_t nchar, int *cmp);
  extern errno_t cstrncmp_m(const string_m s1, const char *cstr, rsize_t nchar, int *cmp);
  extern errno_t wstrncmp_m(const string_m s1, const wchar_t *wstr, rsize_t nchar, int *cmp);

  extern errno_t strslice_m(string_m s1, string_m s2, rsize_t offset, rsize_t len);
  extern errno_t strleft_m(string_m s1, string_m s2, rsize_t len);

  extern errno_t cchar_m(string_m s, rsize_t offset, char *c);
  extern errno_t wchar_m(string_m s, rsize_t offset, wchar_t *w);

#ifdef  __cplusplus
}
#endif
