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

#ifndef __constraint_h_ 
#define __constraint_h_ 1

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#ifdef _MSC_VER
#else
#include <stdint.h>
#endif

typedef int errno_t;
typedef size_t rsize_t;


typedef void (*constraint_handler_t)(const char *msg, const void *ptr, errno_t error);

extern constraint_handler_t ErrorHandler;

extern constraint_handler_t set_constraint_handler_s(constraint_handler_t handler);

extern void abort_handler_s(const char *msg, const void *ptr, errno_t error);
extern void ignore_handler_s(const char *msg, const void *ptr, errno_t error);
extern void strict_handler_s(const char *msg, const void *ptr, errno_t error);

#endif
