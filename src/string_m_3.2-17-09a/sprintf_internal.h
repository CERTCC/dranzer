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

/*
 * This ugly conglomeration is macros allows a single text file to
 * produce code for both NTBSs and WSTRs.  This greatly helps wih
 * maintainability (~halving), although dealing with compile errors
 * can be a bit painful.
 *
 * PREFIX(foo) produces either cfoo or wfoo
 * THISTYPE is either STRTYPE_WSTR or STRTYPE_NTBS
 * TYPE is either char or wchar_t
 * SFUNC(foo) produces either strfoo or wcsfoo
 * LETTER('A') produces either 'A' or L'A'
 * STR("Abc") produces either "Abc" or L"Abc"
 * CHECK(foo) produces either isfoo or iswfoo
 *   (LETTER and STR are the same macro.  They are both used for clarity.)
 */

#include <stdlib.h>
#include <ctype.h>
#include <math.h>

static int
PREFIX(computeLen)(string_m format, va_list ap, size_t *total_width, int *strtype) {
  TYPE *p, *porig;
  errno_t rv;
  *strtype = THISTYPE;
  
  rv = PREFIX(getstr_m)(format, &porig);
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
  while (*p != LETTER('\0')) {
    if (*p++ == LETTER('%')) {
      while (strchr ("-+ #0", *p))
	++p;
      if (*p == LETTER('*')) {
	++p;
	*total_width += abs(va_arg(ap, int));
      }
      else if (CHECK(digit)(p[0]))
	*total_width += SFUNC(toul) (p, (TYPE **) &p, 10);
      /* One could legitimately ask why the isdigit() check.
	 By my reading of the man pages, it should not be
	 required.  However, on a Mac running gcc 4.0.0 20041026,
	 the strtoul function causes a free error later on if there
	 are no valid digits.  Thus, we will always check isdigit
	 before calling strtoul.  -- Hal */

      if (*p == LETTER('.')) {
	++p;
	if (*p == LETTER('*')) {
	  ++p;
	  *total_width += abs (va_arg (ap, int));
	}
	else if (CHECK(digit)(p[0]))
	  *total_width += SFUNC(toul) (p, (TYPE **) &p, 10);
      }
      while (SFUNC(chr) (STR("hlL"), *p))
	++p;
      /* Should be big enough for any format specifier except %s and floats.  */
      *total_width += 30;
      switch (*p) {
      case LETTER('d'):
      case LETTER('i'):
      case LETTER('o'):
      case LETTER('u'):
      case LETTER('x'):
      case LETTER('X'):
      case LETTER('c'):
	(void) va_arg (ap, int);
	break;

      case LETTER('f'):
	(void) va_arg (ap, double);
	/* Since an ieee double can have an exponent of 307, we'll
	   make the buffer wide enough to cover the gross case. */
	*total_width += 307;
	break;

      case LETTER('g'):
      case LETTER('G'):
      case LETTER('e'):
      case LETTER('E'):
	(void) va_arg (ap, double);
	*total_width += 12; /* d.dddddde+dd */
	break;

      case LETTER('s'):
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
      case LETTER('p'):
      case LETTER('n'):
	(void) va_arg (ap, char *);
	break;
      default: ERROR(EINVAL);
      }
      p++;
    }
  }

  free(porig);
  return 0;
}

