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

#include "string_m.h"
#include "string_m_internals.h"
#include "scanf_internals.h"

errno_t vsscanf_m(string_m buf, const string_m format, int *count, va_list args){
  char *str;
  char *fmt;
  char *next;
  char digit;
  int num = 0;
  int qualifier;
  int base;
  int field_width;
  int is_sign = 0;
  errno_t rv;
  size_t rz;
  char *free_str = NULL, *free_fmt = NULL;

  /* runtime constraint checks */
  if (!buf){
    ErrorHandler("vsscanf_m: 1st Argument NULL Pointer", buf, EINVAL);
    ERROR(EINVAL);
  }
  if (!format){
    ErrorHandler("vsscanf_m: 2nd Argument NULL Pointer", format, EINVAL);
    ERROR(EINVAL);
  }

  /* get the strings from the string_m objects */
  if (buf->strtype == STRTYPE_WSTR){
    str = (char *)calloc(wcslen(buf->str.wstr)+1, sizeof(char));
    if (!str) ERROR(ENOMEM);
    free_str = str;
    rz = wcstombs(str, buf->str.wstr, wcslen(buf->str.wstr)+1);
    if (rz == (size_t) -1) ERROR(EINVAL);
  } else if (buf->strtype == STRTYPE_NTBS){
    rv = cgetstr_m(buf, &str);
    if (rv) ERROR(rv);
    free_str = str;
  } else {
    ErrorHandler("vsscanf_m: 1st Argument Invalid String Type", buf, EINVAL);
    ERROR(EINVAL);
  }

  if (format->strtype == STRTYPE_WSTR){
    fmt = (char *)calloc(wcslen(format->str.wstr)+1, sizeof(char));
    if (!fmt) ERROR(ENOMEM);
    free_fmt = fmt;
    rz = wcstombs(fmt, format->str.wstr, wcslen(format->str.wstr)+1);
    if (rz == (size_t) -1) ERROR(EINVAL);
  } else if (format->strtype == STRTYPE_NTBS){
    rv = cgetstr_m(format, &fmt);
    if (rv) {
      free( fmt);
      ERROR(rv);
    }
    free_fmt = fmt;
  } else {
    ErrorHandler("vsscanf_m: 2nd Argument Invalid String Type", format, EINVAL);
    ERROR(EINVAL);
  }

  while(*fmt && *str){
    /* skip leading whitespace */
    /* whitespace in the format string matches
     * any ammount of whitespace in the input.
     * including none.
     */
    if (isspace(*fmt)){
      while(isspace(*fmt))
	++fmt;
      while(isspace(*str))
	++str;
    }

    /* anything that is not a conversion must match exactly */
    if (*fmt != '%' && *fmt){
      if (*fmt++ != *str++){
	ErrorHandler("vsscanf_m: Mismatch error, format and input do not match", fmt, EINVAL);
	ERROR(EINVAL);
      }
      continue;
    }

    /* nothing left in the format string, we are done */
    if (!*fmt)
      break;
		
    /* *fmt = '%', move on to get the qualifiers */
    ++fmt;

    /* skip this conversion */
    if (*fmt == '*'){
      while(!isspace(*fmt) && *fmt)
	++fmt;
      while(!isspace(*str) && *str)
	++str;
      continue;
    }

    /* get the field width */
    field_width = -1;
    if (isdigit(*fmt))
      field_width = skip_atoi((const char **)&fmt);

    /* get the conversion qualifiers */
    qualifier = -1;
    if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'Z' || *fmt == 'z'){
      qualifier = *fmt++;

      if (qualifier == *fmt){
	if (qualifier == 'h'){
	  qualifier = 'H';
	  ++fmt;
	} else if (qualifier == 'l'){
	  qualifier = 'L';
	  fmt++;
	}
      }
    }

    base = 10;
    is_sign = 0;

    if (!*fmt || !*str)
      break;

    switch(*fmt++){
    case 'c':
      {
	if (field_width == -1)
	  field_width = 1;

	/* skip leading whitespace in the buffer */
	while(isspace(*str))
	  ++str;

	if (qualifier == 'l'){
	  wchar_t *c = (wchar_t *)va_arg(args, wchar_t*);
	  do{
	    rv = mbtowc(c, str,1);
	    if (rv == -1) ERROR(rv);
	    c++; str++;
	  }while(--field_width >0 && *str);
	} else if (qualifier == 'h'){
	  char *c = (char *)va_arg(args, char *);
	  do{
	    *c++ = *str++;
	  } while(--field_width > 0 && *str);
	} else {
	  if (buf->strtype == STRTYPE_NTBS){
	    char *c = (char *)va_arg(args, char *);
	    do{ 
	      *c++ = *str++;
	    } while(--field_width > 0 && *str);
	  } else if (buf->strtype == STRTYPE_WSTR){
	    wchar_t *c = (wchar_t *)va_arg(args, wchar_t *);
	    do{
	      rv = mbtowc(c, str, 1);
	      if (rv == -1) ERROR(rv);
	      c++; str++;
	    }while(--field_width > 0 && *str);
	  }
	}
	num++;
      }
      continue;

    case 's':
      {
	string_m *s = (string_m *)va_arg(args, string_m *);
	char *tmp, *pstr;
	size_t size = 0;

	while(isspace(*str)) ++str;

	if (field_width == -1)
	  field_width = INT_MAX;

	/* get the size of the input string */
	pstr = str;
	while(*pstr && !isspace(*pstr++) && size < (size_t )field_width) size++;

	tmp = (char *)calloc(size + 1, sizeof(char));
	if (!tmp) ERROR(ENOMEM);

	pstr = tmp;
	while(!isspace(*str) && field_width-- > 0 && *str)
	  *pstr++ = *str++;

	*pstr = '\0';
	pstr = NULL;
	if ((buf->strtype == STRTYPE_WSTR && qualifier != 'h')|| qualifier == 'l'){
	  wchar_t *wtmp = (wchar_t *)calloc(strlen(tmp)+1, sizeof(wchar_t));
	  if (!wtmp) ERROR(ENOMEM);
	  rz = mbstowcs(wtmp, tmp, strlen(tmp)+1);
	  if (rz == (size_t) -1) ERROR(EINVAL);
	  rv = wstrcpy_m(*s, wtmp);
	  free(wtmp);
	  free(tmp);
	  if (rv) ERROR(rv);
	} else {
	  rv = cstrcpy_m(*s, tmp);
	  free(tmp);
	  if (rv) ERROR(rv);
	}

      }
      continue;
    case 'o':
      base = 8;
      break;
    case 'x':
    case 'X':
      base = 16;
      break;

    case 'i':
      base = 0;
    case 'd':
      is_sign = 1;
    case 'u':
      break;

    case '%':
      /* expect % in the input string */
      if (*str++ != '%'){
	/* set count to number already converted if not null */
	if (count) *count = num;
	ErrorHandler("vsscanf_m: Mismatch Error. Expected '%'", str, EINVAL);
	ERROR(EINVAL);
      }
      break;

    default:
      /* invalid conversion specifier; set count to number already converted
       * and quit.
       */
      if (count) *count = num;
      ErrorHandler("vsscanf_m: Invalid conversion specifier", fmt, EINVAL);
      ERROR(EINVAL);
    }/* end switch */

    /* have numerical conversion */
    /* skip leading whitespace */
    while(isspace(*str)) ++str;

    digit = *str;
    if (is_sign && digit == '-')
      digit = *(str+1);

    if (!digit || (base == 16 && !isxdigit(digit))
	|| (base == 10 && !isdigit(digit))
	|| (base == 8 && (!isdigit(digit) || digit > '7'))
	|| (base == 0 && !isdigit(digit))){
      if (count) *count = num;
      ErrorHandler("vsscanf_m: Conversion mismatch error", str, EINVAL);
      ERROR(EINVAL);
    }

    switch(qualifier){
      /* qualifier is hh */
    case 'H':	
      if (is_sign){
	signed char *s = (signed char *)va_arg(args, signed char *);
	*s = (signed char)simple_strtol(str, &next, base, field_width);
      } else {
	unsigned char *s = (unsigned char *)va_arg(args, unsigned char *);
	*s = (unsigned char)simple_strtoul(str, &next, base, field_width);
      }
      break;
    case 'h':
      if (is_sign){
	short *s = (short *)va_arg(args, short *);
	*s = (short)simple_strtol(str, &next, base, field_width);
      } else {
	unsigned short *s = (unsigned short *)va_arg(args, unsigned short *);
	*s = (unsigned short)simple_strtoul(str, &next, base, field_width);
      }
      break;
    case 'l':
      if (is_sign){
	long *l = (long *)va_arg(args, long *);
	*l = simple_strtol(str, &next, base, field_width);
      } else {
	unsigned long *l = (unsigned long *)va_arg(args, unsigned long *);
	*l = simple_strtoul(str, &next, base, field_width);
      }
      break;
    case 'L':
      if (is_sign){
	long long *l = (long long *)va_arg(args, long long *);
	*l = simple_strtoll(str, &next, base, field_width);
      } else {
	unsigned long long *l = (unsigned long long *)va_arg(args, unsigned long long *);
	*l = simple_strtoull(str, &next, base, field_width);
      }
      break;
    case 'Z':
    case 'z':
      {
	size_t *s = (size_t *)va_arg(args, size_t *);
	*s = (size_t)simple_strtoull(str, &next, base, field_width);
      }
      break;
    default:
      if (is_sign){
	int *i = (int *)va_arg(args, int *);
	*i = (int)simple_strtol(str, &next, base, field_width);
      } else {
	unsigned int *i = (unsigned int *)va_arg(args, unsigned int *);
	*i = (unsigned int)simple_strtoul(str, &next, base,field_width);
      }
      break;
    }/* end switch statement */
    num++;
    if (!next)
      break;
    str = next;
  } /* end while-loop */

  if (count) *count = num;
  free( free_str);
  free( free_fmt);
  return 0;
} /* end vsscanf_m */



errno_t
sscanf_m(string_m buf, const string_m format, int *count, ...){
  va_list ap;
  errno_t rv;

  va_start(ap, count);
  rv = vsscanf_m(buf, format, count, ap);
  va_end(ap);
  return rv;
}
