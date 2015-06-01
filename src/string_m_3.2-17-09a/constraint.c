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

#include "constraint.h"

constraint_handler_t ErrorHandler = strict_handler_s;


void strict_handler_s(const char *msg, const void *ptr, errno_t error) {
  fprintf(stderr, "ERROR %d: %s\n", error, msg);
  return;
}	

void abort_handler_s(const char *msg, const void *ptr, errno_t error){
  fprintf(stderr, "ERROR %d: %s\n", error, msg);
  abort();
}

void ignore_handler_s(const char *msg, const void *ptr, errno_t error){
  msg = msg;
  error = error;
  return;
}
