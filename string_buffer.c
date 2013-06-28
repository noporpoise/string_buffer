/*
 string_buffer.c
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

// POSIX required for kill signal to work
#define _XOPEN_SOURCE 700

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h> // kill on error
#include <ctype.h> // toupper() and tolower()

#include "string_buffer.h"

#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN_SIZE 16

#ifndef ROUNDUP2POW
  #define ROUNDUP2POW(x) (0x1UL << (64 - __builtin_clzl(x)))
#endif

/*********************/
/*  Bounds checking  */
/*********************/

// Bounds check when inserting (pos <= len are valid)
static void _bounds_check_insert(const StrBuf* sbuf, size_t pos,
                                 const char* file, int line, const char* func)
{
  if(pos > sbuf->len)
  {
    fprintf(stderr, "%s:%i:%s() - out of bounds error "
                    "[index: %zu, num_of_bits: %zu]\n",
            file, line, func, pos, sbuf->len);
    kill(getpid(), SIGABRT);
    // errno = EDOM;
    // exit(EXIT_FAILURE);
  }
}

// Bounds check when reading (pos < len are valid)
static void _bounds_check_read(const StrBuf* sbuf, size_t pos,
                               const char* file, int line, const char* func)
{
  if(pos >= sbuf->len)
  {
    fprintf(stderr, "%s:%i:%s() - out of bounds error "
                    "[index: %zu, num_of_bits: %zu]\n",
            file, line, func, pos, sbuf->len);
    kill(getpid(), SIGABRT);
    // errno = EDOM;
    // exit(EXIT_FAILURE);
  }
}

// Bounds check when reading a range (start+len < strlen is valid)
static void _bounds_check_read_range(const StrBuf *sbuf, size_t start, size_t len,
                                     const char* file, int line, const char* func)
{
  if(start + len > sbuf->len)
  {
    fprintf(stderr, "%s:%i:%s() - out of bounds error "
                    "[start: %zu; length: %zu; strlen: %zu; buf:%.*s%s]\n",
            file, line, func, start, len, sbuf->len,
            (int)MIN(5, sbuf->len), sbuf->buff, sbuf->len > 5 ? "..." : "");
    kill(getpid(), SIGABRT);
    // errno = EDOM;
    // exit(EXIT_FAILURE);
  }
}

/******************************/
/*  Constructors/Destructors  */
/******************************/

StrBuf* strbuf_init(size_t capacity)
{
  StrBuf* sbuf = malloc(sizeof(StrBuf));
  if(sbuf == NULL) return NULL;
  return strbuf_alloc(sbuf, capacity);
}

StrBuf* strbuf_create(const char* str)
{
  size_t str_len = strlen(str);
  StrBuf* sbuf = strbuf_init(str_len);
  if(sbuf == NULL) return sbuf;
  strcpy(sbuf->buff, str);
  sbuf->len = str_len;
  return sbuf;
}

StrBuf* strbuf_alloc(StrBuf *sbuf, size_t capacity)
{
  capacity = capacity < MIN_SIZE ? MIN_SIZE : ROUNDUP2POW(capacity+1);
  sbuf->buff = malloc(capacity * sizeof(char));
  sbuf->len = 0;
  sbuf->capacity = capacity;
  if(sbuf->buff == NULL) { free(sbuf); return NULL; }
  return sbuf;
}

void strbuf_dealloc(StrBuf *sbuf)
{
  free(sbuf->buff);
}

void strbuf_reset(StrBuf* sbuf)
{
  sbuf->len = 0;
  sbuf->buff[0] = '\0';
}

void strbuf_free(StrBuf* sbuf)
{
  free(sbuf->buff);
  free(sbuf);
}

StrBuf* strbuf_clone(const StrBuf* sbuf)
{
  // One byte for the string end / null char \0
  StrBuf* sbuf_cpy = strbuf_init(sbuf->len);
  if(sbuf_cpy == NULL) return NULL;
  strcpy(sbuf_cpy->buff, sbuf->buff);
  sbuf_cpy->buff[sbuf->len] = '\0';
  sbuf_cpy->len = sbuf->len;
  return sbuf_cpy;
}

