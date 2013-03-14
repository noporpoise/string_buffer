/*
 strbuf_test.c
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
#include <ctype.h>
#include <zlib.h>

#include "string_buffer.h"

#define MAX(x,y) ((x) >= (y) ? (x) : (y))
#define MIN(x,y) ((x) <= (y) ? (x) : (y))

const char tmp_file1[] = "tmp.strbuf.001.txt";
const char tmp_file2[] = "tmp.strbuf.002.txt";
const char tmp_gzfile1[] = "tmp.strbuf.001.txt.gz";
const char tmp_gzfile2[] = "tmp.strbuf.002.txt.gz";

/*********************/
/*  Testing toolkit  */
/*********************/

char *suite_name;
char suite_pass;
int suites_run = 0, suites_failed = 0, suites_empty = 0;
int tests_passed = 0, tests_failed = 0;
int total_tests_passed = 0, total_tests_failed = 0;

#define QUOTE(str) #str

#define ASSERT(x)  do {                                                        \
    if(x) tests_passed++;                                                      \
    else {                                                                     \
      warn("failed assert [%s:%i] %s", __FILE__, __LINE__, QUOTE(x));          \
      suite_pass = 0; tests_failed++;                                          \
    }                                                                          \
  } while(0)

#define SUITE_START(x) {suite_pass = 1; suite_name = x; \
                        suites_run++; tests_passed = tests_failed = 0;}

#define SUITE_END()  do { \
    printf("Testing %s ", suite_name);                                         \
    int suite_i, tests_in_suite = tests_passed+tests_failed;                   \
    for(suite_i = strlen(suite_name); suite_i<80-9-5; suite_i++) printf(".");  \
    printf("%s", suite_pass ? " pass" : " fail");                              \
    if(tests_in_suite == 0) { printf(" (empty)\n"); suites_empty++;}           \
    else if(tests_passed == 0 || tests_failed == 0) {                          \
      printf(" (%i)\n", tests_in_suite);                                       \
    } else printf(" [%i/%i]\n", tests_passed, tests_in_suite);                 \
    if(!suite_pass) suites_failed++;                                           \
    total_tests_failed += tests_failed;                                        \
    total_tests_passed += tests_passed;                                        \
  } while(0)

#define TEST_STATS()  do { \
    printf(" %i / %i suites failed\n", suites_failed, suites_run);             \
    printf(" %i / %i suites empty\n", suites_empty, suites_run);               \
    printf(" %i / %i tests failed\n", total_tests_failed,                      \
           total_tests_passed+total_tests_failed);                             \
  } while(0)

/* Test MACROs specifically for strbuf */

#define ASSERT_VALID(x) do {                 \
    ASSERT((x)->len == strlen((x)->buff));   \
    ASSERT((x)->len < (x)->capacity);        \
    ASSERT((x)->buff[(x)->len] == '\0');     \
  } while(0)

/**********************/
/*  Output functions  */
/**********************/

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
  if(*(fmt + strlen(fmt) - 1) != '\n') fprintf(stderr, "\n");

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
  if(*(fmt + strlen(fmt) - 1) != '\n') fprintf(stderr, "\n");

  fflush(stderr);
}


/* Tests! */

/************************/
/* Buffered input tests */
/************************/

void test_buffers()
{
  SUITE_START("buffers");

  // Test buffer_init, buffer_append_str, buffer_append_char etc

  buffer_t *buf = buffer_new(4);

  ASSERT(buf->begin == 0);
  ASSERT(buf->end == 0);
  ASSERT(buf->size >= 4);

  buffer_append_char(buf, 'a');
  buffer_append_char(buf, 'b');
  buffer_append_char(buf, 'c');
  buffer_append_char(buf, 'd');
  buffer_append_char(buf, 'e');

  ASSERT(buf->begin == 0);
  ASSERT(buf->end == 5);
  ASSERT(buf->size >= 6);
  ASSERT(strcmp(buf->b, "abcde") == 0);

  // Causes expansion -- tests ensure_capacity
  buffer_append_str(buf, "fghijklmnopqrstuvwxyz");

  ASSERT(buf->begin == 0);
  ASSERT(buf->end == 26);
  ASSERT(buf->size >= 27);
  ASSERT(strcmp(buf->b, "abcdefghijklmnopqrstuvwxyz") == 0);

  buffer_append_char(buf, '\r');
  buffer_append_char(buf, '\n');
  buffer_chomp(buf);

  ASSERT(buf->begin == 0);
  ASSERT(buf->end == 26);
  ASSERT(buf->size >= 27);
  ASSERT(strcmp(buf->b, "abcdefghijklmnopqrstuvwxyz") == 0);

  buffer_free(buf);

  SUITE_END();
}

typedef struct {
  char *text;
  size_t len, size;
} String;

String* string_new(size_t len)
{
  String *st = malloc(sizeof(String));
  st->text = malloc(sizeof(char)*(len+1));
  st->size = len+1;
  st->len = 0;
  st->text[0] = 0;
  return st;
}

void string_free(String *st)
{
  free(st->text);
  free(st);
}

