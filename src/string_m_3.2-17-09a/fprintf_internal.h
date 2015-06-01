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
#include <wctype.h>

#define FLG_SIGN 0x01
#define FLG_LEFT 0x02
#define FLG_ALT 0x04
#define FLG_ZERO 0x08
#define FLG_SPACE 0x10
#define FLG_SIGNED 0x20
#define FLG_CAPS 0x40

typedef enum {
  NONE, h, hh, l, ll, L, q, j, z, t
} modifier_t;

static int ccomputeLen(string_m format, va_list ap, size_t *total_width, int *strtype){
  char *p, *porig;
  errno_t rv;
  *strtype = STRTYPE_NTBS;

  rv = cgetstr_m(format, &porig);
  if (rv) ERROR(rv);

  // get the size of the format string and 
  // add one to make sure that it is never zero, 
  // which might cause malloc to return NULL.
  
  rv = strlen_m(format, total_width);
  if (rv) {
    free(porig);
    ERROR(EINVAL);
  }  
  ++*total_width;

  p = porig;
  while (*p != '\0') {
    if (*p++ == '%') {
      while (strchr ("-+ #0", *p))
	++p;
      if (*p == '*') {
	++p;
	*total_width += abs(va_arg(ap, int));
      }
      else if (isdigit(p[0]))
	*total_width += strtoul (p, (char **) &p, 10);
      /* One could legitimately ask why the isdigit() check.
	 By my reading of the man pages, it should not be
	 required.  However, on a Mac running gcc 4.0.0 20041026,
	 the strtoul function causes a free error later on if there
	 are no valid digits.  Thus, we will always check isdigit
	 before calling strtoul.  -- Hal */

      if (*p == '.') {
	++p;
	if (*p == '*') {
	  ++p;
	  *total_width += abs (va_arg (ap, int));
	}
	else if (isdigit(p[0]))
	  *total_width += strtoul (p, (char **) &p, 10);
      }
      while (strchr("hlL", *p))
	++p;
      /* Should be big enough for any format specifier except %s and floats.  */
      *total_width += 30;
      switch (*p) {
      case 'd':
      case 'i':
      case 'o':
      case 'u':
      case 'x':
      case 'X':
      case 'c':
	(void) va_arg (ap, int);
	break;

      case 'f':
	(void) va_arg (ap, double);
	/* Since an ieee double can have an exponent of 307, we'll
	   make the buffer wide enough to cover the gross case. */
	*total_width += 307;
	break;

      case 'g':
      case 'G':
      case 'e':
      case 'E':
	(void) va_arg (ap, double);
	*total_width += 12; /* d.dddddde+dd */
	break;

      case 's':
	{
	  size_t len;
	  string_m s = va_arg(ap, string_m);
	  if (strlen_m(s, &len) < 0) {
	    free(porig);
	    ERROR(EINVAL);
	  }
	  if (s->strtype == STRTYPE_WSTR)
	    *strtype = STRTYPE_WSTR;
	  *total_width += len;
	}
	break;
      case 'p':
      case 'n':
	(void) va_arg (ap, char *);
	break;
      }
      p++;
    }
  }

  free(porig);
  return 0;
}

