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

#include <ctype.h>
#include <wchar.h>
#include <wctype.h>

#include <string.h>

#include <stdio.h>

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t) -1)
#endif

static int skip_atoi(const char **s){
  int i = 0;

  while(isdigit(**s))
    i = i*10 + *((*s)++) - '0';
  return i;
}

static int wskip_atoi(const wchar_t **s){
  int i=0;

  while(iswdigit(**s))
    i = i*10 + *((*s)++) - L'0';
  return i;
}

static int streamtombs(FILE *file, char **str, size_t field_width){
  size_t size = 0, alloc_size = 20;
  char *pstr;
  int c;

  (*str) = (char *)calloc(alloc_size, sizeof(char));
  if (!*str) ERROR(ENOMEM);

  pstr = *str;
  while( !isspace( (c=getc(file)) ) &&!feof(file) && field_width-- > 0  ){
    if (size++ >= alloc_size){
      alloc_size *= 2;
      *str = (char *)realloc(*str, alloc_size);
      if (!*str) ERROR(ENOMEM);
    }

    *pstr = (unsigned char)c;
    pstr++;
  }
  ungetc(c, file);
  return 0;
}

static int wstreamtombs(FILE *file, wchar_t **str, size_t field_width){
  size_t size = 0, alloc_size = 20;
  wchar_t *pstr;
  wint_t c = WEOF;

  (*str) = (wchar_t *)calloc(alloc_size, sizeof(wchar_t));
  if (!*str) ERROR(ENOMEM);

  pstr = *str;
  while( !feof(file) && field_width-- > 0 && !iswspace( (c=getwc(file)) ) ){
    if (size++ >= alloc_size){
      alloc_size *= 2;
      *str = (wchar_t *)realloc(*str, alloc_size);
      if (!*str) ERROR(ENOMEM);
    }

    *pstr = c;
    pstr++;
  }
  ungetwc(c, file);
  return 0;
}

static unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base, size_t field_width){
  unsigned long result = 0, value;

  if (!base){
    base = 10;
    if (*cp == '0'){
      base = 8;
      cp++;
      if ((toupper(*cp) == 'X') && isxdigit(cp[1])){
	cp++;
	base = 16;
      }
    }
  } else if (base == 16){
    if (cp[0] == '0' && toupper(cp[1]) == 'X')
      cp+= 2;
  }
  while(isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : toupper(*cp)-'A'+10) < base && field_width-- > 0){
    result = result * base + value;
    cp++;
  }
  if (endp)
    *endp = (char *)cp;
  return result;
}

static unsigned long wsimple_strtoul(const wchar_t *cp, wchar_t **endp, unsigned int base, size_t field_width){
  unsigned long result = 0, value;

  if (!base){
    base = 10;
    if (*cp == L'0'){
      base = 8;
      cp++;
      if ((towupper(*cp) == L'X') && iswxdigit(cp[1])){
	cp++;
	base = 16;
      }
    }
  } else if (base == 16){
    if (cp[0] == L'0' && towupper(cp[1]) == L'X')
      cp+=2;
  }
  while(iswxdigit(*cp) && (value = iswdigit(*cp) ? *cp-L'0' : towupper(*cp)-L'A'+10) < base && field_width-- > 0){
    result = result * base + value;
    cp++;
  }
  if (endp)
    *endp = (wchar_t *)cp;
  return result;
}

static long simple_strtol(const char *cp, char **endp, unsigned int base, size_t field_width){
  if (*cp == '-')
    return - (long)simple_strtoul(cp+1, endp, base, field_width);
  return (long)simple_strtoul(cp, endp, base, field_width);
}

static long wsimple_strtol(const wchar_t *cp, wchar_t **endp, unsigned int base, size_t field_width){
  if (*cp == L'-')
    return - (long)wsimple_strtoul(cp+1, endp, base, field_width);
  return (long)wsimple_strtoul(cp, endp, base, field_width);
}

static unsigned long long simple_strtoull(const char *cp, char **endp, unsigned int base,size_t field_width){
  unsigned long long result = 0, value;

  if (!base){
    base = 10;
    if (*cp == '0'){
      base = 8;
      cp++;
      if ((toupper(*cp) == 'X') && isxdigit(cp[1])){
	cp++;
	base = 16;
      }
    }
  } else if (base == 16){
    if (cp[0] == '0' && toupper(cp[1]) == 'X')
      cp+= 2;
  }
  while(isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : toupper(*cp)-'A'+10) < base && field_width-- > 0){
    result = result * base + value;
    cp++;
  }
  if (endp)
    *endp = (char *)cp;
  return result;
}