// Compare buffered vs unbuffered + gzfile vs FILE
void test_buffered_reading()
{
  SUITE_START("buffered reading (getc/gets/readline/skipline)");

  // generate file
  char *tmp = malloc(10000);
  char *tmpptr = tmp;
  strcpy(tmpptr,"hi\nThis is\nOur file\r\n");
  tmpptr += strlen("hi\nThis is\nOur file\r\n");
  int i;
  for(i = 0; i < 1000; i++) *(tmpptr++) = 'a';
  *(tmpptr++) = '\n';
  strcpy(tmpptr, "That's all folks!");

  FILE *file1 = fopen(tmp_file1, "w");
  fputs2(file1, tmp);
  fclose(file1);

  FILE *file2 = fopen(tmp_file2, "w");
  fputs2(file2, tmp);
  fclose(file2);

  gzFile gzfile1 = gzopen(tmp_gzfile1, "w");
  gzputs(gzfile1, tmp);
  gzclose(gzfile1);

  gzFile gzfile2 = gzopen(tmp_gzfile2, "w");
  gzputs(gzfile2, tmp);
  gzclose(gzfile2);

  file1 = fopen(tmp_file1, "r");
  file2 = fopen(tmp_file2, "r");
  gzfile1 = gzopen(tmp_gzfile1, "r");
  gzfile2 = gzopen(tmp_gzfile2, "r");

  buffer_t *fbuf = buffer_new(12);
  buffer_t *gzbuf = buffer_new(12);

  String *st1 = string_new(10);
  String *st2 = string_new(10);
  String *st3 = string_new(10);
  String *st4 = string_new(10);

  // getc
  char c1 = fgetc(file1);
  char c2 = fgetc_buf(file2, fbuf);
  char c3 = gzgetc(gzfile1);
  char c4 = gzgetc_buf(gzfile2, gzbuf);

  ASSERT(c1 == 'h');
  ASSERT(c2 == 'h');
  ASSERT(c3 == 'h');
  ASSERT(c4 == 'h');

  // readline
  freadline(file1, &st1->text, &st1->len, &st1->size);
  freadline_buf(file2, fbuf, &st2->text, &st2->len, &st2->size);
  gzreadline(gzfile1, &st3->text, &st3->len, &st3->size);
  gzreadline_buf(gzfile2, gzbuf, &st4->text, &st4->len, &st4->size);

  ASSERT(strcmp(st1->text, "i\n") == 0);
  ASSERT(strcmp(st2->text, "i\n") == 0);
  ASSERT(strcmp(st3->text, "i\n") == 0);
  ASSERT(strcmp(st4->text, "i\n") == 0);
  ASSERT(st1->len == 2);
  ASSERT(st2->len == 2);
  ASSERT(st3->len == 2);
  ASSERT(st4->len == 2);

  const char *lines[] = {"This is\n","Our file\r\n"};

  st1->len = st2->len = st3->len = st4->len = 0;

  // readline
  freadline(file1, &st1->text, &st1->len, &st1->size);
  freadline_buf(file2, fbuf, &st2->text, &st2->len, &st2->size);
  gzreadline(gzfile1, &st3->text, &st3->len, &st3->size);
  gzreadline_buf(gzfile2, gzbuf, &st4->text, &st4->len, &st4->size);

  ASSERT(strcmp(st1->text, lines[0]) == 0);
  ASSERT(strcmp(st2->text, lines[0]) == 0);
  ASSERT(strcmp(st3->text, lines[0]) == 0);
  ASSERT(strcmp(st4->text, lines[0]) == 0);
  ASSERT(st1->len == strlen(lines[0]));
  ASSERT(st2->len == strlen(lines[0]));
  ASSERT(st3->len == strlen(lines[0]));
  ASSERT(st4->len == strlen(lines[0]));

  // skipline
  fskipline(file1);
  fskipline_buf(file2, fbuf);
  gzskipline(gzfile1);
  gzskipline_buf(gzfile2, gzbuf);

  // gets
  fgets2(file1, st1->text, 10);
  fgets_buf(file2, fbuf, st2->text, 10);
  gzgets2(gzfile1, st3->text, 10);
  gzgets_buf(gzfile2, gzbuf, st4->text, 10);

  ASSERT(strcmp(st1->text, "aaaaaaaaa") == 0);
  ASSERT(strcmp(st2->text, "aaaaaaaaa") == 0);
  ASSERT(strcmp(st3->text, "aaaaaaaaa") == 0);
  ASSERT(strcmp(st4->text, "aaaaaaaaa") == 0);

  st1->len = strlen(st1->text);
  st2->len = strlen(st1->text);
  st3->len = strlen(st1->text);
  st4->len = strlen(st1->text);

  // readline
  freadline(file1, &st1->text, &st1->len, &st1->size);
  freadline_buf(file2, fbuf, &st2->text, &st2->len, &st2->size);
  gzreadline(gzfile1, &st3->text, &st3->len, &st3->size);
  gzreadline_buf(gzfile2, gzbuf, &st4->text, &st4->len, &st4->size);

  for(i = 0; i < 1000; i++)
  {
    ASSERT(st1->text[i] == 'a');
    ASSERT(st2->text[i] == 'a');
    ASSERT(st3->text[i] == 'a');
    ASSERT(st4->text[i] == 'a');
  }
  ASSERT(st1->text[i] == '\n');
  ASSERT(st2->text[i] == '\n');
  ASSERT(st3->text[i] == '\n');
  ASSERT(st4->text[i] == '\n');

  st1->len = st2->len = st3->len = st4->len = 0;

  // gets
  fgets2(file1, st1->text, st1->size);
  fgets_buf(file2, fbuf, st2->text, st2->size);
  gzgets2(gzfile1, st3->text, st3->size);
  gzgets_buf(gzfile2, gzbuf, st4->text, st4->size);

  ASSERT(strcmp(st1->text, "That's all folks!") == 0);
  ASSERT(strcmp(st2->text, "That's all folks!") == 0);
  ASSERT(strcmp(st3->text, "That's all folks!") == 0);
  ASSERT(strcmp(st4->text, "That's all folks!") == 0);

  // free
  string_free(st1);
  string_free(st2);
  string_free(st3);
  string_free(st4);

  // Check file/buffers empty
  ASSERT(fgetc(file1) == -1);
  ASSERT(fgetc_buf(file2, fbuf) == -1);
  ASSERT(gzgetc(gzfile1) == -1);
  ASSERT(gzgetc_buf(gzfile2, gzbuf) == -1);

  // close
  fclose(file1);
  fclose(file2);
  gzclose(gzfile1);
  gzclose(gzfile2);

  SUITE_END();
}