// Get a copy of this StrBuf as a char array
// Returns NULL if not enough memory
char* strbuf_as_str(const StrBuf* sbuf)
{
  char* cpy = malloc((sbuf->len + 1) * sizeof(char));
  if(cpy == NULL) return NULL;

  memcpy(cpy, sbuf->buff, sbuf->len * sizeof(char));
  cpy[sbuf->len] = '\0';

  return cpy;
}

// Get / set characters

char strbuf_get_char(const StrBuf *sbuf, size_t index)
{
  _bounds_check_read(sbuf, index, __FILE__, __LINE__, "strbuf_get_char");
  return sbuf->buff[index];
}

void strbuf_set_char(StrBuf *sbuf, size_t index, char c)
{
  _bounds_check_insert(sbuf, index, __FILE__, __LINE__, "strbuf_set_char");

  if(index == sbuf->len)
  {
    // Extend
    strbuf_ensure_capacity(sbuf, sbuf->len + 1);
    sbuf->buff[sbuf->len++] = c;
    sbuf->buff[sbuf->len] = '\0';
  }
  else {
    sbuf->buff[index] = c;
  }
}

/******************************/
/*  Resize Buffer Functions   */
/******************************/

// Resize the buffer to have capacity to hold a string of length new_len
// (+ a null terminating character).  Can also be used to downsize the buffer's
// memory usage.  Returns 1 on success, 0 on failure.
char strbuf_resize(StrBuf *sbuf, size_t new_len)
{
  size_t capacity = ROUNDUP2POW(new_len+1);
  char *new_buff = realloc(sbuf->buff, capacity * sizeof(char));
  if(new_buff == NULL) return 0;

  sbuf->buff = new_buff;
  sbuf->capacity = capacity;

  if(sbuf->len > new_len)
  {
    // Buffer was shrunk - re-add null byte
    sbuf->len = new_len;
    sbuf->buff[sbuf->len] = '\0';
  }

  return 1;
}

// Ensure capacity for len characters plus '\0' character
void strbuf_ensure_capacity(StrBuf *sbuf, size_t size)
{
  if(sbuf->capacity <= size+1 && !strbuf_resize(sbuf, size))
  {
    fprintf(stderr, "%s:%i:Error: strbuf_ensure_capacity couldn't resize "
                    "buffer. [requested %zu bytes; capacity: %zu bytes]\n",
            __FILE__, __LINE__, size, sbuf->capacity);
    kill(getpid(), SIGABRT);
    // exit(EXIT_FAILURE);
  }
}

static void _ensure_capacity_update_ptr(StrBuf *sbuf, size_t size,
                                        const char **ptr)
{
  if(sbuf->capacity <= size+1)
  {
    size_t oldcap = sbuf->capacity;
    char *oldbuf = sbuf->buff;

    if(!strbuf_resize(sbuf, size))
    {
      fprintf(stderr, "%s:%i:Error: _ensure_capacity_update_ptr couldn't resize "
                      "buffer. [requested %zu bytes; capacity: %zu bytes]\n",
              __FILE__, __LINE__, size, sbuf->capacity);
      kill(getpid(), SIGABRT);
      // exit(EXIT_FAILURE);
    }

    // ptr may have pointed to sbuf, which has now moved
    if(*ptr >= oldbuf && *ptr < oldbuf + oldcap) {
      *ptr = sbuf->buff + (*ptr - oldbuf);
    }
  }
}

void strbuf_shrink(StrBuf *sbuf, size_t new_len)
{
  if(new_len > sbuf->len) {
    fprintf(stderr, "%s:%i:Error: strbuf_shrink arg longer than length "
                    "[new_len: %zu; cur_len: %zu]\n",
            __FILE__, __LINE__, new_len, sbuf->len);
    kill(getpid(), SIGABRT);
    // exit(EXIT_FAILURE);
  }

  sbuf->len = new_len;
  sbuf->buff[sbuf->len] = '\0';
}

