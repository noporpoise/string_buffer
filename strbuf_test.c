/*
 sb_test.c
 project: string_buffer
 url: https://github.com/noporpoise/StringBuffer
 author: Isaac Turner <turner.isaac@gmail.com>

 Copyright (c) 2013, Isaac Turner
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>

#include "string_buffer.h"

#define MAX(x,y) ((x) >= (y) ? (x) : (y))
#define MIN(x,y) ((x) <= (y) ? (x) : (y))

//
// Tests
//
char *suite_name;
char suite_pass;
int suites_run = 0, suites_failed = 0;
int tests_run = 0, tests_failed = 0;

#define QUOTE(str) #str
#define ASSERT(x) {tests_run++; if(!(x)) \
  { warn("failed assert [%s:%i] %s", __FILE__, __LINE__, QUOTE(x)); \
    suite_pass = 0; tests_failed++; }}

#define SUITE_START(x) {suite_pass = 1; suite_name = x; suites_run++;}

#define SUITE_END() printf("Testing %s: %s\n", \
                           suite_name, suite_pass ? "Pass" : "Fail"); \
                    if(!suite_pass) { suites_failed++;}

//
// Utility functions
//

void die(const char *fmt, ...)
__attribute__((format(printf, 1, 2)))
__attribute__((noreturn));

void warn(const char *fmt, ...)
__attribute__((format(printf, 1, 2)));

void die(const char *fmt, ...)
{
  fflush(stdout);

  // Print error
  fprintf(stderr, "Error: ");

  va_list argptr;
  va_start(argptr, fmt);
  vfprintf(stderr, fmt, argptr);
  va_end(argptr);

  // Check if we need to print a newline
  if(*(fmt + strlen(fmt) - 1) != '\n')
  {
    fprintf(stderr, "\n");
  }

  exit(EXIT_FAILURE);
}

void warn(const char *fmt, ...)
{
  fflush(stdout);

  // Print warning
  fprintf(stderr, "Warning: ");

  va_list argptr;
  va_start(argptr, fmt);
  vfprintf(stderr, fmt, argptr);
  va_end(argptr);

  // Check if we need to print a newline
  if(*(fmt + strlen(fmt) - 1) != '\n')
  {
    fprintf(stderr, "\n");
  }

  fflush(stderr);
}


/* Tests! */

void test_clone()
{
  SUITE_START("clone");

  SUITE_END();
}

void test_reset()
{
  SUITE_START("reset");

  SUITE_END();
}

// Test resize and ensure_capacity
// Test that resize can shrink strbuf safely
void test_resize()
{
  SUITE_START("resize");

  SUITE_END();
}

void test_get_set_char()
{
  SUITE_START("get_char / set_char");

  SUITE_END();
}

void test_set()
{
  SUITE_START("set");

  SUITE_END();
}

void test_as_str()
{
  SUITE_START("as_str");

  SUITE_END();
}

void test_append()
{
  SUITE_START("append_char / append_buff / append_str / append_strn");

  SUITE_END();
}

void test_chomp()
{
  SUITE_START("chomp");

  SUITE_END();
}

void test_reverse()
{
  SUITE_START("reverse");

  SUITE_END();
}

void test_substr()
{
  SUITE_START("substr");

  SUITE_END();
}

void test_change_case()
{
  SUITE_START("uppercase / lowercase");

  SUITE_END();
}

void test_copy()
{
  SUITE_START("copy");

  SUITE_END();
}

void test_insert()
{
  SUITE_START("insert");

  SUITE_END();
}

void test_sprintf()
{
  SUITE_START("sprintf");

  SUITE_END();
}

void test_sprintf_at()
{
  SUITE_START("sprintf_at");

  SUITE_END();
}

void test_sprintf_noterm()
{
  SUITE_START("sprintf_noterm");

  SUITE_END();
}

void test_read_gzfile()
{
  SUITE_START("gzreadline / gzskipline");

  SUITE_END();
}

void test_read_file()
{
  SUITE_START("readline / skipline");

  SUITE_END();
}

void test_trim()
{
  SUITE_START("trim");

  SUITE_END();
}

/* Non-function tests */

void test_sscanf()
{
  SUITE_START("using sscanf");

  SUITE_END();
}

/* Old test */