static unsigned long long wsimple_strtoull(const wchar_t* cp, wchar_t **endp, unsigned int base, size_t field_width){
  unsigned long long result = 0, value;

  if (!base){
    base = 10;
    if (*cp == L'0'){
      base = 8;
      cp++;
      if ((towupper(*cp) == L'X') && iswxdigit(cp[1])){
	cp++;
	base = 16;
      }
    }
  } else if (base == 16){
    if (cp[0] == L'0' && towupper(cp[1]) == L'X')
      cp+=2;
  }
  while(iswxdigit(*cp) && (value = iswdigit(*cp) ? *cp-L'0' : towupper(*cp)-L'A'+10) < base && field_width-- > 0){
    result = result*base + value;
    cp++;
  }
  if (endp)
    *endp = (wchar_t *)cp;
  return result;
}

static signed long long simple_strtoll(const char *cp, char **endp, unsigned int base, size_t field_width){
  if (*cp == '-')
    return - (long long)simple_strtoull(cp+1, endp, base, field_width);
  return (long long)simple_strtoull(cp, endp, base, field_width);
}

static signed long long wsimple_strtoll(const wchar_t *cp, wchar_t **endp, unsigned int base, size_t field_width){
  if (*cp == L'-')
    return - (long long) wsimple_strtoull(cp+1, endp, base, field_width);
  return (long long)wsimple_strtoull(cp, endp, base, field_width);
}