/********************/
/* String functions */
/********************/

// Set string buffer to contain a given string
// The string can be a string within the given string buffer
void strbuf_set(StrBuf *sbuf, const char *str)
{
  size_t len = strlen(str);
  _ensure_capacity_update_ptr(sbuf, len, &str);

  // Use memmove to allow overlapping strings
  memmove(sbuf->buff, str, len * sizeof(char));

  sbuf->buff[len] = '\0';
  sbuf->len = len;
}

// src may point to this buffer
void strbuf_append_strn(StrBuf* sbuf, const char* src, size_t len)
{
  // plus 1 for '\0'
  _ensure_capacity_update_ptr(sbuf, sbuf->len + len, &src);
  memcpy(sbuf->buff+sbuf->len, src, len * sizeof(char));
  sbuf->len += len;
  sbuf->buff[sbuf->len] = '\0';
}

void strbuf_append_char(StrBuf* sbuf, char c)
{
  // Adding one character
  strbuf_ensure_capacity(sbuf, sbuf->len + 1);
  sbuf->buff[sbuf->len++] = c;
  sbuf->buff[sbuf->len] = '\0';
}

// Remove \r and \n characters from the end of this StrBuf
// Returns the number of characters removed
size_t strbuf_chomp(StrBuf *sbuf)
{
  size_t old_len = sbuf->len;
  sbuf->len = string_chomp(sbuf->buff, sbuf->len);
  return old_len - sbuf->len;
}

// Reverse a string
void strbuf_reverse(StrBuf *sbuf)
{
  string_reverse_region(sbuf->buff, sbuf->len);
}

char* strbuf_substr(const StrBuf *sbuf, size_t start, size_t len)
{
  _bounds_check_read_range(sbuf, start, len, __FILE__, __LINE__, "strbuf_substr");

  char* new_string = malloc((len+1) * sizeof(char));
  strncpy(new_string, sbuf->buff + start, len);
  new_string[len] = '\0';

  return new_string;
}

void strbuf_to_uppercase(StrBuf *sbuf)
{
  char* pos;
  char* end = sbuf->buff + sbuf->len;
  for(pos = sbuf->buff; pos < end; pos++) *pos = toupper(*pos);
}

void strbuf_to_lowercase(StrBuf *sbuf)
{
  char* pos;
  char* end = sbuf->buff + sbuf->len;
  for(pos = sbuf->buff; pos < end; pos++) *pos = tolower(*pos);
}

// Copy a string to this StrBuf, overwriting any existing characters
// Note: dst_pos + len can be longer the the current dst StrBuf
void strbuf_copy(StrBuf* dst, size_t dst_pos, const char* src, size_t len)
{
  if(src == NULL || len == 0) return;

  _bounds_check_insert(dst, dst_pos, __FILE__, __LINE__, "strbuf_copy");

  // Check if dst buffer can handle string
  // src may have pointed to dst, which has now moved
  size_t newlen = MAX(dst_pos + len, dst->len);
  _ensure_capacity_update_ptr(dst, newlen, &src);

  // memmove instead of strncpy, as it can handle overlapping regions
  memmove(dst->buff+dst_pos, src, len * sizeof(char));

  if(dst_pos + len > dst->len)
  {
    // Extended string - add '\0' char
    dst->len = dst_pos + len;
    dst->buff[dst->len] = '\0';
  }
}