/***********************/
/* String buffer tests */
/***********************/

void _test_clone(const char *str)
{
  StrBuf *a = strbuf_create(str);
  StrBuf *b = strbuf_clone(a);

  ASSERT(strcmp(a->buff,b->buff) == 0);
  ASSERT(a->capacity == b->capacity);
  ASSERT(a->len == b->len);
  ASSERT_VALID(a);
  ASSERT_VALID(b);

  strbuf_free(a);
  strbuf_free(b);
}

void test_clone()
{
  SUITE_START("clone");

  _test_clone("");
  _test_clone("ASDFASDFASDFASDF");
  _test_clone("                                                               "
              "                                                               "
              "                                                               "
              "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
              "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
  _test_clone("0");
  _test_clone("\n");
  _test_clone(" ");

  SUITE_END();
}


void _test_reset(const char *str)
{
  StrBuf *a = strbuf_create(str);
  size_t capacity = a->capacity;
  strbuf_reset(a);

  ASSERT(a->buff[0] == '\0');
  ASSERT(a->len == 0);
  ASSERT(a->capacity == capacity);

  strbuf_free(a);
}

void test_reset()
{
  SUITE_START("reset");

  _test_reset("a");
  _test_reset("ab");
  _test_reset("abc");
  _test_clone("                                                              ");
  _test_reset("");

  SUITE_END();
}

// Test resize and ensure_capacity
// Test that resize can shrink strbuf safely
void _test_resize(const char *str, size_t new_len)
{
  StrBuf *sbuf = strbuf_create(str);
  ASSERT(strcmp(str, sbuf->buff) == 0);
  ASSERT_VALID(sbuf);

  strbuf_resize(sbuf, new_len);
  ASSERT_VALID(sbuf);
  ASSERT(sbuf->len == MIN(new_len, strlen(str)));
  ASSERT(strncmp(str, sbuf->buff, MIN(new_len, strlen(str))) == 0);

  strbuf_free(sbuf);
}
void test_resize()
{
  SUITE_START("resize");

  _test_resize("", 0);
  _test_resize("", 10000);
  _test_resize("abc", 10000);
  _test_resize("abc", 0);
  _test_resize("abc", 1);
  _test_resize("abc", 3);
  _test_resize("abcdefghijklmnopqrstuvwxyz", 0);
  _test_resize("abcdefghijklmnopqrstuvwxyz", 1);
  _test_resize("abcdefghijklmnopqrstuvwxyz", 10);
  _test_resize("abcdefghijklmnopqrstuvwxyz", 255);
  _test_resize("abcdefghijklmnopqrstuvwxyz", 10000);

  SUITE_END();
}

void test_get_set_char()
{
  SUITE_START("get_char / set_char");

  StrBuf *sbuf = strbuf_create("abcd");

  strbuf_set_char(sbuf, 0, 'z');
  strbuf_set_char(sbuf, 1, 'y');
  ASSERT(strcmp(sbuf->buff, "zycd") == 0);
  strbuf_set_char(sbuf, 2, 'x');
  strbuf_set_char(sbuf, 3, 'w');
  ASSERT(strcmp(sbuf->buff, "zyxw") == 0);
  strbuf_set_char(sbuf, 4, 'v');
  strbuf_set_char(sbuf, 5, 'u');
  ASSERT(strcmp(sbuf->buff, "zyxwvu") == 0);

  ASSERT(strbuf_get_char(sbuf, 0) == 'z');
  ASSERT(strbuf_get_char(sbuf, 1) == 'y');
  ASSERT(strbuf_get_char(sbuf, 2) == 'x');
  ASSERT(strbuf_get_char(sbuf, 3) == 'w');
  ASSERT(strbuf_get_char(sbuf, 4) == 'v');
  ASSERT(strbuf_get_char(sbuf, 5) == 'u');

  strbuf_free(sbuf);

  SUITE_END();
}


void _test_set(StrBuf *sbuf, const char *str)
{
  strbuf_set(sbuf, str);
  ASSERT(strcmp(sbuf->buff, str) == 0);
  ASSERT_VALID(sbuf);
}
void test_set()
{
  SUITE_START("set");
  StrBuf *sbuf = strbuf_create("abcd");

  _test_set(sbuf, "abcd");
  _test_set(sbuf, "");
  _test_set(sbuf, "a");
  _test_set(sbuf, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJLKMNOPQRSTUVWXY");
  _test_set(sbuf, "ab");
  _test_set(sbuf, "abc");
  _test_set(sbuf, "");

  strbuf_free(sbuf);
  SUITE_END();
}

void _test_as_str(StrBuf *sbuf, const char *str)
{
  strbuf_set(sbuf, str);
  char *tmp = strbuf_as_str(sbuf);
  ASSERT(strcmp(str, sbuf->buff) == 0);
  ASSERT(strcmp(tmp, str) == 0);
  ASSERT_VALID(sbuf);
  free(tmp);
}
void test_as_str()
{
  SUITE_START("as_str");
  StrBuf *sbuf = strbuf_new();

  _test_as_str(sbuf, "");
  _test_as_str(sbuf, "a");
  _test_as_str(sbuf, "ab");
  _test_as_str(sbuf, "abc");
  _test_as_str(sbuf, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJLKMNOPQRSTUVWXY");
  _test_as_str(sbuf, "abc");

  strbuf_free(sbuf);
  SUITE_END();
}

void _test_append(StrBuf* sbuf, char c, char *str, char *str2, int n,
                  const StrBuf *append)
{
  size_t len = sbuf->len;
  size_t extend = append->len;
  size_t end = len + extend;

  strbuf_append_buff(sbuf, append);
  strbuf_append_char(sbuf, c);
  strbuf_append_str(sbuf, str);
  strbuf_append_strn(sbuf, str2, n);

  size_t str1len = strlen(str);

  ASSERT(strncmp(sbuf->buff+len, append->buff, extend) == 0);
  ASSERT(sbuf->buff[end] == c);
  ASSERT(strncmp(sbuf->buff+end+1, str, str1len) == 0);
  ASSERT(strncmp(sbuf->buff+end+1+str1len, str2, n) == 0);

  ASSERT(sbuf->len == len + 1 + str1len + n + extend);
  ASSERT_VALID(sbuf);
}
void test_append()
{
  SUITE_START("append_char / append_buff / append_str / append_strn");
  StrBuf *sbuf = strbuf_new();

  _test_append(sbuf, 'a', "", "", 0, sbuf);
  _test_append(sbuf, 'b', "a", "xxy", 1, sbuf);
  _test_append(sbuf, 'c', "a", "xxy", 3, sbuf);
  _test_append(sbuf, 'd', "abcdefghijklmno", "abcdefghijklmno", 0, sbuf);
  _test_append(sbuf, 'd', "abcdefghijklmno", "abcdefghijklmno", 15, sbuf);
  _test_append(sbuf, 'd', "", "", 0, sbuf);

  StrBuf *empty = strbuf_new();
  _test_append(sbuf, 'd', "", "", 0, empty);
  _test_append(empty, 'd', "", "", 0, sbuf);

  strbuf_free(empty);
  strbuf_free(sbuf);
  SUITE_END();
}


void _test_chomp(const char *str)
{
  size_t len = strlen(str);
  size_t trim = len;
  while(trim > 0 && (str[trim-1] == '\r' || str[trim-1] == '\n')) trim--;

  StrBuf *sbuf = strbuf_create(str);
  ASSERT_VALID(sbuf);
  size_t buf_len = sbuf->len;

  strbuf_chomp(sbuf);
  ASSERT_VALID(sbuf);
  size_t buf_trim = sbuf->len;

  ASSERT(buf_len == len);
  ASSERT(buf_trim == trim);

  strbuf_free(sbuf);
}
void test_chomp()
{
  SUITE_START("chomp");

  _test_chomp("\n");
  _test_chomp("");
  _test_chomp("\r\n");
  _test_chomp("asdfa\nasdf");
  _test_chomp("asdfa\n\r");
  _test_chomp("asdfa\r\n");
  _test_chomp("asdfa\n");
  _test_chomp("asdfa\n ");

  SUITE_END();
}

void _test_reverse(const char *str)
{
  size_t len = strlen(str);
  StrBuf *sbuf = strbuf_create(str);
  strbuf_reverse(sbuf);
  ASSERT(sbuf->len == len);
  ASSERT_VALID(sbuf);

  size_t i;
  for(i = 0; i < len; i++) ASSERT(str[i] == sbuf->buff[len-i-1]);

  strbuf_free(sbuf);
}
void test_reverse()
{
  SUITE_START("reverse");

  _test_reverse("");
  _test_reverse("ASDFASDF");
  _test_reverse("   ");
  _test_reverse("\n\n\n");
  _test_reverse("abcdefghijklmnopqrstuvwxyz");

  SUITE_END();
}


void _test_substr(const char *str, size_t start, size_t len)
{
  StrBuf *sbuf = strbuf_create(str);
  ASSERT(strcmp(sbuf->buff, str) == 0);
  ASSERT_VALID(sbuf);

  char *tmp = strbuf_substr(sbuf, start, len);
  ASSERT(strncmp(tmp, sbuf->buff+start, len) == 0);
  ASSERT(len == strlen(tmp));

  free(tmp);
  strbuf_free(sbuf);
}
void test_substr()
{
  SUITE_START("substr");

  _test_substr("", 0, 0);
  _test_substr("a", 0, 1);
  _test_substr("a", 0, 0);
  _test_substr("a", 1, 0);
  _test_substr("abcdef", 3, 0);
  _test_substr("abcdef", 3, 1);
  _test_substr("abcdef", 3, 3);
  _test_substr("abcdef", 0, 6);
  _test_substr("abcdef", 2, 2);
  _test_substr("abcdefghijklmnopqrstuvwxyz", 0, 1);
  _test_substr("abcdefghijklmnopqrstuvwxyz", 25, 1);
  _test_substr("abcdefghijklmnopqrstuvwxyz", 5, 5);

  SUITE_END();
}

void _test_change_case(const char *str)
{
  StrBuf *sbuf = strbuf_create(str);

  strbuf_to_uppercase(sbuf);
  ASSERT_VALID(sbuf);
  char *upper = strbuf_as_str(sbuf);

  strbuf_to_lowercase(sbuf);
  ASSERT_VALID(sbuf);
  char *lower = strbuf_as_str(sbuf);

  // Length checks
  size_t len = strlen(str);
  ASSERT(strlen(upper) == len);
  ASSERT(strlen(lower) == len);
  ASSERT(sbuf->len == len);

  size_t i;
  for(i = 0; i < len; i++) {
    ASSERT(upper[i] == toupper(str[i]));
    ASSERT(lower[i] == tolower(str[i]));
  }

  free(upper);
  free(lower);
  strbuf_free(sbuf);
}
void test_change_case()
{
  SUITE_START("uppercase / lowercase");

  _test_change_case("");
  _test_change_case("asdfasdf");
  _test_change_case("asdf");
  _test_change_case("ASDFASDF:. asdfasdf \nasdfasdf'aougyqvo23=-=12#");

  SUITE_END();
}


void _test_copy(StrBuf *sbuf, size_t pos, const char *from, size_t len)
{
  char *frmcpy = strdup(from);
  size_t orig_len = sbuf->len;

  char *orig = strbuf_as_str(sbuf);
  ASSERT_VALID(sbuf);
  ASSERT(strcmp(sbuf->buff, orig) == 0);

  strbuf_copy(sbuf, pos, from, len);

  ASSERT(sbuf->len == MAX(orig_len, pos+len));
  ASSERT_VALID(sbuf);

  ASSERT(strncmp(sbuf->buff, orig, pos) == 0);
  ASSERT(strncmp(sbuf->buff+pos, frmcpy, len) == 0);
  ASSERT(strncmp(sbuf->buff+pos+len, orig+pos+len, sbuf->len-pos-len) == 0);

  free(frmcpy);
  free(orig);
}
void test_copy()
{
  SUITE_START("copy");
  StrBuf *sbuf = strbuf_create("");

  _test_copy(sbuf, 0, "", 0);
  _test_copy(sbuf, 0, "asdf", 0);
  _test_copy(sbuf, 0, "asdf", 1);

  strbuf_set(sbuf, "");
  _test_copy(sbuf, 0, "asdf", 4);

  int i, j;
  for(i = 0; i <= 4; i++)
  {
    strbuf_set(sbuf, "asdf");
    _test_copy(sbuf, i, "asdf", 2);
    strbuf_set(sbuf, "asdf");
    _test_copy(sbuf, i, "asdf", 4);
  }

  strbuf_set(sbuf, "asdfasdfasdf");
  _test_copy(sbuf, 8, "df", 2);
  strbuf_set(sbuf, "asdfasdfasdf");
  _test_copy(sbuf, 8, "", 0);

  strbuf_set(sbuf, "asdfasdfasdf");
  _test_copy(sbuf, 8, sbuf->buff, sbuf->len);

  for(i = 0; i <= 4; i++)
  {
    for(j = 0; j <= 4; j++)
    {
      strbuf_set(sbuf, "asdf");
      _test_copy(sbuf, i, sbuf->buff, j);
    }
  }

  strbuf_free(sbuf);
  SUITE_END();
}

void _test_insert(StrBuf *sbuf, size_t pos, size_t len, const char *from)
{
  char *frmcpy = (from == NULL ? calloc(1,1) : strdup(from));
  size_t orig_len = sbuf->len;

  char *orig = strbuf_as_str(sbuf);
  ASSERT_VALID(sbuf);
  ASSERT(strcmp(sbuf->buff, orig) == 0);

  strbuf_insert(sbuf, pos, from, len);

  ASSERT(sbuf->len == orig_len + len);
  ASSERT(sbuf->len < sbuf->capacity);

  ASSERT(strncmp(sbuf->buff, orig, pos) == 0);
  ASSERT(strncmp(sbuf->buff+pos, frmcpy, len) == 0);
  ASSERT(strncmp(sbuf->buff+pos+len, orig+pos, orig_len-pos) == 0);

  free(frmcpy);
  free(orig);
}
void test_insert()
{
  SUITE_START("insert");
  StrBuf *sbuf = strbuf_create("");

  _test_insert(sbuf, 0, 0, NULL);
  _test_insert(sbuf, 0, 0, "");
  _test_insert(sbuf, 0, 0, "asdf");
  _test_insert(sbuf, 0, 1, "asdf");

  strbuf_set(sbuf, "");
  _test_insert(sbuf, 0, 4, "asdf");

  int i, j;
  for(i = 0; i <= 4; i++)
  {
    strbuf_set(sbuf, "asdf");
    _test_insert(sbuf, i, 2, "asdf");
    strbuf_set(sbuf, "asdf");
    _test_insert(sbuf, i, 4, "asdf");
  }

  strbuf_set(sbuf, "asdfasdfasdf");
  _test_insert(sbuf, 8, 2, "df");
  strbuf_set(sbuf, "asdfasdfasdf");
  _test_insert(sbuf, 8, 0, "");

  strbuf_set(sbuf, "asdfasdfasdf");
  _test_insert(sbuf, 8, sbuf->len, sbuf->buff);

  for(i = 0; i <= 4; i++)
  {
    for(j = 0; j <= 4; j++)
    {
      strbuf_set(sbuf, "asdf");
      _test_insert(sbuf, i, j, sbuf->buff);
    }
  }

  strbuf_set(sbuf, "abcdefghij");
  strbuf_insert(sbuf, 3, sbuf->buff+1, 5);
  ASSERT(strcmp(sbuf->buff, "abcbcdefdefghij") == 0);

  strbuf_free(sbuf);
  SUITE_END();
}

void test_overwrite()
{
  SUITE_START("overwrite");
  StrBuf *sbuf = strbuf_new();

  strbuf_set(sbuf, "aaabbccc");

  strbuf_overwrite(sbuf, 3, 2, "BBB", 3);
  ASSERT(strcmp(sbuf->buff, "aaaBBBccc") == 0);
  ASSERT_VALID(sbuf);

  strbuf_overwrite(sbuf, 3, 3, "_x", 1);
  ASSERT(strcmp(sbuf->buff, "aaa_ccc") == 0);
  ASSERT_VALID(sbuf);

  strbuf_set(sbuf, "abcdefghijklmnopqrstuvwxyz");
  // replace de with abcdef
  strbuf_overwrite(sbuf, 3, 2, sbuf->buff, 6);
  ASSERT(strcmp(sbuf->buff, "abcabcdeffghijklmnopqrstuvwxyz") == 0);
  ASSERT_VALID(sbuf);

  // replace abcdef with de
  strbuf_overwrite(sbuf, 3, 6, sbuf->buff+6, 2);
  ASSERT(strcmp(sbuf->buff, "abcdefghijklmnopqrstuvwxyz") == 0);
  ASSERT_VALID(sbuf);

  // do nothing
  strbuf_overwrite(sbuf, 3, 0, sbuf->buff+6, 0);
  ASSERT(strcmp(sbuf->buff, "abcdefghijklmnopqrstuvwxyz") == 0);
  ASSERT_VALID(sbuf);

  // delete b
  strbuf_overwrite(sbuf, 1, 1, sbuf->buff, 0);
  ASSERT(strcmp(sbuf->buff, "acdefghijklmnopqrstuvwxyz") == 0);
  ASSERT_VALID(sbuf);

  // swap ghij with hi
  strbuf_overwrite(sbuf, 5, 4, sbuf->buff+6, 2);
  ASSERT(strcmp(sbuf->buff, "acdefhiklmnopqrstuvwxyz") == 0);
  ASSERT_VALID(sbuf);

  // replace o with z
  strbuf_overwrite(sbuf, 11, 1, sbuf->buff+22, 1);
  ASSERT(strcmp(sbuf->buff, "acdefhiklmnzpqrstuvwxyz") == 0);
  ASSERT_VALID(sbuf);

  // replace pq with stuv
  strbuf_overwrite(sbuf, 12, 2, sbuf->buff+15, 4);
  ASSERT(strcmp(sbuf->buff, "acdefhiklmnzstuvrstuvwxyz") == 0);
  ASSERT_VALID(sbuf);

  // replace stuv with e
  strbuf_overwrite(sbuf, 12, 4, sbuf->buff+3, 1);
  ASSERT(strcmp(sbuf->buff, "acdefhiklmnzerstuvwxyz") == 0);
  ASSERT_VALID(sbuf);

  // replace lmn with "A"
  strbuf_overwrite(sbuf, 8, 3, "AB", 1);
  ASSERT(strcmp(sbuf->buff, "acdefhikAzerstuvwxyz") == 0);
  ASSERT_VALID(sbuf);

  // replace A with "XYZ"
  strbuf_overwrite(sbuf, 8, 1, "XYZ", 3);
  ASSERT(strcmp(sbuf->buff, "acdefhikXYZzerstuvwxyz") == 0);
  ASSERT_VALID(sbuf);

  // replace XYZ with "Zz"
  strbuf_overwrite(sbuf, 8, 3, sbuf->buff+10, 2);
  ASSERT(strcmp(sbuf->buff, "acdefhikZzzerstuvwxyz") == 0);
  ASSERT_VALID(sbuf);

  // replace zer with "zz"
  strbuf_overwrite(sbuf, 10, 3, sbuf->buff+9, 2);
  ASSERT(strcmp(sbuf->buff, "acdefhikZzzzstuvwxyz") == 0);
  ASSERT_VALID(sbuf);

  strbuf_free(sbuf);
  SUITE_END();
}

void test_delete()
{
  SUITE_START("delete");
  StrBuf *sbuf = strbuf_new();

  strbuf_set(sbuf, "aaaBBccc");

  strbuf_delete(sbuf, 3, 2);
  ASSERT(strcmp(sbuf->buff, "aaaccc") == 0);
  ASSERT_VALID(sbuf);

  strbuf_delete(sbuf, 3, 0);
  ASSERT(strcmp(sbuf->buff, "aaaccc") == 0);
  ASSERT_VALID(sbuf);

  strbuf_delete(sbuf, 0, 0);
  ASSERT(strcmp(sbuf->buff, "aaaccc") == 0);
  ASSERT_VALID(sbuf);

  strbuf_delete(sbuf, 0, 1);
  ASSERT(strcmp(sbuf->buff, "aaccc") == 0);
  ASSERT_VALID(sbuf);

  strbuf_delete(sbuf, 4, 1);
  ASSERT(strcmp(sbuf->buff, "aacc") == 0);
  ASSERT_VALID(sbuf);

  strbuf_delete(sbuf, 4, 0);
  ASSERT(strcmp(sbuf->buff, "aacc") == 0);
  ASSERT_VALID(sbuf);

  strbuf_free(sbuf);
  SUITE_END();
}

void test_sprintf()
{
  SUITE_START("sprintf");
  StrBuf *sbuf = strbuf_new();

  // although valid, GCC complains about formatted strings of length 0
  #ifdef __clang__
  strbuf_sprintf(sbuf, "");
  ASSERT(strcmp(sbuf->buff, "") == 0);
  ASSERT_VALID(sbuf);
  #endif

  strbuf_sprintf(sbuf, "hi. ");
  ASSERT(strcmp(sbuf->buff, "hi. ") == 0);
  ASSERT_VALID(sbuf);

  // Note: strbuf_sprintf appends -> so we still have 'hi. '
  strbuf_sprintf(sbuf, "A dozen is another way of saying %i, except for bakers "
                       "where it means %lu for some reason.  No other "
                       "profession is known to have its own dozen", 12, 13UL);

  ASSERT(strcmp(sbuf->buff,
                "hi. A dozen is another way of saying 12, except for bakers "
                "where it means 13 for some reason.  No other "
                "profession is known to have its own dozen") == 0);
  ASSERT_VALID(sbuf);

  strbuf_reset(sbuf);
  strbuf_sprintf(sbuf, "woot %s %i %c", "what excitement", 12, '?');
  ASSERT(strcmp(sbuf->buff, "woot what excitement 12 ?") == 0);
  ASSERT_VALID(sbuf);

  strbuf_reset(sbuf);
  strbuf_sprintf(sbuf, "bye");
  ASSERT(strcmp(sbuf->buff, "bye") == 0);
  ASSERT_VALID(sbuf);

  strbuf_free(sbuf);
  SUITE_END();
}

void test_sprintf_at()
{
  SUITE_START("sprintf_at");
  StrBuf *sbuf = strbuf_new();

  // although valid, GCC complains about formatted strings of length 0
  #ifdef __clang__
  strbuf_sprintf_at(sbuf, 0, "");
  ASSERT(strcmp(sbuf->buff, "") == 0);
  ASSERT_VALID(sbuf);
  #endif

  strbuf_sprintf_at(sbuf, 0, "hi. ");
  ASSERT(strcmp(sbuf->buff, "hi. ") == 0);
  ASSERT_VALID(sbuf);

  strbuf_sprintf_at(sbuf, 2, " bye. ");
  ASSERT(strcmp(sbuf->buff, "hi bye. ") == 0);
  ASSERT_VALID(sbuf);

  strbuf_sprintf_at(sbuf, 0, "woot %s %i %c", "what excitement", 12, '?');
  ASSERT(strcmp(sbuf->buff, "woot what excitement 12 ?") == 0);
  ASSERT_VALID(sbuf);

  strbuf_sprintf_at(sbuf, 5, "moo %i", 6);
  ASSERT(strcmp(sbuf->buff, "woot moo 6") == 0);
  ASSERT_VALID(sbuf);

  strbuf_free(sbuf);
  SUITE_END();
}

void test_sprintf_noterm()
{
  SUITE_START("sprintf_noterm");
  StrBuf *sbuf = strbuf_new();

  // although valid, GCC complains about formatted strings of length 0
  #ifdef __clang__
  strbuf_sprintf_noterm(sbuf, 0, "");
  ASSERT(strcmp(sbuf->buff, "") == 0);
  ASSERT_VALID(sbuf);
  #endif

  strbuf_sprintf_noterm(sbuf, 0, "hi. ");
  ASSERT(strcmp(sbuf->buff, "hi. ") == 0);
  ASSERT_VALID(sbuf);

  strbuf_sprintf_noterm(sbuf, 2, " bye. ");
  ASSERT(strcmp(sbuf->buff, "hi bye. ") == 0);
  ASSERT_VALID(sbuf);

  strbuf_sprintf_noterm(sbuf, 0, "woot %s %i %c", "what excitement", 12, '?');
  ASSERT(strcmp(sbuf->buff, "woot what excitement 12 ?") == 0);
  ASSERT_VALID(sbuf);

  strbuf_sprintf_noterm(sbuf, 5, "moo %i", 6);
  ASSERT(strcmp(sbuf->buff, "woot moo 6excitement 12 ?") == 0);
  ASSERT_VALID(sbuf);

  // although valid, GCC complains about formatted strings of length 0
  #ifdef __clang__
  StrBuf *sbuf2 = strbuf_clone(sbuf);
  strbuf_sprintf_noterm(sbuf, 5, "");
  ASSERT(strcmp(sbuf->buff, sbuf2->buff) == 0);
  ASSERT_VALID(sbuf2);
  strbuf_free(sbuf2);
  #endif

  strbuf_free(sbuf);
  SUITE_END();
}

#define ftest(fname,type_t,__open,__close,__puts,__readline,__skipline) \
  void fname(const char *path) \
  { \
    /* Generate file */ \
    int i; \
    type_t out = __open(path, "w"); \
    if(out == NULL) die("Couldn't open: %s", path); \
    __puts(out, "hi\nthis is\nour file\n"); \
    /* 4th line: print 1000x'a' */ \
    for(i = 0; i < 1000; i++) __puts(out, "a"); \
    __close(out); \
    type_t file = __open(path, "r"); \
    if(file == NULL) die("Couldn't open: %s", path); \
    StrBuf *line = strbuf_new(); \
    __readline(line, file); \
    ASSERT(strcmp(line->buff, "hi\n") == 0); \
    ASSERT(line->len == strlen(line->buff)); \
    ASSERT(line->len < line->capacity); \
    strbuf_chomp(line); \
    ASSERT(strcmp(line->buff, "hi") == 0); \
    ASSERT(line->len == strlen(line->buff)); \
    ASSERT(line->len < line->capacity); \
    __skipline(file); \
    strbuf_reset(line); \
    __readline(line, file); \
    ASSERT(strcmp(line->buff, "our file\n") == 0); \
    ASSERT(line->len == strlen(line->buff)); \
    ASSERT(line->len < line->capacity); \
    strbuf_chomp(line); \
    ASSERT(strcmp(line->buff, "our file") == 0); \
    ASSERT(line->len == strlen(line->buff)); \
    ASSERT(line->len < line->capacity); \
    strbuf_reset(line); \
    __readline(line, file); \
    ASSERT(line->len == 1000); \
    ASSERT(line->len < line->capacity); \
    for(i = 0; i < 1000; i++) { ASSERT(line->buff[i] == 'a'); } \
    ASSERT(line->buff[1000] == '\0'); \
    strbuf_free(line); \
    __close(file); \
  }

ftest(test_gzfile,gzFile,gzopen,gzclose,gzputs2,strbuf_gzreadline,strbuf_gzskipline)
ftest(test_file,FILE*,fopen,fclose,fputs2,strbuf_readline,strbuf_skipline)

void test_read_gzfile()
{
  SUITE_START("gzreadline / gzskipline");
  test_gzfile(tmp_gzfile1);
  SUITE_END();
}

void test_read_file()
{
  SUITE_START("readline / skipline");
  test_file(tmp_file1);
  SUITE_END();
}

void test_read_nonempty()
{
  SUITE_START("read nonempty");

  FILE *fh = fopen(tmp_file1, "w");
  if(fh == NULL) die("Cannot write tmp output file: %s", tmp_file1);
  fprintf(fh, "hi\n\r\n\r\n"
              "bye\nx\ny\n"
              "\n\n\nz\n\n\n");
  fclose(fh);

  fh = fopen(tmp_file1, "r");
  if(fh == NULL) die("Cannot read tmp output file: %s", tmp_file1);

  StrBuf *sbuf = strbuf_new();
  ASSERT(strbuf_readline_nonempty(sbuf, fh));
  strbuf_chomp(sbuf);
  ASSERT(strcmp(sbuf->buff,"hi")==0);

  strbuf_reset(sbuf);
  ASSERT(strbuf_readline_nonempty(sbuf, fh));
  strbuf_chomp(sbuf);
  ASSERT(strcmp(sbuf->buff,"bye")==0);

  strbuf_reset(sbuf);
  ASSERT(strbuf_readline(sbuf, fh));
  strbuf_chomp(sbuf);
  ASSERT(strcmp(sbuf->buff,"x")==0);

  strbuf_reset(sbuf);
  ASSERT(strbuf_readline_nonempty(sbuf, fh));
  strbuf_chomp(sbuf);
  ASSERT(strcmp(sbuf->buff,"y")==0);
  ASSERT_VALID(sbuf);

  strbuf_reset(sbuf);
  ASSERT(strbuf_readline(sbuf, fh));
  strbuf_chomp(sbuf);
  ASSERT(strcmp(sbuf->buff,"")==0);

  strbuf_reset(sbuf);
  ASSERT(strbuf_readline_nonempty(sbuf, fh));
  strbuf_chomp(sbuf);
  ASSERT(strcmp(sbuf->buff,"z")==0);

  strbuf_reset(sbuf);
  ASSERT(strbuf_readline_nonempty(sbuf, fh) == 0);
  ASSERT(sbuf->len == 0);

  strbuf_reset(sbuf);
  ASSERT(strbuf_readline(sbuf,fh) == 0);
  ASSERT(sbuf->len == 0);

  ASSERT_VALID(sbuf);
  strbuf_free(sbuf);
  fclose(fh);

  SUITE_END();
}

// test trim, ltrim, rtrim
// trim removes whitespace (isspace(c)) from both sides of a str
void _test_trim(const char *str, const char *ans)
{
  StrBuf *sbuf = strbuf_create(str);
  strbuf_trim(sbuf);
  ASSERT(strcmp(sbuf->buff, ans) == 0);
  ASSERT(sbuf->len == strlen(sbuf->buff));
  ASSERT(sbuf->len < sbuf->capacity);
  strbuf_free(sbuf);
}
void _test_trim2(const char *str, const char *alphabet, const char *ans,
                     void (*trim)(StrBuf *sbuf, const char* list))
{
  StrBuf *sbuf = strbuf_create(str);
  trim(sbuf, alphabet);
  ASSERT(strcmp(sbuf->buff, ans) == 0);
  ASSERT(sbuf->len == strlen(sbuf->buff));
  ASSERT(sbuf->len < sbuf->capacity);
  strbuf_free(sbuf);
}
void test_trim()
{
  SUITE_START("trim");

  // Trim whitespace from either side
  _test_trim("","");
  _test_trim("   ","");
  _test_trim("\r\n","");
  _test_trim("\r \n\t","");
  _test_trim(":\r\n\t.",":\r\n\t.");
  _test_trim("\r \n \t.:",".:");
  _test_trim(".:\r \n \t",".:");
  _test_trim(" abcdefghi\r\njklmn opqrst\tu\nvwxyz",
             "abcdefghi\r\njklmn opqrst\tu\nvwxyz");
  _test_trim(" abcdefghi\r\njklmn opqrst\tu\nvwxyz\n",
             "abcdefghi\r\njklmn opqrst\tu\nvwxyz");
  _test_trim("abcdefghi\r\njklmn opqrst\tu\nvwxyz",
             "abcdefghi\r\njklmn opqrst\tu\nvwxyz");

  // Trim a given alphabet from the left hand side
  _test_trim2("","","", strbuf_ltrim);
  _test_trim2("","abc","", strbuf_ltrim);
  _test_trim2("zabc","abc","zabc", strbuf_ltrim);
  _test_trim2("abacbz","abc","z", strbuf_ltrim);
  _test_trim2("ab:c\nadzb:d\n asdf","abc : \n","dzb:d\n asdf", strbuf_ltrim);

  // Trim a given alphabet from the right hand side
  _test_trim2("","","", strbuf_rtrim);
  _test_trim2("","abc","", strbuf_rtrim);
  _test_trim2("abcz","abc","abcz", strbuf_rtrim);
  _test_trim2("zabacb","abc","z", strbuf_rtrim);
  _test_trim2("ab:c:d\n asdfacb:\n  a","abc : \n", "ab:c:d\n asdf", strbuf_rtrim);

  SUITE_END();
}

/* Non-function tests */

void test_sscanf()
{
  SUITE_START("using sscanf");
  StrBuf *sbuf = strbuf_new();
  char *input = "I'm sorry Dave I can't do that";
  
  strbuf_ensure_capacity(sbuf, strlen(input));
  sscanf(input, "I'm sorry %s I can't do that", sbuf->buff);
  sbuf->len = strlen(sbuf->buff);

  ASSERT(strcmp(sbuf->buff, "Dave") == 0);

  strbuf_free(sbuf);
  SUITE_END();
}

/* Old tests */

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


int main()
{
  test_buffers();
  test_buffered_reading();

  test_clone();
  test_reset();
  test_resize();

  test_get_set_char();
  test_set();
  test_as_str();
  test_append();
  test_chomp();
  test_trim();
  test_reverse();
  test_substr();
  test_change_case();

  test_copy();
  test_insert();
  test_overwrite();
  test_delete();

  test_sprintf();
  test_sprintf_at();
  test_sprintf_noterm();

  test_read_gzfile();
  test_read_file();
  test_read_nonempty();

  test_sscanf();

  printf("\n");
  TEST_STATS();

  printf("\n THE END.\n");

  // Old tests
  // test_all_whitespace_old();
  // test_split_old();
  
  return 0;
}
