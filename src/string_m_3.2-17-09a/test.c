#ifdef __windows
#include "stdafx.h"
#endif

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
#include <wchar.h>
#include <errno.h>
#include "stdbool.h"
#include <stdlib.h>
#include <string.h>
#include "string_m.h"
#include "string_m_internals.h"

#define _CRT_SECURE_NO_DEPRECATE
#define TEST(A) {						\
  rv = (A);							\
  if (rv) {							\
    fprintf(stdout, " Error %d from %s\n", rv, # A);		\
    nerror++;							\
    iserr++;							\
  }}

#define TESTFAIL(A, B) {					\
  rv = (A);							\
  if (rv != (B)) {						\
    fprintf(stdout, " Expected error %d, got %d from %s", (B), rv, # A); \
    nerror++;							\
    iserr++;							\
  }}

#define DESTROY(A) {						\
    if (strdelete_m(&(A)) != 0) {				\
      fprintf (stdout, " Unable to strdelete_m %s", # A);	\
      nerror++;							\
      iserr++;							\
    }}

int nerror = 0;

void strcreate_test();
void strcpy_test();
void strtok_test();
void setcharset_test();
void sprintf_test();
void strdelete_test();
void strncat_test();

const char    TEST_STRING1[] = "hello, world.";
const char    TEST_STRING2[] = "hello, universe.";
const wchar_t TEST_WSTRING1[] = L"hello, world.";
const wchar_t TEST_WSTRING2[] = L"hello, universe.";

int
testlen(string_m str, size_t expectlen) {
  errno_t rv;
  size_t len;
  int iserr=0;

  TEST(strlen_m(str, &len));
  if (!rv) {
    if (len != expectlen) {
      fprintf (stdout, " Expected length of %zd, got length of %zd",
	       expectlen, len);
      nerror++;
      return 1;
    }
  }
  return 0;
}

int
wdisplay(string_m str, const wchar_t *expected) {
  errno_t rv;
  wchar_t *wstr;
  int iserr=0;

  TEST(wgetstr_m(str, &wstr));
  if (!rv) {
    if (!wstr) {
      if (expected) {
	fprintf (stdout, " Got NULL, expected non-NULL");
	nerror++;
	iserr++;
      }
    } else if (!expected) {
      fprintf (stdout, " Got non-NULL, expected NULL");
      nerror++;
      iserr++;
    } else if (wcscmp(wstr, expected) != 0) {
      fwprintf (stdout, L" Got \"%ls\", expected \"%ls\"",
		wstr, expected);
      nerror++;
      iserr++;
    }
    if (wstr) {
      fwprintf(stdout, L" \"%ls\" ", wstr);
      /* ignore errors to wprintf */
      free(wstr);
    } else {
      wprintf( L"(null)");
    }
  }
  return iserr;
}

int
display(string_m str, const char *expected) {
  errno_t rv;
  char *cstr;
  int iserr=0;

  TEST(cgetstr_m(str, &cstr));
  if (!rv) {
    if (!cstr) {
      if (expected) {
	fprintf (stdout, " Got NULL, expected non-NULL");
	nerror++;
	iserr++;
      }
    } else if (!expected) {
      fprintf (stdout, " Got non-NULL, expected NULL");
      nerror++;
      iserr++;
    } else if (strcmp(cstr, expected) != 0) {
      fprintf (stdout, " Got \"%s\", expected \"%s\"",
	       cstr, expected);
      nerror++;
      iserr++;
    }
    if (cstr) {
      if ((rv = printf(" \"%s\" ", cstr)) < 0) {
	free(cstr);
	perror("printf");
	return rv;
      }
      free(cstr);
    } else {
      printf (" (null)");
    }
  }
  return iserr;
}