// Insert: copy to a StrBuf, shifting any existing characters along
void strbuf_insert(StrBuf* dst, size_t dst_pos, const char* src, size_t len)
{
  if(src == NULL || len == 0) return;

  _bounds_check_insert(dst, dst_pos, __FILE__, __LINE__, "strbuf_insert");

  // Check if dst buffer has capacity for inserted string plus \0
  // src may have pointed to dst, which will be moved in realloc when
  // calling ensure capacity
  _ensure_capacity_update_ptr(dst, dst->len + len, &src);

  char *insert = dst->buff+dst_pos;

  // dst_pos could be at the end (== dst->len)
  if(dst_pos < dst->len)
  {
    // Shift some characters up
    memmove(insert + len, insert, (dst->len - dst_pos) * sizeof(char));

    if(src >= dst->buff && src < dst->buff + dst->capacity)
    {
      // src/dst strings point to the same string in memory
      if(src < insert) memmove(insert, src, len * sizeof(char));
      else if(src > insert) memmove(insert, src+len, len * sizeof(char));
    }
    else memmove(insert, src, len * sizeof(char));
  }
  else memmove(insert, src, len * sizeof(char));

  // Update size
  dst->len += len;
  dst->buff[dst->len] = '\0';
}

// Overwrite dst_pos..(dst_pos+dst_len-1) with src_len chars from src
// if dst_len != src_len, content to the right of dst_len is shifted
// Example:
//   strbuf_set(sbuf, "aaabbccc");
//   char *data = "xxx";
//   strbuf_overwrite(sbuf,3,2,data,strlen(data));
//   // sbuf is now "aaaxxxccc"
//   strbuf_overwrite(sbuf,3,2,"_",1);
//   // sbuf is now "aaa_ccc"
void strbuf_overwrite(StrBuf *dst, size_t dst_pos, size_t dst_len,
                      const char *src, size_t src_len)
{
  _bounds_check_read_range(dst, dst_pos, dst_len, __FILE__, __LINE__,
                           "strbuf_overwrite");

  if(src == NULL) return;
  if(dst_len == src_len) strbuf_copy(dst, dst_pos, src, src_len);
  size_t newlen = dst->len + src_len - dst_len;

  _ensure_capacity_update_ptr(dst, newlen, &src);

  if(src >= dst->buff && src < dst->buff + dst->capacity)
  {
    if(src_len < dst_len) {
      // copy
      memmove(dst->buff+dst_pos, src, src_len * sizeof(char));
      // resize (shrink)
      memmove(dst->buff+dst_pos+src_len, dst->buff+dst_pos+dst_len,
              (dst->len-dst_pos-dst_len) * sizeof(char));
    }
    else
    {
      // Buffer is going to grow and src points to this buffer

      // resize (grow)
      memmove(dst->buff+dst_pos+src_len, dst->buff+dst_pos+dst_len,
              (dst->len-dst_pos-dst_len) * sizeof(char));

      char *tgt = dst->buff + dst_pos;
      char *end = dst->buff + dst_pos + src_len;

      if(src < tgt + dst_len)
      {
        size_t len = MIN((size_t)(end - src), src_len);
        memmove(tgt, src, len);
        tgt += len;
        src += len;
        src_len -= len;
      }

      if(src >= tgt + dst_len)
      {
        // shift to account for resizing
        src += src_len - dst_len;
        memmove(tgt, src, src_len);
      }
    }
  }
  else
  {
    // resize
    memmove(dst->buff+dst_pos+src_len, dst->buff+dst_pos+dst_len,
            (dst->len-dst_pos-dst_len) * sizeof(char));
    // copy
    memcpy(dst->buff+dst_pos, src, src_len * sizeof(char));
  }

  dst->len = newlen;
  dst->buff[dst->len] = '\0';
}

void strbuf_delete(StrBuf *sbuf, size_t pos, size_t len)
{
  _bounds_check_read_range(sbuf, pos, len, __FILE__, __LINE__, "strbuf_delete");
  memmove(sbuf->buff+pos, sbuf->buff+pos+len, sbuf->len-pos-len);
  sbuf->len -= len;
  sbuf->buff[sbuf->len] = '\0';
}

/**************************/
/*         sprintf        */
/**************************/