static int
PREFIX(outputString)(string_m s, string_m format,  va_list args) {
  TYPE *p, *porig;
  TYPE *result;
  errno_t rv;

  if (s->strtype != THISTYPE) ERROR(EINVAL);
  result = s->str.PREFIX(str);

  rv = PREFIX(getstr_m)(format, &porig);
  if (rv) ERROR(rv);

  for (p = porig; *p; p++) {
    if (*p == LETTER('%')) {
      int flags = 0;
      int fwidth = -1;
      int dcnt = -1;
      modifier_t mod = NONE;
      int base = 10;

      if (p[1] == LETTER('%')) {
	*result++ = LETTER('%');
	*p += 2;
	continue;
      }

      for(;;) {
	switch (*++p) {
	case LETTER('\0'):
	  free(porig);
	  ERROR(EINVAL);

	case LETTER('-'):
	  flags |= FLG_LEFT;
	  continue;
	case LETTER('+'):
	  flags |= FLG_SIGN;
	  continue;
	case LETTER('#'):
	  flags |= FLG_ALT;
	  continue;
	case LETTER('0'):
	  flags |= FLG_ZERO;
	  continue;
	case LETTER(' '):
	  flags |= FLG_SPACE;
	  continue;
	default:
	  break;
	}
	break;
      }

      /* Width specifiers */
      if (*p == LETTER('*')) {
	++p;
	fwidth = va_arg(args, int);
	if (fwidth < 0) {
	  flags |= FLG_LEFT;
	  fwidth = -fwidth;
	  /* Deal with 0x8000000 width */
	  if (fwidth < 0) fwidth = 0x7ffffff;
	}
      } else if (CHECK(digit)(p[0])) 
	fwidth = SFUNC(toul) (p, &p, 10);
      else fwidth = 0;

      if (*p == LETTER('.')) {
	++p;
	if (*p == LETTER('*')) {
	  ++p;
	  dcnt = abs (va_arg (args, int));
	}
	else if (CHECK(digit)(p[0]))
	  dcnt = SFUNC(toul) (p, (TYPE **) &p, 10);
	else
	  dcnt = 0;
      }

      for(;;) {
	if (mod != NONE) p++;
	switch (*p) {
	case LETTER('\0'):
	  free(porig);
	  ERROR(EINVAL);

	case LETTER('h'):
	  if (mod == NONE) mod = h;
	  else if (mod == h) mod = hh;
	  else {
	    free(porig);
	    ERROR(EINVAL);
	  }
	  continue;
	case LETTER('l'):
	  if (mod == NONE) mod = l;
	  else if (mod == l) mod = ll;
	  else{
	    free(porig);
	    ERROR(EINVAL);
	  }
	  continue;
	case LETTER('L'):
	  if (mod == NONE) mod = L;
	  else{
	    free(porig);
	    ERROR(EINVAL);
	  }
	  continue;
	default:
	  break;
	}
	break;
      }

      /* Should be big enough for any format specifier except %s and floats.  */
      switch (*p) {
      case LETTER('\0'):
	free(porig);
	ERROR(EINVAL);

      case LETTER('d'):
      case LETTER('i'):
	flags |= FLG_SIGNED;
      case LETTER('u'):
	base = 10;
	goto integer;

      case LETTER('o'):
	base = 8;
	goto integer;
					
      case LETTER('X'):
	flags |= FLG_CAPS;
      case LETTER('x'):
	base = 16;
	if (flags & FLG_ALT) {
	  *result++ = LETTER('0');
	  if (flags & FLG_CAPS) 
	    *result++ = LETTER('X');
	  else
	    *result++ = LETTER('x');
	}

      integer:
	{
	  unsigned int val;
	  TYPE s = LETTER('\0');
	  TYPE tmp[64];
	  static TYPE lower_digits[17] = STR("0123456789abcdef");
	  static TYPE upper_digits[17] = STR("0123456789ABCDEF");
	  const TYPE *digits = lower_digits;

	  int plen;
	  TYPE *p;

	  if (flags & FLG_CAPS) digits = upper_digits;

	  if (flags & FLG_SIGNED) {
	    int t = va_arg(args, int);
	    if (t < 0) {
	      val = t;
	      val = (0xffffffff - val) + 1;
	      s = LETTER('-');
	    } else val = t;
	  }

	  if (s == LETTER('\0')) {
	    if (flags & FLG_SIGN)
	      s = LETTER('+');
	    else if (flags & FLG_SPACE)
	      s = LETTER(' ');
	  }

	  p = &(tmp[sizeof(tmp)-1]);
	  *p = LETTER('\0');
	  plen = 0;
	  if (val == 0) {
	    *--p = LETTER('0');
	    /* No need to prepend a 0 if it'll already be there */
	    if (base == 8) flags &= ~FLG_ALT;
	  }
	  while (val) {
	    *--p = digits[val % base];
	    plen++;
	    val /= base;
	  }
	  if ((flags & FLG_ALT) && (base == 8)) {
	    *--p = LETTER('0');
	  }
	  if (s != LETTER('\0')) *--p = s;

	  if (!(flags & FLG_LEFT)) {
	    while (plen < fwidth) {
	      if (flags & FLG_ZERO) {
		*result++ = LETTER('0');
	      } else
		*result++ = LETTER(' ');
	      fwidth--;
	    }
	  }
	  while (*p) {
	    *result++ = *p++;
	    fwidth--;
	  }
	  while (fwidth > 0) {
	    *result++ = LETTER(' ');
	    fwidth--;
	  }
	}
		
	break;

      case LETTER('c'):
	{
	  unsigned int c = va_arg (args, int);
	  if (c > 255) {
	    free(porig);
	    ERROR(EINVAL);
	  }
	  *result++ = (TYPE)c;
	}
	break;

      case LETTER('E'):
	flags |= FLG_CAPS;

      case LETTER('e'):
	{ /* -d.ddddde+03 */
	  double val = va_arg(args, double);
	  printf("Specifier not currently supported: %%e, %f\n", val);
	  if(dcnt == -1) dcnt = 6;
	}
	break;

      case LETTER('f'):
	{ /* -ddd.dddddd */
	  double val = va_arg (args, double);
	  printf("Specifier not currently supported: %%f, %f\n", val);
	  if (dcnt == -1) dcnt = 6;

	}
	break;
      case LETTER('g'):
      case LETTER('G'):
	{ /* either LETTER('f') or LETTER('e') depending... */
	  double val = va_arg (args, double);
	  printf("Specifier not currently supported: %%g, %f\n", val);
	}
	break;
      case LETTER('s'):
	{
	  string_m s = va_arg(args, string_m);
	  size_t len;
	  TYPE *str;

	  if (strlen_m(s, &len) < 0){
	    free(porig);
	    ERROR(EINVAL);
	  }

				
	  if (!(flags & FLG_LEFT)) {
	    while (len < (size_t)fwidth) {
	      *result++ = LETTER(' ');
	      fwidth--;
	    }
	  }	  
	  rv = PREFIX(getstr_m)(s, &str);
	  if (rv) {
	    free(porig);
	    ERROR(EINVAL);
	  }
	  if (str) {
	    TYPE *ptr = str;
	    while (*ptr) {
	      *result++ = *ptr++;
	      fwidth--;
	    }
	    ptr = NULL;
	    free(str);
	  }
	  while (fwidth > 0) {
	    *result++ = LETTER(' ');
	    fwidth--;
	  }
	}
	break;
      case LETTER('p'):
      case LETTER('n'):
	(void) va_arg (args, char *);
	break;
      default: ERROR(EINVAL);
      }
    } else {
      *result++ = *p;
    }		
  }

  *result++ = '\0';


  // if destination string s1 has charset defined make sure
  // all characters in source are valid
  if ( (s->charset.PREFIX(str)) && (SFUNC(spn)(s->str.PREFIX(str), s->charset.PREFIX(str)) != SFUNC(len)(s->str.PREFIX(str))) ) {
    free(porig);
    ERROR(EINVAL);
  }

  free(porig);
  return 0;


}