int main() {

  errno_t	rv;
  rsize_t	count=0;
  _Bool         condition;
  char	        c3,tmp;
  wchar_t	c1,c2,wtmp;

  //short shrt;
  int i1;

  string_m str1 = NULL;
  string_m str2 = NULL;
  string_m str3 = NULL;
  string_m str4 = NULL;

  string_m dest = NULL;
  string_m format = NULL;

  int iserr=0;


  // NOTE:  The printf statements below are not considered "safe" but are used
  //        here simply for testing.

  /* test the strcreate functions */
  strcreate_test();

  /* test the strcpy functions */
  strcpy_test();

  /* test the strtok functions */
  strtok_test();

  /* test the setcharset function */
  setcharset_test();

  sprintf_test();

  strdelete_test();

  strncat_test();

  /* strcmp_m testing */
  printf("\nstrcmp_m setup --->");
  iserr = 0;
  TEST(strcreate_m(&str1, TEST_STRING1, 0, NULL));
  TEST(strcreate_m(&str2, TEST_STRING2, 0, NULL));
  TEST(strcreate_m(&str3, NULL, 0, NULL));
  TEST(strcreate_m(&str4, "", 0, NULL));
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");
	
  printf("strcmp_m Case #1 --->");
  iserr = 0;
  TEST(strcmp_m(str1, str1, &condition));
  if(!rv){
    if(condition != 0){
      iserr++;
      nerror++;
    }
  }
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strcmp_m Case #2 --->");
  iserr = 0;
  TEST(strcmp_m(str1, str2, &condition));
  if(!rv){
    if(condition <= 0){
      iserr++;
      nerror++;
    }
  }
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strcmp_m Case #3 --->");
  iserr = 0;
  TEST(strcmp_m(str3, str3, &condition));
  if(!rv){
    if(condition != 0){
      iserr++;
      nerror++;
    }
  }
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strcmp_m Case #4 --->");
  iserr = 0;
  TEST(strcmp_m(str4, str4, &condition));
  if(!rv){
    if(condition != 0){
      iserr++;
      nerror++;
    }
  }
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strcmp_m Case #5 --->");
  iserr = 0;
  TEST(strcmp_m(str3,str4, &condition));
  if(!rv){
    if(condition >= 0){
      iserr++;
      nerror++;
    }
  }
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("\nstrncmp_m Case #1 --->");
  iserr = 0;
  TEST(strncmp_m(str1, str1, strlen(TEST_STRING1), &condition));
  if(!rv){
    if(condition != 0){
      iserr++;
      nerror++;
    }
  }
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strncmp_m Case #2 --->");
  iserr = 0;
  TEST(strncmp_m(str1, str2, 1, &condition));
  if(!rv){
    if(condition != 0){
      iserr++;
      nerror++;
    }
  }
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strncmp_m Case #3 --->");
  iserr = 0;
  TEST(strncmp_m(str1, str3, 20, &condition));
  if(!rv){
    if(condition <= 0){
      iserr++;
      nerror++;
    }
  }
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strncmp_m Case #4 --->");
  iserr = 0;
  TEST(strncmp_m(str3, str1, 20, &condition));
  if(!rv){
    if(condition >= 0){
      iserr++;
      nerror++;
    }
  }
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strncmp_m Case #5 --->");
  iserr = 0;
  TEST(strncmp_m(str3, str4, 0, &condition));
  if(!rv){
    if(condition != 0){
      iserr++;
      nerror++;
    }
  }	
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strncmp_m Case #6 --->");
  iserr = 0;
  TEST(cstrncmp_m(str1, "sello, world.", 1, &condition));
  if(!rv){
    if(condition >= 0){
      iserr++;
      nerror++;
    }
  }
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("Tearing down strcmp_m --->");
  iserr = 0;
  DESTROY(str1);
  DESTROY(str2);
  DESTROY(str3);
  DESTROY(str4);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("\ncstrchr_m Case #1 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, NULL, 0, NULL));
  TEST(strcreate_m(&str2, TEST_STRING1, 0, NULL));
  TEST(cstrchr_m(str1, str2, 'o'));
  if(!rv){
    iserr |= display(str1, "o, world.");
    iserr |= testlen(str1, strlen("o, world."));
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("cstrchr_m Case #2 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, NULL, 0, NULL));
  TEST(strcreate_m(&str2, TEST_STRING1, 0,NULL));
  TEST(cstrchr_m(str1, str2, 'z'));
  if(!rv){
    TEST(isnull_m(str1, &condition));
    if(!condition) {
      printf("Expected NULL string, got Non-NULL string");
      iserr++;
      nerror++;
    }
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("cstrchr_m Case #3 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, NULL, 0, NULL));
  TEST(strcreate_m(&str2, NULL, 0, NULL));
  TEST(cstrchr_m(str1, str2, 'a'));
  if(!rv){
    TEST(isnull_m(str1, &condition));
    if(!condition){
      printf("Expected NULL string, got Non-NULL string");
      iserr++;
      nerror++;
    }
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("cstrchr_m Case #4 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, NULL, 0, NULL));
  TEST(wstrcreate_m(&str2, TEST_WSTRING1, 0, NULL));
  TEST(cstrchr_m(str1, str2, 'h'));
  if(!rv){
    iserr |= wdisplay(str1, TEST_WSTRING1);
    iserr |= testlen(str1, wcslen(TEST_WSTRING1));
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  /* strspn_m testing */
  printf("\nstrspn_m Case #1 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, TEST_STRING1, 0, NULL));
  TEST(strcreate_m(&str2, "helo, wrd.", 0, NULL));
  TEST(strspn_m(str1, str2, &count));
  if(!rv){
    if(count != strlen(TEST_STRING1)){
      printf(" Expected %zd, got %d", strlen(TEST_STRING1), condition); 
      iserr++;
      nerror++;
    }
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strspn_m Case #2 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, TEST_STRING1, 0, NULL));
  TEST(wstrcreate_m(&str2, L"helo, wrd.", 0, NULL));
  TEST(strspn_m(str1, str2, &count));
  if(!rv){
    if(count != strlen(TEST_STRING1)){
      printf("Expected %zd, got %d", strlen(TEST_STRING1), condition);
      iserr++;
      nerror++;
    }
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strspn_m Case #3 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, NULL, 0, NULL));
  TEST(wstrcreate_m(&str2, L"helo, wrd.", 0, NULL));
  TEST(strspn_m(str1, str2, &count));
  if(!rv){
    if(count != 0){
      printf("Expected %d, got %d", 0, condition);
      iserr++;
      nerror++;
    }
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strspn_m Case #4 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, TEST_STRING1, 0, NULL));
  TEST(wstrcreate_m(&str2, NULL, 0, NULL));
  TEST(strspn_m(str1, str2, &count));
  if(!rv){
    if(count != strlen(TEST_STRING1)){
      printf("Expected %zd, got %d", strlen(TEST_STRING1), condition);
      iserr++;
      nerror++;
    }
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strncspn_m Case #1 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, TEST_STRING1, 0, NULL));
  TEST(strcreate_m(&str2, "helo, wrd.", 0, NULL));
  TEST(strcspn_m(str1, str2, &count));
  if(!rv){
    if(count != 0){
      printf("Expected 0, got %d", condition );
      iserr++;
      nerror++;
    }
  }	
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strncspn_m Case #2 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, "abcd hello, world.", 0, NULL));
  TEST(strcreate_m(&str2, "helo, wrd.", 0, NULL));
  TEST(strcspn_m(str1,str2, &count));
  if(!rv){
    if(count != 3){
      printf(" Expected 3 got %zd", count);
      iserr++;
      nerror++;
    }
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strncspn_m Case #3 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, NULL, 0, NULL));
  TEST(strcreate_m(&str2, "helo, wrd.", 0, NULL));
  TEST(strcspn_m(str1,str2, &count));
  if(!rv){
    if(count != 0){
      printf(" Expected 0 got %zd", count);
      iserr++;
      nerror++;
    }
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strncspn_m Case #4 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, "", 0, NULL));
  TEST(strcreate_m(&str2, "helo, wrd.", 0, NULL));
  TEST(strcspn_m(str1,str2, &count));
  if(!rv){
    if(count != 0){
      printf(" Expected 0 got %zd", count);
      iserr++;
      nerror++;
    }
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strncspn_m Case #5 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, TEST_STRING1, 0, NULL));
  TEST(strcreate_m(&str2, "", 0, NULL));
  TEST(strcspn_m(str1,str2, &count));
  if(!rv){
    if(count != strlen(TEST_STRING1)){
      printf(" Expected %zd got %zd", strlen(TEST_STRING1), count);
      iserr++;
      nerror++;
    }
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strncspn_m Case #6 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, TEST_STRING1, 0, NULL));
  TEST(strcreate_m(&str2, NULL, 0, NULL));
  TEST(strcspn_m(str1,str2, &count));
  if(!rv){
    if(count != strlen(TEST_STRING1)){
      printf(" Expected %zd got %zd", strlen(TEST_STRING1), count);
      iserr++;
      nerror++;
    }
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("\nsetcharset_m Case #1 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, TEST_STRING1, 0, NULL));
  TEST(strcreate_m(&str2, "helo, wrd.", 0, NULL));
  TEST(setcharset_m(str1, str2));
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("\nsetcharset_m Case #2 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, TEST_STRING1, 0, NULL));
  TEST(strcreate_m(&str2, "hlo, wrd.", 0, NULL));
  TESTFAIL(setcharset_m(str1, str2), EINVAL);
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("\nsetcharset_m Case #3 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, NULL, 0, NULL));
  TEST(strcreate_m(&str2, "helo, wrd.", 0, NULL));
  TEST(setcharset_m(str1, str2));
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("\nNTBS testing completed\n\n");

  fwide(stdout, 1);
  perror("fwide");

  errno = 0;

  printf ("strcreate_m for wide_t testing--->");
  TEST(strcreate_m(&dest, NULL, 0, NULL));
  if (!rv) printf (" [success]\n");
  else printf (" [failure]\n");
  printf ("\n");

  printf ("wstrcpy_m Case #1 --->");
  iserr = 0;
  TEST(wstrcpy_m(dest, L"Hello world"));
  if (!rv) {
    rv = wdisplay(dest, L"Hello world");
  }
  if (rv) printf (" [failure]\n");
  else printf (" [success]\n");

  printf ("cstrcpy_m/wgetstr_m Case #1 --->");
  TEST(cstrcpy_m(dest, "Goodbye"));
  if (!rv) {
    rv = wdisplay(dest, L"Goodbye");
  }
  if (rv) printf (" [failure]\n");
  else printf (" [success]\n");

  printf ("cstrcpy_m/wstrcat_m Case #1 --->");
  TEST(cstrcpy_m(dest, "Aufwieder"));
  if (!rv) TEST(wstrcat_m(dest, L"sehen"));
  if (!rv) {
    rv = wdisplay(dest, L"Aufwiedersehen");
    rv |= testlen(dest, wcslen(L"Aufwiedersehen"));
  }
  if (rv) printf (" [failure]\n");
  else printf (" [success]\n");

  printf ("wstrcpy_m/cstrcat_m Case #1 --->");
  TEST(wstrcpy_m(dest, L"Bis"));
  if (!rv) TEST(cstrcat_m(dest, " spaeter"));
  if (!rv) {
    rv = wdisplay(dest, L"Bis spaeter");
    rv |= testlen(dest, wcslen(L"Bis spaeter"));
  }
  if (rv) printf (" [failure]\n");
  else printf (" [success]\n");

  DESTROY(dest);

  printf ("wstrcreate_m Case #1 --->");
  iserr = 0;
  TEST(wstrcreate_m(&dest, L"Hello world", 0, NULL));
  if (!rv) {
    rv = wdisplay(dest, L"Hello world");
    DESTROY(dest);
  }
  if (rv) printf (" [failure]\n");
  else printf (" [success]\n");

  printf ("strcreate_m/wgetstr_m Case #2 --->");
  TEST(strcreate_m(&dest, "Goodbye", 0, NULL));
  if (!rv) {
    rv = wdisplay(dest, L"Goodbye");
    DESTROY(dest);
  }
  if (rv) printf (" [failure]\n");
  else printf (" [success]\n");

  printf ("strcreate_m/wstrcat_m Case #2 --->");
  dest = NULL;
  TEST(strcreate_m(&dest, "Aufwieder", 0, NULL));
  if (!rv) TEST(wstrcat_m(dest, L"sehen"));
  if (!rv) {
    rv = wdisplay(dest, L"Aufwiedersehen");
    rv |= testlen(dest, wcslen(L"Aufwiedersehen"));
  }
  if (dest) DESTROY(dest);
  if (rv) printf (" [failure]\n");
  else printf (" [success]\n");

  printf ("wstrcpy_m/cstrcat_m Case #1 --->");
  dest = NULL;
  TEST(wstrcreate_m(&dest, L"Bis", 0, NULL));
  if (!rv) TEST(cstrcat_m(dest, " spaeter"));
  if (!rv) {
    rv = wdisplay(dest, L"Bis spaeter");
    rv |= testlen(dest, wcslen(L"Bis spaeter"));
  }
  if (dest) DESTROY(dest);
  if (rv) printf (" [failure]\n");
  else printf (" [success]\n");

  printf ("wcharset Case #1 --->");
  dest = NULL;
  iserr = 0;
  TEST(wstrcreate_m(&dest, L"Bis", 0, L"ABCisspatr "));
  // e is illegal
  if (!rv) TESTFAIL(wstrcat_m(dest, L" spaeter"), EINVAL);
  if (!rv) {
    rv = wdisplay(dest, L"Bis spaeter");
    rv |= testlen(dest, wcslen(L"Bis spaeter"));
  }
  if (dest) DESTROY(dest);
  if (iserr) printf (" [failure]\n");
  else printf (" [success]\n");

  printf ("wcharset Case #2 --->");
  dest = NULL;
  iserr = 0;
  TEST(wstrcreate_m(&dest, L"Bis", 0, L"ABCisspaeter "));
  if (!rv) TEST(wstrcat_m(dest, L" spaeter"));
  if (!rv) {
    rv = wdisplay(dest, L"Bis spaeter");
    rv |= testlen(dest, wcslen(L"Bis spaeter"));
  }
  if (dest) DESTROY(dest);
  if (iserr) printf (" [failure]\n");
  else printf (" [success]\n");

  printf ("wcharset Case #3 --->");
  dest = NULL;
  iserr = 0;
  TEST(wstrcreate_m(&dest, L"Bis", 0, L"ABCisspatr "));
  // e is illegal
  if (!rv) TESTFAIL(wstrcpy_m(dest, L" spaeter"), EINVAL);
  if (!rv) {
    rv = wdisplay(dest, L"Bis spaeter");
    rv |= testlen(dest, wcslen(L"Bis spaeter"));
  }
  if (dest) DESTROY(dest);
  if (iserr) printf (" [failure]\n");
  else printf (" [success]\n");

  printf ("wcharset Case #4 --->");
  dest = NULL;
  iserr = 0;
  TEST(wstrcreate_m(&dest, L"Bis", 0, L"ABCisspatr "));
  // e is illegal
  if (!rv) TESTFAIL(wstrncpy_m(dest, L" spaeter", 20), EINVAL);
  if (!rv) {
    rv = wdisplay(dest, L" spaeter");
    rv |= testlen(dest, wcslen(L" spaeter"));
  }
  if (dest) DESTROY(dest);
  if (iserr) printf (" [failure]\n");
  else printf (" [success]\n");

  printf ("wcharset Case #5 --->");
  dest = NULL;
  iserr = 0;
  TEST(strcreate_m(&dest, "Bis", 0, "ABCisspatr "));
  // e is illegal
  if (!rv) TESTFAIL(wstrncpy_m(dest, L" spaeter", 20), EINVAL);
  if (!rv) {
    rv = wdisplay(dest, L" spaeter");
    rv |= testlen(dest, wcslen(L" spaeter"));
  }
  if (dest) DESTROY(dest);
  if (iserr) printf (" [failure]\n");
  else printf (" [success]\n");

  printf ("wcharset Case #6 --->");
  dest = NULL;
  iserr = 0;
  TEST(strcreate_m(&dest, "Bis", 0, "ABCisspatr "));
  // e is illegal
  if (!rv) TEST(wstrncpy_m(dest, L" spaeter", 4));
  if (!rv) {
    rv = wdisplay(dest, L" spa");
    rv |= testlen(dest, wcslen(L" spa"));
  }
  if (dest) DESTROY(dest);
  if (iserr) printf (" [failure]\n");
  else printf (" [success]\n");

  printf ("wcharset Case #7 --->");
  dest = NULL;
  iserr = 0;
  TEST(strcreate_m(&dest, "Bis", 0, "ABCisspatr "));
  // e is illegal
  if (!rv) TESTFAIL(wstrncpy_m(dest, L" spaeter", 20), EINVAL);
  if (!rv) {
    rv = wdisplay(dest, L" spaeter");
    rv |= testlen(dest, wcslen(L" spaeter"));
  }
  if (dest) DESTROY(dest);
  if (iserr) printf (" [failure]\n");
  else printf (" [success]\n");

  printf ("wcharset Case #8 --->");
  dest = NULL;
  iserr = 0;
  TEST(strcreate_m(&dest, "Bis", 0, "ABCisspatr "));
  // e is illegal
  if (!rv) TEST(wstrncpy_m(dest, L" spaeter", 4));
  if (!rv) {
    rv = wdisplay(dest, L" spa");
    rv |= testlen(dest, wcslen(L" spa"));
  }
  if (dest) DESTROY(dest);
  if (iserr) printf (" [failure]\n");
  else printf (" [success]\n");

  printf ("strtok_m Case #1 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, NULL, 0, NULL));
  TEST(strcreate_m(&str2, TEST_STRING1, 0, NULL));
  TEST(wstrcreate_m(&str3, L" ", 0, NULL));
  TEST(strcreate_m(&str4, NULL, 0, NULL));
  // Cannot strtok_m a NTBS with a WSTR delimeter set
  TESTFAIL(strtok_m(str1, str2, str3, str4), EINVAL);
  DESTROY(str1);
  DESTROY(str2);
  DESTROY(str3);
  DESTROY(str4);
  if (!iserr) printf (" [success]\n");
  else printf (" [failure]\n");

  printf ("strtok_m Case #2 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, NULL, 0, NULL));
  TEST(wstrcreate_m(&str2, TEST_WSTRING1, 0, NULL));
  TEST(wstrcreate_m(&str3, L" ", 0, NULL));
  TEST(strcreate_m(&str4, NULL, 0, NULL));
  TEST(strtok_m(str1, str2, str3, str4));
  if (!iserr) {
    iserr |= wdisplay(str1, L"hello,");
    iserr |= testlen(str1, wcslen(L"hello,"));

    iserr |= wdisplay(str4, L"world.");
    iserr |= testlen(str4, wcslen(L"world."));
  }
  DESTROY(str1);
  DESTROY(str2);
  DESTROY(str3);
  DESTROY(str4);
  if (!iserr) printf (" [success]\n");
  else printf (" [failure]\n");

  //strncat test
  printf("\nwstrncat_m Case #1 --->");
  iserr = 0;
  TEST(wstrcreate_m(&str1, TEST_WSTRING1, 0, NULL));
  TEST(wstrcreate_m(&str2, TEST_WSTRING2, 0, NULL));
  TEST(strncat_m(str1, str2, wcslen(TEST_WSTRING1)+wcslen(TEST_WSTRING1)));
  if(!rv){
    iserr |= wdisplay(str1, L"hello, world.hello, universe.");
    iserr |= testlen(str1, wcslen(TEST_WSTRING1)+wcslen(TEST_WSTRING2));
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf ("\nWide testing completed\n\n");

  fprintf (stdout, "\nTotal testing errors: %d\n", nerror);

  printf("strslice_m Case #1 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, TEST_STRING1, 0, NULL));
  TEST(strcreate_m(&str2, TEST_STRING1, 0, NULL));
  TEST(strslice_m(str1, str2, 7, 2));
  if(!rv){
    iserr |= display(str1, "hello, world.wo");
    iserr |= testlen(str1, strlen("hello, world.wo"));
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strslice_m Case #2 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, TEST_STRING1, 0, NULL));
  TEST(strcreate_m(&str2, TEST_STRING1, 0, NULL));
  TEST(strslice_m(str1, str2, 7, 0));
  if(!rv){
    iserr |= display(str1, "");
    iserr |= testlen(str1, strlen(""));
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");
	
  printf("strslice_m Case #3 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, TEST_STRING1, 0, NULL));
  TEST(strcreate_m(&str2, TEST_STRING1, 0, NULL));
  TEST(strslice_m(str1, str2, 14, 2));
  if(!rv){
    TEST(isnull_m(str1, &condition));
    if(!condition){
      iserr++;
      nerror++;
    }
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strslice_m Case #4 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, TEST_STRING1, 0, NULL));
  TEST(strcreate_m(&str2, TEST_STRING1, 0, NULL));
  TEST(strslice_m(str1, str2, 13, 2));
  if(!rv){
    iserr |= display(str1, "");
    iserr |= testlen(str1, strlen(""));
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strslice_m Case #5 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, TEST_STRING1, 0, NULL));
  TEST(strcreate_m(&str2, TEST_STRING1, 0, NULL));
  TEST(strslice_m(str1, str2, 11, 5));
  if(!rv){
    iserr |= display(str1, "hello, world.d.");
    iserr |= testlen(str1, strlen("hello, world.d."));
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strslice_m Case #6 --->");
  iserr = 0;
  TEST(wstrcreate_m(&str1, L"aaa", 0, L"ab"));
  TEST(wstrcreate_m(&str2, L"ccbb", 0, NULL));
  TEST(strslice_m(str1, str2, 2, 2));
  if(!rv){
    iserr |= wdisplay(str1, L"aaabb");
    iserr |= testlen(str1, wcslen(L"aaabb"));
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strleft_m Case #1 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, "aaa", 0, "ab"));
  TEST(strcreate_m(&str2, "bbccc", 0, NULL));
  TEST(strleft_m(str1, str2, 2));
  if(!rv){
    iserr |= display(str1, "aaabb");
    iserr |= testlen(str1, strlen("aaabb"));
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strleft_m Case #2 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, "aaa", 0, "ab"));
  TEST(strcreate_m(&str2, NULL, 0, NULL));
  TEST(strleft_m(str1, str2, 2));
  if(!rv){
    TEST(isnull_m(str1, &condition));
    if(!condition) iserr++;
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("cchar_m Case #1 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, TEST_STRING1, 0, NULL));
  TEST(cchar_m(str1, 1, &tmp));
  if(!rv){
    if(tmp != 'e') iserr++;
  }
  DESTROY(str1);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("wchar_m Case #1 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, TEST_STRING1, 0, NULL));
  TEST(wchar_m(str1, 1, &wtmp));
  if(!rv){
    if(tmp != L'e') iserr++;
  }
  DESTROY(str1);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");
   

  TEST(strcreate_m(&str1, "hello %d\n", 0, NULL));
  TEST(fprintf_m(stdout, str1, &condition, 10));
  DESTROY(str1);


  TEST(strcreate_m(&format, "%s I have %d coconuts and %d shrimp\n", 0, NULL));
  TEST(strcreate_m(&str1, TEST_STRING1, 0, NULL));
  TEST(fprintf_m(stdout, format, &condition, str1, 100, 10));
  DESTROY(format);
  DESTROY(str1);

  TEST(strcreate_m(&format, "%c %lc %hc",0,NULL));
  TEST(wstrcreate_m(&str1, L"xkw", 0, NULL));
  TEST(sscanf_m(str1, format, &condition, &c1, &c2, &c3));
  DESTROY(str1);
  DESTROY(format);
  printf("%c %c %c\n", (char) c1, (char) c2, c3);

  TEST(strcreate_m(&format, "%s %ls %hs", 0, NULL));
  TEST(strcreate_m(&str1, "hello world today",0, NULL)); 
  TEST(strcreate_m(&str2,NULL,0,NULL));
  TEST(wstrcreate_m(&str3,NULL,0,NULL));
  TEST(strcreate_m(&str4, NULL, 0, NULL));
  TEST(sscanf_m(str1, format, &condition, &str2, &str3, &str4));
  printf("%s %ls %s\n", str2->str.cstr, str3->str.wstr, str4->str.cstr);
  DESTROY(format);
  DESTROY(str1);
  DESTROY(str2);
  DESTROY(str3);
  DESTROY(str4);

  TEST(strcreate_m(&format, "%2s %ls %4hs", 0, NULL));
  TEST(wstrcreate_m(&str1, L"hello world today",0, NULL)); 
  TEST(wstrcreate_m(&str2,NULL,0,NULL));
  TEST(wstrcreate_m(&str3,NULL,0,NULL));
  TEST(strcreate_m(&str4, NULL, 0, NULL));
  TEST(sscanf_m(str1, format, &condition, &str2, &str3, &str4));
  printf("%ls %ls %s\n", (wchar_t*) str2->str.cstr, str3->str.wstr, str4->str.cstr);
  DESTROY(format);
  DESTROY(str1);
  DESTROY(str2);
  DESTROY(str3);
  DESTROY(str4);

  TEST(strcreate_m(&format, "%*2s %ls %4hs", 0, NULL));
  TEST(wstrcreate_m(&str1, L"hello world today",0, NULL)); 
  TEST(wstrcreate_m(&str2,NULL,0,NULL));
  TEST(strcreate_m(&str4, NULL, 0, NULL));
  TEST(sscanf_m(str1, format, &condition, &str2, &str4));
  printf("%ls %s\n", (wchar_t*) str2->str.cstr, str4->str.cstr);
  DESTROY(format);
  DESTROY(str1);
  DESTROY(str2);
  DESTROY(str4);

  TEST(strcreate_m(&format, "%2s %hs %4hs", 0, NULL));
  TEST(wstrcreate_m(&str1, L"hello world today",0, NULL)); 
  TEST(wstrcreate_m(&str2,NULL,0,NULL));
  TEST(wstrcreate_m(&str3,NULL,0,NULL));
  TEST(wstrcreate_m(&str4, NULL, 0, NULL));
  TEST(sscanf_m(str1, format, &condition, &str2, &str3, &str4));
  printf("%ls %s %s\n", (wchar_t*) str2->str.cstr, (char*) str3->str.wstr, str4->str.cstr);
  DESTROY(format);
  DESTROY(str1);
  DESTROY(str2);
  DESTROY(str3);
  DESTROY(str4);

  TEST(strcreate_m(&format, "%2d", 0, NULL));
  TEST(strcreate_m(&str1, "12345", 0, NULL));
  TEST(sscanf_m(str1, format, &condition, &i1));
  printf("%d\n", i1);
  DESTROY(str1);
  DESTROY(format);
  /*
    TEST(strcreate_m(&format, "%s", 0, NULL));
    TEST(strcreate_m(&str1, NULL, 0, NULL));
    printf("enter string: \n");
    TEST(fscanf_m(stdin, format, &condition, &str1));
    fflush(stdin);
    printf("%s", str1->str.cstr);
    DESTROY(str1);
    DESTROY(format);

    TEST(strcreate_m(&format, "%4d %2lu", 0, NULL));
    printf("enter 2 numbers: \n");
    TEST(fscanf_m(stdin, format, &condition, &i1, &l));
    fflush(stdin);
    printf("%d %u\n", i1, l);
    DESTROY(format);
	
    printf("testing fscanf_m on input.txt\n");
    condition = fopen_s(&f,"input.txt", "r");
    if(condition != 0){
    fprintf(stderr, "Error opening input.txt with return code %d\n", condition);
    } else {
    fwide(f, 1);
    ungetwc(getwc(f), f);
    TEST(strcreate_m(&format, "%s %4d %lu\n", 0, NULL));
    TEST(wstrcreate_m(&str1, NULL, 0, NULL));
    TEST(fscanf_m(f, format, &condition, &str1, &i1, &l));
    printf("%s %d %u\n", str1->str.wstr, i1, l);
    DESTROY(str1);
    DESTROY(format);

    fclose(f);
    }
  */


  return 0;
}

void strcreate_test(){
  errno_t	rv;
  _Bool   	condition;

  string_m str1 = NULL;
  string_m str2 = NULL;
  string_m str3 = NULL;

  int iserr=0;

  printf("Testing strcreate_m function...\n");
  // strcreate_m Case #1: Normal string
  printf("\nstrcreate_m() Case #1-->");
    
  TEST(strcreate_m(&str1, TEST_STRING1, 0, NULL));
  if (!rv) {
    rv = display(str1, TEST_STRING1);
    rv |= testlen(str1, strlen(TEST_STRING1));
    DESTROY(str1);
  }
  if (!rv) printf(" [success]\n");
  else printf (" [failure]\n");

  // strcreate_m Case #2: Empty string
  printf("strcreate_m() Case #2-->");    
    
  TEST(strcreate_m(&str2, "", 0, NULL));
  iserr = rv;
  if (!rv) {
    // check for null string
    TEST(isnull_m(str2, &condition));
    if (rv) iserr++;
    else if (condition) {
      printf("isNull(wrong) ");
      iserr++;
      nerror++;
    }

    // check for emptry string
    TEST(isempty_m(str2, &condition));
    if (rv) iserr++;
    else if (!condition) {
      printf("!isEmpty(wrong) ");
      iserr++;
      nerror++;
    }

    // print actual string
    iserr |= display(str2, "");
    iserr |= testlen(str2, strlen(""));
    DESTROY(str2);
  } // end else create str succeeded
  if (!iserr) printf (" [success]\n");
  else printf (" [failure]\n");

  // strcreate_m Case #3: NULL string
  printf("strcreate_m() Case #3-->");    
    
  TEST(strcreate_m(&str3, NULL, 0, NULL));
  iserr = rv;
  if (!rv) {
    // check for null string
    TEST(isnull_m(str3, &condition));
    if (!rv && !condition) {
      printf (" !isNull(wrong)");
      iserr++;
    }

    // check for emptry string
    TEST(isempty_m(str3, &condition));
    if (!rv && !condition) {
      printf (" !isEmpty(wrong)");
      iserr++;
    }

    // print actual string
    TEST(display(str3, NULL));
    iserr |= testlen(str3, 0);
    DESTROY(str3);
  }
  if (!iserr) printf (" [success]\n");
  else printf (" [failure]\n");

  printf("\n\nDone testing strcreate_m\n\n");
}

void strcpy_test(){
  errno_t	rv;

  string_m str1 = NULL;
  string_m str2 = NULL;
  string_m str3 = NULL;
  string_m str4 = NULL;

  int iserr=0;

  // strcpy_m Case Setup
  printf("\nSet up for strcpy_m Cases --->");

  iserr = 0;
  // Original String
  TEST(strcreate_m(&str1, TEST_STRING1, 0, NULL));
  // Larger size 
  TEST(strcreate_m(&str2, TEST_STRING2, 0, NULL));
  // Empty string
  TEST(strcreate_m(&str3, "", 0, NULL));
  // NULL string
  TEST(strcreate_m(&str4, NULL, 0, NULL));
  if (!iserr) printf (" [success]\n");
  else printf (" [failure]\n");

  // strcpy_m Case #1: Same size string copy
  printf("strcpy_m() Case #1-->");    
  TEST(strcpy_m(str2, str1));
  iserr = rv;
  if (!rv) {
    iserr |= display(str1, TEST_STRING1);
    iserr |= testlen(str1, strlen(TEST_STRING1));

    iserr |= display(str2, TEST_STRING1);
    iserr |= testlen(str2, strlen(TEST_STRING1));
  }
  if (!iserr) printf (" [success]\n");
  else printf (" [failure]\n");

  // strcpy_m Case #2: Copy to Empty string (Reallocation of destination)
  printf("strcpy_m() Case #2-->");    
  TEST(strcpy_m(str3, str1));
  iserr = rv;
  if (!rv) {
    iserr |= display(str1, TEST_STRING1);
    iserr |= testlen(str1, strlen(TEST_STRING1));

    iserr |= display(str3, TEST_STRING1);
    iserr |= testlen(str3, strlen(TEST_STRING1));
  }
  if (!iserr) printf (" [success]\n");
  else printf (" [failure]\n");

  // strcpy_m Case #3: Invalid string copy
  printf("strcpy_m() Case #3-->");    
  TEST(strcpy_m(str4, str1));
  iserr = rv;
  if (!rv) {
    iserr |= display(str1, TEST_STRING1);
    iserr |= testlen(str1, strlen(TEST_STRING1));

    iserr |= display(str4, TEST_STRING1);
    iserr |= testlen(str4, strlen(TEST_STRING1));
  }
  if (!iserr) printf (" [success]\n");
  else printf (" [failure]\n");

  printf("Tear down after strcpy_m Cases -->");
  iserr = 0;
  DESTROY(str1);
  DESTROY(str2);
  DESTROY(str3);
  DESTROY(str4);
  if (!iserr) printf (" [success]\n");
  else printf (" [failure]\n");

  str1 = str2 = str3 = str4 = NULL;
}

void strtok_test(){
  errno_t	rv;

  string_m str1 = NULL;
  string_m str2 = NULL;
  string_m str3 = NULL;
  string_m str4 = NULL;

  int iserr=0;

  printf("Testing strtok_m function....\n");
  // strtok_m Case #1: Normal string tokenization
  printf("\nstrtok_m() Case #1 -->");
  iserr = 0;
  TEST(strcreate_m(&str1, NULL, 0, NULL));
  TEST(strcreate_m(&str2, TEST_STRING1, 0, NULL));
  TEST(strcreate_m(&str3, " ", 0, NULL));
  TEST(strcreate_m(&str4, NULL, 0, NULL));
  TEST(strtok_m(str1, str2, str3, str4));
  if (!iserr) {
    iserr |= display(str1, "hello,");
    iserr |= testlen(str1, strlen("hello,"));

    iserr |= display(str4, "world.");
    iserr |= testlen(str4, strlen("world."));
  }
  DESTROY(str1);
  DESTROY(str2);
  DESTROY(str3);
  DESTROY(str4);
  if (!iserr) printf (" [success]\n");
  else printf (" [failure]\n");

  printf("\n\nDone testing strtok_m\n");
  str1 = str2 = str3 = str4 = NULL;
}

void setcharset_test(){
  errno_t	rv;
  int		iserr = 0;

  string_m 	str1 = NULL;
  string_m 	str2 = NULL;
  string_m 	str3 = NULL;

  printf("Testing setcharset()....\n");

  // setcharset() Case #1: create string with valid char set
  printf("setcharset() Case #1 -->");    
  TEST(strcreate_m(&str1, "aaabbbcccabc", 0, "abc"));
  if (!rv) {
    printf (" [success]\n");
  } else printf (" [failure]\n");

  // setcharset() Case #2: setcharset that invalidates existing string
  printf("setcharset() Case #2 -->"); 
  TESTFAIL(strcreate_m(&str2, "aaabbebcccabc", 0, "abc"), EINVAL);
  if (!rv) {
    printf (" [failure]\n");
  } else printf (" [success]\n");

  // setcharset() Case #3: concatenate string containing invalid chars
  printf("setcharset() Case #3 -->");
  TEST(strcreate_m(&str3, "aaadbbbecccabc", 0, "abcde"));
  iserr = 0;
  if (!rv) TESTFAIL(strcat_m(str1, str3), EINVAL);
  if (!iserr) printf (" [success]\n");
  else printf (" [failure]\n");
  errno = 0;

  // setcharset() cleanup
  printf("Cleaning up after setcharset test cases -->");    
  iserr = 0;
  DESTROY(str1);
  DESTROY(str2);
  DESTROY(str3);
  if (!iserr) printf (" [success]\n");
  else printf (" [failure]\n");

  printf("\n\nDone testing setcharset\n\n");
  str1 = str2 = str3 = NULL;
}

void sprintf_test(){
  errno_t	rv;
  int		iserr = 0;
  int		count = 0;
  string_m 	dest = NULL;
  string_m 	format = NULL;

  printf("Testing sprintf functions....\n");
  // vsprintf_m() test case
  printf ("sprintf_m Case #1A -->");
  dest = format = NULL;
  // Set-up
  iserr = 0;
  TEST(strcreate_m(&dest, NULL, 0, NULL));
  if (!iserr) TEST(strcreate_m(&format, "int = %d", 0, NULL));
  if (!iserr) TEST(sprintf_m(dest, format, &count, 7));
  if (!iserr) {
    const char *expected = "int = 7";
    iserr |= display(dest, expected);
    iserr |= testlen(dest, strlen(expected));
  }
  if (format) DESTROY(format);
  if (dest) DESTROY(dest);
  if (!iserr) printf (" [success]\n");
  else printf (" [failure]\n");


  printf ("sprintf_m Case #1B -->");
  dest = format = NULL;
  // Set-up
  iserr = 0;
  TEST(strcreate_m(&dest, NULL, 0, NULL));
  if (!iserr) TEST(strcreate_m(&format, "int = %3d", 0, NULL));
  if (!iserr) TEST(sprintf_m(dest, format,&count, 7));
  if (!iserr) {
    const char *expected = "int =   7";
    iserr |= display(dest, expected);
    iserr |= testlen(dest, strlen(expected));
  }
  if (format) DESTROY(format);
  if (dest) DESTROY(dest);
  if (!iserr) printf (" [success]\n");
  else printf (" [failure]\n");

  printf ("sprintf_m Case #1C -->");
  dest = format = NULL;
  // Set-up
  iserr = 0;
  TEST(strcreate_m(&dest, NULL, 0, NULL));
  if (!iserr) TEST(strcreate_m(&format, "int = %-3d", 0, NULL));
  if (!iserr) TEST(sprintf_m(dest, format, &count, 7));
  if (!iserr) {
    const char *expected = "int = 7  ";
    iserr |= display(dest, expected);
    iserr |= testlen(dest, strlen(expected));
  }
  if (format) DESTROY(format);
  if (dest) DESTROY(dest);
  if (!iserr) printf (" [success]\n");
  else printf (" [failure]\n");

  printf ("sprintf_m Case #1D -->");
  dest = format = NULL;
  // Set-up
  iserr = 0;
  TEST(strcreate_m(&dest, NULL, 0, NULL));
  if (!iserr) TEST(strcreate_m(&format, "int = %03d", 0, NULL));
  if (!iserr) TEST(sprintf_m(dest, format, &count, 7));
  if (!iserr) {
    const char *expected = "int = 007";
    iserr |= display(dest, expected);
    iserr |= testlen(dest, strlen(expected));
  }
  if (format) DESTROY(format);
  if (dest) DESTROY(dest);
  if (!iserr) printf (" [success]\n");
  else printf (" [failure]\n");

  printf ("sprintf_m Case #2 -->");
  dest = format = NULL;
  iserr = 0;
  TEST(strcreate_m(&format, "str = %s", 0, NULL));
  if (!iserr) TEST(strcreate_m(&dest, NULL, 0, NULL));
  if (!iserr) TEST(sprintf_m(dest, format, &count, format));
  if (!iserr) {
    const char *expected = "str = str = %s";
    iserr |= display(dest, expected);
    iserr |= testlen(dest, strlen(expected));
  }
  if (dest) DESTROY(dest);
  if (format) DESTROY(format);
  if (!iserr) printf (" [success]\n");
  else printf (" [failure]\n");

  iserr = 0;
  printf ("sprintf_m Case #3 -->");
  dest = format = NULL;
  TEST(strcreate_m(&format, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", 0, NULL));
  if (!iserr) TEST(strcreate_m(&dest, NULL, 0, NULL));
  if (!iserr) TEST(sprintf_m(dest, format, &count,  'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a'));
  if (!iserr) {
    const char *expected = "aaaaaaaaaaaaaaaaaaaaaaaaaaa";
    iserr |= display(dest, expected);
    iserr |= testlen(dest, strlen(expected));
  }
  if (dest) DESTROY(dest);
  if (format) DESTROY(format);
  if (!iserr) printf (" [success]\n");
  else printf (" [failure]\n");

  iserr = 0;
  printf ("sprintf_m Case #3 -->");
  dest = format = NULL;
  TEST(strcreate_m(&format, "%c%c", 0, NULL));
  if (!iserr) TEST(strcreate_m(&dest, NULL, 0, "ab"));
  if (!iserr) TESTFAIL(sprintf_m(dest, format, &count, 'a', 'c'), EINVAL);
  if (!iserr) {
    const char *expected = NULL;
    iserr |= display(dest, expected);
    iserr |= testlen(dest, 0);
  }
  if (dest) DESTROY(dest);
  if (format) DESTROY(format);
  if (!iserr) printf (" [success]\n");
  else printf (" [failure]\n");

  iserr = 0;
  printf("sprintf_m %%f test -->");
  dest = format = NULL;
  TEST(strcreate_m(&format, "%f", 0, NULL));
  if(!iserr) TEST(strcreate_m(&dest, NULL, 0, NULL));
  if(!iserr) TEST(sprintf_m(dest,format, &count, .0111));
  if(!iserr) {
    const char *expected = ".011100";
    iserr |= display(dest,expected);
    iserr |= testlen(dest, strlen(expected));
  }
  if(dest) DESTROY(dest);
  if(format) DESTROY(format);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("\n\nDone testing sprintf_m\n\n");
}

void strdelete_test(){
  errno_t	rv;
  int		iserr = 0;

  string_m	str1 = NULL;

  iserr = 0;
  printf ("strdelete_m Case #1 -->");
  str1 = NULL;
  TEST(strdelete_m(&str1));
  TESTFAIL(strdelete_m(NULL), EINVAL);
  TEST(strcreate_m(&str1, "%c%c", 0, NULL));
  DESTROY(str1);
  if (!iserr) printf (" [success]\n");
  else printf (" [failure]\n");
}

void strncat_test(){
  errno_t	rv;
  int		iserr = 0;

  string_m str1 = NULL;
  string_m str2 = NULL;


  // strncat_m testing
  printf("strncat_m Case #1 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, NULL, 0, NULL));
  TEST(strcreate_m(&str2, TEST_STRING1, 0, NULL));
  TEST(strncat_m(str1, str2, strlen(TEST_STRING1)+1));
  if(!rv){
    iserr |= display(str1, TEST_STRING1);
    iserr |= testlen(str1, strlen(TEST_STRING1));
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strncat_m Case #2 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, TEST_STRING1, 0, NULL));
  TEST(strcreate_m(&str2, NULL, 0, NULL));
  TEST(strncat_m(str1, str2, strlen(TEST_STRING1)+1));
  if(!rv){
    iserr |= display(str1, TEST_STRING1);
    iserr |= testlen(str1, strlen(TEST_STRING1));
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strncat_m Case #3 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, TEST_STRING1, 0, NULL));
  TEST(strcreate_m(&str2, TEST_STRING1, 0, NULL));
  TEST(strncat_m(str1, str2, strlen(TEST_STRING1)));
  if(!rv){
    iserr |= display(str1, "hello, world.hello, world.");
    iserr |= testlen(str1, strlen("hello, world.hello, world."));
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strncat_m Case #4 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, TEST_STRING1, 0, NULL));
  TEST(strcreate_m(&str2, TEST_STRING1, 0, NULL));
  TEST(strncat_m(str1, str2, 0));
  if(!rv){
    iserr |= display(str1, TEST_STRING1);
    iserr |= testlen(str1, strlen(TEST_STRING1));
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strncat_m Case #5 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, TEST_STRING1, 0, NULL));
  TEST(strcreate_m(&str2, TEST_STRING2, 0, NULL));
  TEST(strncat_m(str1, str2, 5));
  if(!rv){
    iserr |= display(str1, "hello, world.hello");
    iserr |= testlen(str1, strlen("hello, world.hello"));
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strncat_m Case #6 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, "aaaaa", 0, "a"));
  TEST(strcreate_m(&str2, "aabbb", 0, NULL));
  TESTFAIL(strncat_m(str1, str2, 3), EINVAL);
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");

  printf("strncat_m Case #7 --->");
  iserr = 0;
  TEST(strcreate_m(&str1, "aaaa", 0, "a"));
  TEST(strcreate_m(&str2, "aaaabb", 0, NULL));
  TEST(strncat_m(str1, str2, 4));
  if(!rv){
    iserr |= display(str1, "aaaaaaaa");
    iserr |= testlen(str1, strlen("aaaaaaaa"));
  }
  DESTROY(str1);
  DESTROY(str2);
  if(!iserr) printf(" [success]\n");
  else printf(" [failure]\n");
}