static int
wcomputeLen(string_m format, va_list ap, size_t *total_width, int *strtype){
  wchar_t *p, *porig;
  errno_t rv;

  rv = wgetstr_m(format, &porig);
  if (rv) ERROR(rv);

  // get the size of the format string and 
  // add one to make sure that it is never zero, 
  // which might cause malloc to return NULL.
  rv = strlen_m(format, total_width);
  if (rv) {
    free(porig);
    ERROR(EINVAL);
  }
  ++*total_width;

  p = porig;
  while (*p != L'\0') {
    if (*p++ == L'%') {
      while (wcschr (L"-+ #0", *p))
	++p;
      if (*p == L'*') {
	++p;
	*total_width += abs(va_arg(ap, int));
      }
      else if (iswdigit(p[0]))
	*total_width += wcstoul(p, (wchar_t **) &p, 10);
      /* One could legitimately ask why the isdigit() check.
	 By my reading of the man pages, it should not be
	 required.  However, on a Mac running gcc 4.0.0 20041026,
	 the strtoul function causes a free error later on if there
	 are no valid digits.  Thus, we will always check isdigit
	 before calling strtoul.  -- Hal */

      if (*p == L'.') {
	++p;
	if (*p == L'*') {
	  ++p;
	  *total_width += abs (va_arg (ap, int));
	}
	else if (iswdigit(p[0]))
	  *total_width += wcstoul(p, (wchar_t **) &p, 10);
      }
      while (wcschr (L"hlL", *p))
	++p;
      /* Should be big enough for any format specifier except %s and floats.  */
      *total_width += 30;
      switch (*p) {
      case L'd':
      case L'i':
      case L'o':
      case L'u':
      case L'x':
      case L'X':
      case L'c':
	(void) va_arg (ap, int);
      break;

      case L'f':
	(void) va_arg (ap, double);
      /* Since an ieee double can have an exponent of 307, we'll
	 make the buffer wide enough to cover the gross case. */
      *total_width += 307;
      break;

      case 'g':
      case L'G':
      case L'e':
      case L'E':
	(void) va_arg (ap, double);
      *total_width += 12; /* d.dddddde+dd */
      break;

      case L's':
	{
	  size_t len;
	  string_m s = va_arg(ap, string_m);
	  if (strlen_m(s, &len) < 0) {
	    free(porig);
	    ERROR(EINVAL);
	  }
	  if (s->strtype == STRTYPE_WSTR)
	    *strtype = STRTYPE_WSTR;
	  *total_width += len;
	}
      break;
      case L'p':
      case L'n':
	(void) va_arg (ap, char *);
      break;
      }
      p++;
    }
  }

  free(porig);
  return 0;
}

/* coutputString(FILE *file, string_m format, int *count, va_list args)
 * The coutputString function formats its parameters args into a string according
 * to the format contained in the NTBS stored in the managed string format and
 * outputs the result to file.
 *
 *  0 is returned if no runtime constraints are violated otherwise a non-zero value
 *  is returned
 */