static errno_t cvfscanf_m(FILE *file, const string_m format, int *count, va_list args){
  char *fmt;
  char *next;
  int inchr = WEOF;
  int qualifier,  num = 0;
  int base, is_sign;
  int digit;
  errno_t rv;
  char *str;
  size_t field_width;

  if (!format){
    ErrorHandler("vfscanf_m: 2nd Argument NULL Pointer", format, EINVAL);
    ERROR(EINVAL);
  }

  if (format->strtype == STRTYPE_NTBS){
    rv = cgetstr_m(format, &fmt);
    if (rv) ERROR(rv);
  } else if (format->strtype == STRTYPE_WSTR){
    fmt = (char *)calloc(wcslen(format->str.wstr)+1, sizeof(char));
    if (!fmt) ERROR(ENOMEM);
    wcstombs(fmt, format->str.wstr, wcslen(format->str.wstr)+1);
    //		wcstombs_s(&numconverted, fmt, wcslen(format->str.wstr)+1, format->str.wstr, wcslen(format->str.wstr)+1);
  } else {
    ErrorHandler("vfscanf_m: 2nd Argument Invalid String Type", format, EINVAL);
    ERROR(EINVAL);
  }

  while (*fmt && !feof(file)){
    /* skip leading whitespace */
    if (isspace(*fmt)){
      while (*fmt &&isspace(*fmt)) ++fmt;
      while (!feof(file) && (isspace((inchr = getc(file))) || inchr == '\n'));
      if(inchr != WEOF)
	ungetc(inchr, file);
    }

    /* must match exactly if not conversion */
    if ( *fmt && *fmt != '%'){
      if (*fmt != (inchr = getc(file))){
	free(fmt);
	ErrorHandler("vfscanf_m: Mismatch error. Format string does not match input", format, EINVAL);
	ERROR(EINVAL);
      }
      continue;
    }

    if (!*fmt || feof(file))
      break;

    fmt++;

    /* skip this conversion */
    if (*fmt == '*'){
      while(!isspace(*fmt) && *fmt) ++fmt;
      while(!isspace((inchr = getc(file))) && !feof(file)) ;
      ungetc(inchr, file);
      continue;
    }

    /* get the field width */
    field_width = (size_t)-1;
    if (isdigit(*fmt)) {
      field_width = skip_atoi((const char **)&fmt);
    }

    /* get the qualifier */
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

    if (!*fmt || feof(file))
      break;
    switch(*fmt++){
    case 'c':
      {
	if (field_width == (size_t) -1) field_width = 1;

	/* skip leading whitespace */
	if (isspace((inchr = getc(file))) || inchr == '\n')
	  while(isspace((inchr = getc(file))) || inchr == '\n') ;
	ungetc(inchr, file);

	if (qualifier == 'l'){
	  wchar_t *c = (wchar_t *)va_arg(args, wchar_t *);
	  while(field_width-- > 0 && !feof(file)){
	    inchr = getc(file);
	    mbtowc(c++, (char *)&inchr, 1);
	  }
	} else {
	  char *c = (char *)va_arg(args, char *);
	  while(field_width-- > 0 && !feof(file)){
	    inchr = getc(file);
	    *c++ = (unsigned char)inchr;
	  }
	}
	num++;
      }
      continue;
    case 's':
      {
	string_m *s = (string_m *)va_arg(args, string_m *);
	char *tmp;
	size_t size = 0;
	size_t alloc_size = 20;


	if (field_width == (size_t) -1) field_width =  SIZE_MAX;

	/* skip leading whitespace */
	if (isspace((inchr = getc(file))) || inchr == '\n')
	  while(isspace((inchr = getc(file))) || inchr == '\n');
	ungetc(inchr, file);

	tmp = (char *)calloc( (field_width == SIZE_MAX) ? alloc_size : field_width , sizeof(char));
	if (!tmp) ERROR(ENOMEM);
	while(field_width-- > 0 && !isspace((inchr=getc(file))) && !feof(file)){
	  if (size > (*s)->maxsize){
	    ErrorHandler("vfscanf_m: Max size exceded for string", *s, EINVAL);
	    ERROR(EINVAL);
	  }
	  if ( (size+1) == alloc_size){
	    alloc_size *=2;
	    tmp = (char *)realloc(tmp, alloc_size);
	    if (!tmp) ERROR(ENOMEM);
	  }

	  tmp[size++] = (unsigned char)inchr;
	}
	ungetc(inchr, file);
	tmp[size] = '\0';

	if (qualifier == 'l'){
	  wchar_t *wtmp = (wchar_t *)calloc(strlen(tmp)+1, sizeof(wchar_t));
	  if (!wtmp) ERROR(ENOMEM);
	  mbstowcs(wtmp, tmp, strlen(tmp)+1);
	  rv = wstrcpy_m(*s, wtmp);
	  free(wtmp);
	  free(tmp);
	  if (rv) ERROR(rv);
	} else {
	  rv = cstrcpy_m(*s, tmp);
	  free(tmp);
	  if (rv) ERROR(rv);
	}
	num++;
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
      inchr = getc(file);
      if (inchr != '%'){
	ungetc(inchr, file);
	if (count) *count = num;
	ErrorHandler("vfscanf_m: Mismatch Error", fmt, EINVAL);
	ERROR(EINVAL);
      }
      break;
    default:
      if (count) *count = num;
      ErrorHandler("vfscanf_m: Invalid conversion", fmt, EINVAL);
      ERROR(EINVAL);
    } /* end switch */

    while(isspace((inchr = getc(file)))) ;
    ungetc(inchr, file);

    digit = getc(file);
    if (is_sign && digit == '-'){
      digit = getc(file);
      ungetc(digit, file);
      ungetc('-', file);
    } else {
      ungetc(digit, file);
    }

    if (!digit || (base == 16 && !isxdigit(digit))
	|| (base == 10 && !isdigit(digit))
	|| (base == 8 && (!isdigit(digit) || digit > '7'))
	|| (base == 0 && !isdigit(digit))){
      if (count) *count = num;
      ErrorHandler("vsscanf_m: Conversion mismatch error", file, EINVAL);
      ERROR(EINVAL);
    }


    /* get the input up to field_width character or until the first space */
    if (field_width == (size_t) -1 ) 
      field_width = SIZE_MAX;
    streamtombs(file, &str, field_width);
    switch(qualifier){

    case 'H':
      if (is_sign){
	signed char *c = (signed char *)va_arg(args, signed char *);
	*c = (signed char) simple_strtol(str, &next,base, field_width);
      } else {
	unsigned char *c = (unsigned char *)va_arg(args, unsigned char *);
	*c = (unsigned char)simple_strtoul(str, &next, base, field_width);
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
    }
    num++;

  } /* end while-loop */

  if (count) *count = num;
  return 0;
}


static errno_t wvfscanf_m(FILE *file, const string_m format, int *count, va_list args){
  wchar_t *fmt;
  wchar_t *next;
  wint_t inchr = WEOF;
  int qualifier,  num = 0;
  int base, is_sign;
  wchar_t digit;
  errno_t rv;
  wchar_t *str;
  size_t field_width;
  if (!format){
    ErrorHandler("vfscanf_m: 2nd Argument NULL Pointer", format, EINVAL);
    ERROR(EINVAL);
  }

  if (format->strtype == STRTYPE_NTBS){
    rv = str2wstr_m(format);
    if (rv) ERROR(rv);
  } else {
    ErrorHandler("vfscanf_m: 2nd Argument Invalid String Type", format, EINVAL);
    ERROR(EINVAL);
  }
  
  rv = wgetstr_m(format, &fmt);
  if (rv) ERROR(rv);

  while(*fmt && !feof(file)){
    /* skip leading whitespace */
    if (iswspace(*fmt)){
      while(*fmt &&iswspace(*fmt)) ++fmt;
      while(!feof(file) && (iswspace((inchr = getwc(file))) || inchr == L'\n'));
      if(inchr != WEOF) ungetwc(inchr, file);
    }

    /* must match exactly if not conversion */
    if ( *fmt && *fmt != L'%'){
      if (*fmt != (inchr = getwc(file))){
	free(fmt);
	ErrorHandler("vfscanf_m: Mismatch error. Format string does not match input", format, EINVAL);
	ERROR(EINVAL);
      }
      continue;
    }

    if (!*fmt || feof(file))
      break;

    fmt++;

    /* skip this conversion */
    if (*fmt == L'*'){
      while(!iswspace(*fmt) && *fmt) ++fmt;
      while(!iswspace((inchr = getwc(file))) && !feof(file)) ;
      if(inchr != WEOF) ungetwc(inchr, file);
      continue;
    }

    /* get the field width */
    field_width = (size_t)-1;
    if (iswdigit(*fmt)) {
      field_width = wskip_atoi((const wchar_t **)&fmt);
    }

    /* get the qualifier */
    qualifier = -1;
    if (*fmt == L'h' || *fmt == L'l' || *fmt == L'L' || *fmt == L'Z' || *fmt == L'z'){
      qualifier = *fmt++;

      if (qualifier == *fmt){
	if (qualifier == L'h'){
	  qualifier = L'H';
	  ++fmt;
	} else if (qualifier == L'l'){
	  qualifier = L'L';
	  fmt++;
	}
      }
    }
    base = 10;
    is_sign = 0;

    if (!*fmt || feof(file))
      break;
    switch(*fmt++){
    case L'c':
      {
	if (field_width == (size_t) -1) field_width = 1;

	/* skip leading whitespace */
	if (iswspace((inchr = getwc(file))) || inchr == L'\n')
	  while(iswspace((inchr = getwc(file))) || inchr == L'\n') ;
	ungetwc(inchr, file);

	if (qualifier == L'h'){
	  char *c = (char *)va_arg(args, char *);
	  while(field_width-- > 0 && !feof(file)){
	    inchr = getwc(file);
	    wctomb(c++, inchr);
	  }
	}else {
	  wchar_t *c = (wchar_t *)va_arg(args, wchar_t *);
	  c = NULL; //eliminate unused variable warning
	  while(field_width-- > 0 && !feof(file)){
	    inchr = getwc(file);
	  }
	}
	num++;
      }
    continue;
    case 's':
      {
	string_m *s = (string_m *)va_arg(args, string_m *);
	wchar_t *tmp;
	size_t size = 0;
	size_t alloc_size = 20;


	if (field_width == (size_t) -1) field_width = SIZE_MAX;

	/* skip leading whitespace */
	if (iswspace((inchr = getwc(file))) || inchr == L'\n')
	  while (iswspace((inchr = getwc(file))) || inchr == L'\n');
	ungetwc(inchr, file);

	tmp = (wchar_t *)calloc( (field_width == SIZE_MAX) ? alloc_size : field_width , sizeof(wchar_t));
	if (!tmp) ERROR(ENOMEM);
	while (!isspace((inchr=getwc(file))) && field_width-- > 0 &&  !feof(file)){
	  if (size > (*s)->maxsize){
	    ErrorHandler("vfscanf_m: Max size exceded for string", *s, EINVAL);
	    ERROR(EINVAL);
	  }
	  if (size+1 == alloc_size){
	    alloc_size *=2;
	    tmp = (wchar_t *)realloc(tmp, alloc_size);
	    if (!tmp) ERROR(ENOMEM);
	  }

	  tmp[size++] = inchr;
	}
	ungetwc(inchr, file);
	tmp[size] = L'\0';

	if (qualifier == L'h'){
	  char *ctmp = (char *)calloc(wcslen(tmp)+1, sizeof(char));
	  if (!ctmp) ERROR(ENOMEM);
	  wcstombs(ctmp, tmp, size);
	  rv = cstrcpy_m(*s, ctmp);
	  free(ctmp);
	  free(tmp);
	  if (rv) ERROR(rv);
	} else {
	  rv = wstrcpy_m(*s, tmp);
	  free(tmp);
	  if (rv) ERROR(rv);
	}
	num++;
      }
      continue;
    case L'o':
      base = 8;
    break;
    case L'x':
    case L'X':
      base = 16;
    break;
    case L'i':
      base = 0;
    case L'd':
      is_sign = 1;
    case L'u':
      break;
    case L'%':
      inchr = getwc(file);
      if (inchr != L'%'){
	ungetwc(inchr, file);
	if (count) *count = num;
	ErrorHandler("vfscanf_m: Mismatch Error", fmt, EINVAL);
	ERROR(EINVAL);
      }
    break;
    default:
      if (count) *count = num;
      ErrorHandler("vfscanf_m: Invalid conversion", fmt, EINVAL);
      ERROR(EINVAL);
    } /* end switch */

    while(iswspace((inchr = getwc(file)))) ;
    ungetwc(inchr, file);

    digit = getwc(file);
    if (is_sign && digit == L'-'){
      digit = getwc(file);
      ungetwc(digit, file);
      ungetwc('-', file);
    } else {
      ungetwc(digit, file);
    }

    if (!digit || (base == 16 && !iswxdigit(digit))
	|| (base == 10 && !iswdigit(digit))
	|| (base == 8 && (!iswdigit(digit) || digit > L'7'))
	|| (base == 0 && !iswdigit(digit))){
      if (count) *count = num;
      ErrorHandler("vsscanf_m: Conversion mismatch error", file, EINVAL);
      ERROR(EINVAL);
    }


    /* get the input up to field_width character or until the first space */
    if (field_width == (size_t) -1 ) 
      field_width = SIZE_MAX;
    wstreamtombs(file, &str, field_width);
    switch(qualifier){

    case L'H':
      if (is_sign){
	signed char *c = (signed char *)va_arg(args, signed char *);
	*c = (signed char) wsimple_strtol(str, &next,base, field_width);
      } else {
	unsigned char *c = (unsigned char *)va_arg(args, unsigned char *);
	*c = (unsigned char)wsimple_strtoul(str, &next, base, field_width);
      }
    break;
    case 'h':
      if (is_sign){
	short *s = (short *)va_arg(args, short *);
	*s = (short)wsimple_strtol(str, &next, base, field_width);
      } else {
	unsigned short *s = (unsigned short *)va_arg(args, unsigned short *);
	*s = (unsigned short)wsimple_strtoul(str, &next, base, field_width);
      }
      break;
    case 'l':
      if (is_sign){
	long *l = (long *)va_arg(args, long *);
	*l = wsimple_strtol(str, &next, base, field_width);
      } else {
	unsigned long *l = (unsigned long *)va_arg(args, unsigned long *);
	*l = wsimple_strtoul(str, &next, base, field_width);
      }
      break;
    case 'L':
      if (is_sign){
	long long *l = (long long *)va_arg(args, long long *);
	*l = wsimple_strtoll(str, &next, base, field_width);
      } else {
	unsigned long long *l = (unsigned long long *)va_arg(args, unsigned long long *);
	*l = wsimple_strtoull(str, &next, base, field_width);
      }
      break;
    case 'Z':
    case 'z':
      {
	size_t *s = (size_t *)va_arg(args, size_t *);
	*s = (size_t)wsimple_strtoull(str, &next, base, field_width);
      }
      break;
    default:
      if (is_sign){
	int *i = (int *)va_arg(args, int *);
	*i = (int)wsimple_strtol(str, &next, base, field_width);
      } else {
	unsigned int *i = (unsigned int *)va_arg(args, unsigned int *);
	*i = (unsigned int)wsimple_strtoul(str, &next, base,field_width);
      }
      break;
    }
    num++;

  } /* end while-loop */

  if (count) *count = num;
  return 0;
}