int strbuf_vsprintf(StrBuf *sbuf, size_t pos, const char* fmt, va_list argptr)
{
  _bounds_check_insert(sbuf, pos, __FILE__, __LINE__, "strbuf_vsprintf");

  // Length of remaining buffer
  size_t buf_len = sbuf->capacity - pos;
  if(buf_len == 0 && !strbuf_resize(sbuf, sbuf->capacity << 1)) {
    fprintf(stderr, "%s:%i:Error: Out of memory\n", __FILE__, __LINE__);
    kill(getpid(), SIGABRT);
    // exit(EXIT_FAILURE);
  }

  // Make a copy of the list of args incase we need to resize buff and try again
  va_list argptr_cpy;
  va_copy(argptr_cpy, argptr);

  int num_chars = vsnprintf(sbuf->buff+pos, buf_len, fmt, argptr);

  // num_chars is the number of chars that would be written (not including '\0')
  // num_chars < 0 => failure
  if((size_t)(num_chars + 1) >= buf_len)
  {
    strbuf_ensure_capacity(sbuf, pos+num_chars);

    // now use the argptr copy we made earlier
    // Don't need to use vsnprintf now, vsprintf will do since we know it'll fit
    num_chars = vsprintf(sbuf->buff+pos, fmt, argptr_cpy);

    va_end(argptr_cpy);
  }

  // Don't need to NUL terminate, vsprintf/vnsprintf does that for us
  if(num_chars < 0)
  {
    // Errors occurred - report, and make sure string is terminated
    fprintf(stderr, "Warning: strbuf_sprintf something went wrong..\n");
    sbuf->buff[sbuf->len] = '\0';
  }
  else
  {
    // Update length
    sbuf->len = pos + num_chars;
  }

  return num_chars;
}

// Appends sprintf
int strbuf_sprintf(StrBuf *sbuf, const char* fmt, ...)
{
  va_list argptr;
  va_start(argptr, fmt);
  int num_chars = strbuf_vsprintf(sbuf, sbuf->len, fmt, argptr);
  va_end(argptr);

  return num_chars;
}

int strbuf_sprintf_at(StrBuf *sbuf, size_t pos, const char* fmt, ...)
{
  _bounds_check_insert(sbuf, pos, __FILE__, __LINE__, "strbuf_sprintf_at");

  va_list argptr;
  va_start(argptr, fmt);
  int num_chars = strbuf_vsprintf(sbuf, pos, fmt, argptr);
  va_end(argptr);

  return num_chars;
}

// Does not prematurely end the string if you sprintf within the string
// (vs at the end)
int strbuf_sprintf_noterm(StrBuf *sbuf, size_t pos, const char* fmt, ...)
{
  _bounds_check_insert(sbuf, pos, __FILE__, __LINE__, "strbuf_sprintf_noterm");

  va_list argptr;
  va_start(argptr, fmt);
  int num_chars = vsnprintf(NULL, 0, fmt, argptr);
  va_end(argptr);
  
  // Save overwritten char
  char last_char = (pos+num_chars < sbuf->len) ? sbuf->buff[pos+num_chars] : 0;
  size_t len = sbuf->len;

  va_start(argptr, fmt);
  num_chars = strbuf_vsprintf(sbuf, pos, fmt, argptr);
  va_end(argptr);

  // Restore length if shrunk, null terminate if extended
  if(sbuf->len < len) sbuf->len = len;
  else sbuf->buff[sbuf->len] = '\0';

  // Re-instate overwritten character
  sbuf->buff[pos+num_chars] = last_char;

  return num_chars;
}


/*****************/
/* File handling */
/*****************/

// Reading a FILE
size_t strbuf_readline(StrBuf *sbuf, FILE *file)
{
  return freadline(file, &(sbuf->buff), &(sbuf->len), &(sbuf->capacity));
}

size_t strbuf_gzreadline(StrBuf *sbuf, gzFile file)
{
  return gzreadline(file, &sbuf->buff, &sbuf->len, &sbuf->capacity);
}