static errno_t coutputString(FILE *file, string_m format, int *count, va_list args) {
  char *p, *porig;
  errno_t rv;
  
  rv = cgetstr_m(format, &porig);
  if (rv) ERROR(rv);

  for (p = porig; *p; p++) {
    if (*p == '%') {
      int flags = 0;
      int fwidth = -1;
      int dcnt = -1;
      modifier_t mod = NONE;
      int base = 10;

      if (p[1] =='%') {
	fputc('%', file);
	*p += 2;
	(void) (*count)++;
	continue;
      }

      for(;;){
	switch (*++p) {
	case '\0':
	  free(porig);
	  ERROR(EINVAL);
	case '-':
	  flags |= FLG_LEFT;
	  continue;
	case '+':
	  flags |= FLG_SIGN;
	  continue;
	case '#':
	  flags |= FLG_ALT;
	  continue;
	case '0':
	  flags |= FLG_ZERO;
	  continue;
	case ' ':
	  flags |= FLG_SPACE;
	  continue;
	default:
	  break;
	}
	break;
      }

      /* Width specifiers */
      if (*p == '*') {
	++p;
	fwidth = va_arg(args, int);
	if (fwidth < 0) {
	  flags |= FLG_LEFT;
	  fwidth = -fwidth;
	  /* Deal with 0x8000000 width */
	  if (fwidth < 0) fwidth = 0x7ffffff;
	}
      } 
      else if (isdigit(p[0])) {
	fwidth = strtoul (p, &p, 10);
      }
      else { 
	fwidth = 0;
      }

      if (*p == '.') {
	++p;
	if (*p == '*') {
	  ++p;
	  dcnt = abs (va_arg (args, int));
	}
	else if (isdigit(p[0]))
	  dcnt = strtoul(p, (char **) &p, 10);
	else
	  dcnt = 0;
      }

      for(;;) {
	if (mod != NONE) p++;
	switch (*p) {
	case '\0':
	  free(porig);
	  ERROR(EINVAL);
	case 'h':
	  if (mod == NONE) mod = h;
	  else if (mod == h) mod = hh;
	  else { free(porig); ERROR(EINVAL);}
	  continue;
	case 'l':
	  if (mod == NONE) mod = l;
	  else if (mod == l) mod = ll;
	  else { free(porig); ERROR(EINVAL);}
	  continue;
	case 'L':
	  if (mod == NONE) mod = L;
	  else { free(porig);
	    ERROR(EINVAL);}
	  continue;
	default:
	  break;
	}
	break;
      }

      /* Should be big enough for any format specifier except %s and floats.  */
      switch (*p) {
      case '\0':
	free(porig);
	ERROR(EINVAL);

      case 'd':
      case 'i':
	flags |= FLG_SIGNED;
      case 'u':
	base = 10;
	goto integer;

      case 'o':
	base = 8;
	goto integer;
					
      case 'X':
	flags |= FLG_CAPS;
      case 'x':
	base = 16;
	if (flags & FLG_ALT) {
	  fputc('0', file);
	  if (flags & FLG_CAPS) 
	    fputc('X', file);
	  else
	    fputc('x', file);
	  *count += 2;
	}

      integer:
	{
	  unsigned int val;
	  char s = '\0';
	  char tmp[64];
	  static char lower_digits[17] = "0123456789abcdef";
	  static char upper_digits[17] = "0123456789ABCDEF";
	  const char *digits = lower_digits;

	  int plen;
	  char *p;

	  if (flags & FLG_CAPS) digits = upper_digits;

	  if (flags & FLG_SIGNED) {
	    int t = va_arg(args, int);
	    if (t < 0) {
	      val = t;
	      val = (0xffffffff - val) + 1;
	      s = '-';
	    } else val = t;
	  }

	  if (s == '\0') {
	    if (flags & FLG_SIGN)
	      s = '+';
	    else if (flags & FLG_SPACE)
	      s = ' ';
	  }

	  p = &tmp[63];
	  *p = '\0';
	  plen = 0;
	  if (val == 0) {
	    *--p = '0';
	    /* No need to prepend a 0 if it'll already be there */
	    if (base == 8) flags &= ~FLG_ALT;
	  }
	  while (val) {
	    *--p = digits[val % base];
	    plen++;
	    val /= base;
	  }
	  if ((flags & FLG_ALT) && (base == 8)) {
	    *--p = '0';
	  }
	  if (s != '\0') *--p = s;

	  if (!(flags & FLG_LEFT)) {
	    while (plen < fwidth) {
	      if (flags & FLG_ZERO) {
		fputc('0', file);
	      } else
		fputc(' ', file);
	      fwidth--;
	    }
	  }
	  while (*p) {
	    fputc(*p++, file);
	    fwidth--;
	    (void) (*count)++;
	  }
	  while (fwidth > 0) {
	    fputc(' ', file);
	    fwidth--;
	    (void) (*count)++;
	  }
	}
		
	break;

      case 'c':
	{
	  unsigned int c = va_arg (args, int);
	  if (c > 255) { free(porig);
	    ERROR(EINVAL);}

	  fputc((char)c, file);
	  (void) (*count)++;
	}
	break;

      case 'e':
      case 'E':
	{ /* -d.ddddde+03 */
	  double val = va_arg (args, double);
	  printf("Specifier not currently supported: %%e, %f\n", val);
	  if (dcnt == -1) dcnt = 6;
	}
	break;

      case 'f':
	{ /* -ddd.dddddd */
	  double val = va_arg (args, double);
	  printf("Specifier not currently supported: %%f, %f\n", val);
	  if (dcnt == -1) dcnt = 6;
	}
	break;

      case 'g':
      case 'G':
	{ /* either 'f' or 'e' depending... */
	  double val = va_arg (args, double);
	  printf("Specifier not currently supported: %%g, %f\n", val);
	}
	break;
      case 's':
	{
	  string_m s = va_arg(args, string_m);
	  size_t len;
	  char *str;

	  if (strlen_m(s, &len) < 0) 	{ free(porig);
	    ERROR(EINVAL);}

				
	  if (!(flags & FLG_LEFT)) {
	    while (len < (size_t) fwidth) {
	      fputc(' ', file);
	      fwidth--;
	      (void) (*count)++;
	    }
	  }
	  
	  rv = cgetstr_m(s, &str);
	  if (rv){
	    free(porig);
	    ERROR(EINVAL);
	  }
	  if (str) {
	    char *ptr = str;
	    while (*ptr) {
	      fputc(*ptr++, file);
	      fwidth--;
	      (void) (*count)++;
	    }
	    ptr = NULL;
	    free(str);
	  }
	  while (fwidth > 0) {
	    fputc(' ', file);
	    fwidth--;
	    (void) (*count)++;
	  }
	}
	break;
      case 'p':
      case 'n':
	(void) va_arg (args, char *);
	break;
      }
    } else {
      fputc(*p, file);
      (void) (*count)++;
    }		
  }

  free(porig);
  return 0;

}

 
static errno_t woutputString(FILE *file, string_m format,int *count, va_list args){
  wchar_t *p, *porig;
  errno_t rv;
  int outlen = 0;

  rv = wgetstr_m(format, &porig);
  if (rv) ERROR(rv);

  for (p = porig; *p; p++) {
    if (*p == L'%') {
      int flags = 0;
      int fwidth = -1;
      int dcnt = -1;
      modifier_t mod = NONE;
      int base = 10;

      if (p[1] == L'%') {
	fputwc(L'%', file);
	outlen++;
	*p += 2*sizeof(wchar_t);
	continue;
      }

      for(;;) {
	switch (*++p) {
	case L'\0':
	  free(porig);
	ERROR(EINVAL);
	case L'-':
	  flags |= FLG_LEFT;
	continue;
	case L'+':
	  flags |= FLG_SIGN;
	continue;
	case L'#':
	  flags |= FLG_ALT;
	continue;
	case L'0':
	  flags |= FLG_ZERO;
	continue;
	case L' ':
	  flags |= FLG_SPACE;
	continue;
	default:
	  break;
	}
	break;
      }

      /* Width specifiers */
      if (*p == L'*') {
	++p;
	fwidth = va_arg(args, int);
	if (fwidth < 0) {
	  flags |= FLG_LEFT;
	  fwidth = -fwidth;
	  /* Deal with 0x8000000 width */
	  if (fwidth < 0) fwidth = 0x7ffffff;
	}
      } else if (iswdigit(p[0])) 
	fwidth = wcstoul (p, &p, 10);
      else { 
	fwidth = 0;
      }

      if (*p == L'.') {
	++p;
	if (*p == L'*') {
	  ++p;
	  dcnt = abs (va_arg (args, int));
	}
	else if (iswdigit(p[0]))
	  dcnt = wcstoul (p, (wchar_t **) &p, 10);
	else
	  dcnt = 0;
      }

      for(;;) {
	if (mod != NONE) p++;
	switch (*p) {
	case L'\0':
	  free(porig);
	ERROR(EINVAL);
	case L'h':
	  if (mod == NONE) mod = h;
	  else if (mod == h) mod = hh;
	  else { free(porig);
	    ERROR(EINVAL);}
	continue;
	case L'l':
	  if (mod == NONE) mod = l;
	  else if (mod == l) mod = ll;
	  else { free(porig);
	    ERROR(EINVAL);}
	continue;
	case L'L':
	  if (mod == NONE) mod = L;
	  else { free(porig);
	    ERROR(EINVAL);}
	continue;
	default:
	  break;
	}
	break;
      }

      /* Should be big enough for any format specifier except %s and floats.  */
      switch (*p) {
      case L'\0':
	free(porig);
      ERROR(EINVAL);
      case L'd':
      case L'i':
	flags |= FLG_SIGNED;
      case L'u':
	base = 10;
      goto integer;

      case L'o':
	base = 8;
      goto integer;
					
      case L'X':
	flags |= FLG_CAPS;
      case L'x':
	base = 16;
      if (flags & FLG_ALT) {
	fputwc(L'0', file);
	outlen++;
	if (flags & FLG_CAPS) 
	  fputwc(L'X', file);
	else
	  fputwc(L'x', file);
	outlen++;
      }

      integer:
      {
	unsigned int val;
	wchar_t s = L'\0';
	wchar_t tmp[64];
	static wchar_t lower_digits[17] = L"0123456789abcdef";
	static wchar_t upper_digits[17] = L"0123456789ABCDEF";
	const wchar_t *digits = lower_digits;

	int plen;
	wchar_t *p;

	if (flags & FLG_CAPS) digits = upper_digits;

	if (flags & FLG_SIGNED) {
	  int t = va_arg(args, int);
	  if (t < 0) {
	    val = t;
	    val = (0xffffffff - val) + 1;
	    s = L'-';
	  } else val = t;
	}

	if (s == L'\0') {
	  if (flags & FLG_SIGN)
	    s = L'+';
	  else if (flags & FLG_SPACE)
	    s = L' ';
	}

	p = &tmp[63];//sizeof(tmp)-1
	*p = L'\0';
	plen = 0;
	if (val == 0) {
	  *--p = L'0';
	  /* No need to prepend a 0 if it'll already be there */
	  if (base == 8) flags &= ~FLG_ALT;
	}
	while (val) {
	  *--p = digits[val % base];
	  plen++;
	  val /= base;
	}
	if ((flags & FLG_ALT) && (base == 8)) {
	  *--p = L'0';
	}
	if (s != L'\0') *--p = s;

	if (!(flags & FLG_LEFT)) {
	  while (plen < fwidth) {
	    if (flags & FLG_ZERO) {
	      fputwc(L'0',file);
	    } else
	      fputwc(L' ',file);
	    fwidth--;
	    outlen++;
	  }
	}
	while (*p) {
	  fputwc(*p++,file);
	  fwidth--;
	  outlen++;
	}
	while (fwidth > 0) {
	  fputwc(L' ',file);
	  fwidth--;
	  outlen++;
	}
      }
		
      break;

      case L'c':
	{
	  unsigned int c = va_arg (args, int);
	  if (c > 255) {	free(porig);
	    ERROR(EINVAL);}

	  fputwc((wchar_t)c, file);
	  outlen++;
	}
      break;

      case L'e':
      case L'E':
	{ /* -d.ddddde+03 */
	  double val = va_arg (args, double);
	  printf("Specifier not currently supported: %%e, %f\n", val);
	  if (dcnt == -1) dcnt = 6;
	}
      break;

      case L'f':
	{ /* -ddd.dddddd */
	  double val = va_arg (args, double);

	  printf("Specifier not currently supported: %%f, %f\n", val);
	  if (dcnt == -1) dcnt = 6;
	}
      break;

      case L'g':
      case L'G':
	{ /* either 'f' or LETTER('e') depending... */
	  double val = va_arg (args, double);
	  printf("Specifier not currently supported: %%g, %f\n", val);
	}
      break;
      case L's':
	{
	  string_m s = va_arg(args, string_m);
	  size_t len;
	  wchar_t *str;

	  if (strlen_m(s, &len) < 0) 	{free(porig);
	    ERROR(EINVAL);}

				
	  if (!(flags & FLG_LEFT)) {\
	    while (len < (size_t) fwidth) {
	      fputwc(L' ',file);
	      outlen++;
	      fwidth--;
	    }
	  }
	  
	  rv = wgetstr_m(s, &str);
	  if (rv) {
	    free(porig);
	    ERROR(EINVAL);
	  }
	  if (str) {
	    wchar_t *ptr = str;
	    while (*ptr) {
	      fputwc(*ptr++, file);
	      outlen++;
	      fwidth--;
	    }
	    ptr = NULL;
	    free(str);
	  }
	  while (fwidth > 0) {
	    fputwc(L' ',file);
	    outlen++;
	    fwidth--;
	  }
	}
      break;
      case L'p':
      case L'n':
	(void) va_arg (args, char *);
      break;
      }
    } else {
      fputwc(*p, file);
      outlen++;
    }		
  }


  if(count) *count = outlen;

  free(porig);
  return 0;

}