void _test_trim_old(const char* str)
{
  size_t len = strlen(str);
  char* str_cpy = (char*) malloc(len+1);
  strcpy(str_cpy, str);
  char* new_str = string_trim(str_cpy);

  printf("trim('%s'): '%s'\n", str, new_str);
  free(str_cpy);
}

void test_trim_old()
{
  printf("Test trim:\n");
  _test_trim_old("  \t asdf asdf \n ");
  _test_trim_old("a");
  _test_trim_old("");
}

void test_all_whitespace_old()
{
  printf("Test string_is_all_whitespace:\n");
  char* str = "  \tasdf";
  printf("string_is_all_whitespace('%s'): %i\n", str, string_is_all_whitespace(str));
  str = "  \t ";
  printf("string_is_all_whitespace('%s'): %i\n", str, string_is_all_whitespace(str));
}

void _test_split_old(char* split, char* txt)
{
  char** results;
  
  printf("split '%s' by '%s': (", txt, split);
  
  long count = string_split(split, txt, &results);

  if(count > 0)
  {
    printf("'%s'", results[0]);
    free(results[0]);
  
    int i;
    for(i = 1; i < count; i++)
    {
      printf(", '%s'", results[i]);
      free(results[i]);
    }

    free(results);
  }

  printf(")\n");
}

void test_split_old()
{
  _test_split_old("/", "a/b");
  _test_split_old("/", "/");
  _test_split_old("/", "/b");
  _test_split_old("/", "a/");
  _test_split_old("/", "asdf");
  _test_split_old("/", "");
  _test_split_old("", "asdf");
  _test_split_old("", "");
}

void test_add_char_old()
{
  StrBuf* sbuf = strbuf_init(100);
  
  strbuf_append_char(sbuf, 'a');
  strbuf_append_char(sbuf, 'b');
  printf("'%s' (length: %lu)\n", sbuf->buff, sbuf->len);
  strbuf_free(sbuf);
}

void test_sprintf_old()
{
  printf("printf:\n");
  StrBuf* sbuf = strbuf_init(100);
  
  strbuf_sprintf(sbuf, "hi ello");
  printf("'%s' (length: %lu)\n", sbuf->buff, sbuf->len);
  
  strbuf_sprintf_noterm(sbuf, 0, "woot %i %s;", 12, "byebye");
  printf("'%s' (length: %lu)\n", sbuf->buff, sbuf->len);

  char *a = "wooo-%s-xx";
  char *b = "hihi";

  strbuf_sprintf_at(sbuf, sbuf->len, a, b);
  printf("'%s' (length: %lu)\n", sbuf->buff, sbuf->len);

  strbuf_reset(sbuf);
  strbuf_resize(sbuf, 10);
  strbuf_append_str(sbuf, "asdfasdf");

  strbuf_sprintf(sbuf, a, b);
  printf("'%s' (length: %lu)\n", sbuf->buff, sbuf->len);
  strbuf_free(sbuf);
}

void test_sscanf_old()
{
  StrBuf *sbuf = strbuf_new();
  char *input = "I'm sorry Dave I can't do that";
  
  strbuf_ensure_capacity(sbuf, strlen(input));
  sscanf(input, "I'm sorry %s I can't do that", sbuf->buff);
  sbuf->len = strlen(sbuf->buff);

  printf("Name: '%s'\n", sbuf->buff);

  strbuf_free(sbuf);
}

void test_insert_old()
{
  StrBuf *sbuf = strbuf_create("hello hello");
  strbuf_insert(sbuf, 6, sbuf->buff+3, 5);
  printf("result: '%s'\n", sbuf->buff);
}

int main()
{
  test_clone();
  test_reset();
  test_resize();
  test_get_set_char();
  test_set();
  test_as_str();
  test_append();
  test_chomp();
  test_reverse();
  test_substr();
  test_change_case();
  test_copy();
  test_insert();
  test_sprintf();
  test_sprintf_at();
  test_sprintf_noterm();
  test_read_gzfile();
  test_read_file();
  test_trim();
  test_sscanf();

  printf("\n");
  printf(" %i / %i suites failed\n", suites_failed, suites_run);
  printf(" %i / %i tests failed\n", tests_failed, tests_run);

  printf("\n THE END.\n");

  // Old tests
  // test_insert_old();
  // test_sscanf_old();
  // test_add_char_old();
  // test_sprintf_old();
  // test_all_whitespace_old();
  // test_split_old();
  // test_trim_old();
  
  return 0;
}