// Reading a FILE
size_t strbuf_readline_buf(StrBuf *sbuf, FILE *file, buffer_t *in)
{
  return freadline_buf(file, in, &sbuf->buff, &sbuf->len, &sbuf->capacity);
}

size_t strbuf_gzreadline_buf(StrBuf *sbuf, gzFile file, buffer_t *in)
{
  return gzreadline_buf(file, in, &sbuf->buff, &sbuf->len, &sbuf->capacity);
}

size_t strbuf_skipline(FILE* file)
{
  return fskipline(file);
}

size_t strbuf_gzskipline(gzFile file)
{
  return gzskipline(file);
}

size_t strbuf_skipline_buf(FILE* file, buffer_t *in)
{
  return fskipline_buf(file, in);
}

size_t strbuf_gzskipline_buf(gzFile file, buffer_t *in)
{
  return gzskipline_buf(file, in);
}

#define _func_read_nonempty(name,type_t,__readline)                            \
  size_t name(StrBuf *line, type_t fh)                                         \
  {                                                                            \
    size_t i, origlen = line->len;                                             \
    while(__readline(line, fh) > 0) {                                          \
      i = origlen;                                                             \
      while(i < line->len && (line->buff[i] == '\r' || line->buff[i] == '\n')) \
        i++;                                                                   \
      if(i < line->len) return line->len - origlen;                            \
      line->len = origlen;                                                     \
      line->buff[line->len] = '\0';                                            \
    }                                                                          \
    return 0;                                                                  \
  }

_func_read_nonempty(strbuf_readline_nonempty,FILE*,strbuf_readline)
_func_read_nonempty(strbuf_gzreadline_nonempty,gzFile,strbuf_gzreadline)


#define _func_read(name,type_t,__read) \
  size_t name(StrBuf *sbuf, type_t file, size_t len)                           \
  {                                                                            \
    if(len == 0) return 0;                                                     \
    strbuf_ensure_capacity(sbuf, sbuf->len + len);                             \
    long read;                                                                 \
    if((read = __read(file,sbuf->buff+sbuf->len,len)) <= 0) return 0;          \
    sbuf->len += read;                                                         \
    return read;                                                               \
  }

_func_read(strbuf_gzread, gzFile, gzread2)
_func_read(strbuf_fread, FILE*, fread2)

// read FILE
// returns number of characters read
// or 0 if EOF
size_t strbuf_reset_readline(StrBuf *sbuf, FILE *file)
{
  strbuf_reset(sbuf);
  return strbuf_readline(sbuf, file);
}

// read gzFile
// returns number of characters read
// or 0 if EOF
size_t strbuf_reset_gzreadline(StrBuf *sbuf, gzFile file)
{
  strbuf_reset(sbuf);
  return strbuf_gzreadline(sbuf, file);
}

/**********/
/*  trim  */
/**********/

// Trim whitespace characters from the start and end of a string
void strbuf_trim(StrBuf *sbuf)
{
  if(sbuf->len == 0)
    return;

  // Trim end first
  while(sbuf->len > 0 && isspace(sbuf->buff[sbuf->len-1]))
    sbuf->len--;

  sbuf->buff[sbuf->len] = '\0';

  if(sbuf->len == 0)
    return;

  size_t start = 0;

  while(start < sbuf->len && isspace(sbuf->buff[start]))
    start++;

  if(start != 0)
  {
    sbuf->len -= start;
    memmove(sbuf->buff, sbuf->buff+start, sbuf->len * sizeof(char));
    sbuf->buff[sbuf->len] = '\0';
  }
}

// Trim the characters listed in `list` from the left of `sbuf`
// `list` is a null-terminated string of characters
void strbuf_ltrim(StrBuf *sbuf, const char* list)
{
  size_t start = 0;

  while(start < sbuf->len && (strchr(list, sbuf->buff[start]) != NULL))
    start++;

  if(start != 0)
  {
    sbuf->len -= start;
    memmove(sbuf->buff, sbuf->buff+start, sbuf->len * sizeof(char));
    sbuf->buff[sbuf->len] = '\0';
  }
}

// Trim the characters listed in `list` from the right of `sbuf`
// `list` is a null-terminated string of characters
void strbuf_rtrim(StrBuf *sbuf, const char* list)
{
  if(sbuf->len == 0)
    return;

  while(sbuf->len > 0 && strchr(list, sbuf->buff[sbuf->len-1]) != NULL)
    sbuf->len--;

  sbuf->buff[sbuf->len] = '\0';
}

/**************************/
/* Other String Functions */
/**************************/

// Reverse a string region
void string_reverse_region(char *str, size_t length)
{
  char *a = str, *b = str + length - 1;
  char tmp;
  while(a < b) {
    tmp = *a; *a = *b; *b = tmp;
    a++; b--;
  }
}

char string_is_all_whitespace(const char* s)
{
  int i;
  for(i = 0; s[i] != '\0' && isspace(s[i]); i++);
  return (s[i] == '\0');
}

char* string_next_nonwhitespace(char* s)
{
  while(*s != '\0' && isspace(*s)) s++;
  return (*s == '\0' ? NULL : s);
}

// Strip whitespace the the start and end of a string.  
// Strips whitepace from the end of the string with \0, and returns pointer to
// first non-whitespace character
char* string_trim(char* str)
{
  // Work backwards
  char *end = str+strlen(str);
  while(end > str && isspace(*(end-1))) end--;
  *end = '\0';

  // Work forwards: don't need start < len because will hit \0
  while(isspace(*str)) str++;

  return str;
}

// Removes \r and \n from the ends of a string and returns the new length
size_t string_chomp(char* str, size_t len)
{
  while(len > 0 && (str[len-1] == '\r' || str[len-1] == '\n')) len--;
  str[len] = '\0';
  return len;
}

// Returns count
size_t string_count_char(const char* str, int c)
{
  size_t count = 0;
  const char *tmp = str;

  while((tmp = strchr(tmp, c)) != NULL)
  {
    tmp++;
    count++;
  }

  return count;
}

// Returns the number of strings resulting from the split
long string_split(const char* split, const char* txt, char*** result)
{
  size_t split_len = strlen(split);
  size_t txt_len = strlen(txt);

  // result is temporarily held here
  char** arr;

  if(split_len == 0)
  {
    // Special case
    if(txt_len == 0)
    {
      *result = NULL;
      return 0;
    }
    else
    {
      arr = malloc(txt_len * sizeof(char*));
    
      size_t i;

      for(i = 0; i < txt_len; i++)
      {
        arr[i] = malloc(2 * sizeof(char));
        arr[i][0] = txt[i];
        arr[i][1] = '\0';
      }

      *result = arr;
      return txt_len;
    }
  }
  
  const char* find = txt;
  long count = 1; // must have at least one item

  while((find = strstr(find, split)) != NULL)
  {
    //printf("Found1: '%s'\n", find);
    count++;
    find += split_len;
  }

  // Create return array
  arr = malloc(count * sizeof(char*));
  
  count = 0;
  const char* last_position = txt;

  while((find = strstr(last_position, split)) != NULL)
  {
    long str_len = find - last_position;

    arr[count] = malloc((str_len+1) * sizeof(char));
    strncpy(arr[count], last_position, str_len);
    arr[count][str_len] = '\0';
    
    count++;
    last_position = find + split_len;
  }

  // Copy last item
  long str_len = txt + txt_len - last_position;
  arr[count] = malloc((str_len+1) * sizeof(char));

  if(count == 0) strcpy(arr[count], txt);
  else          strncpy(arr[count], last_position, str_len);

  arr[count][str_len] = '\0';
  count++;
  
  *result = arr;
  
  return count;
}
