
#ifndef _BUFFERED_INPUT_HEADER
#define _BUFFERED_INPUT_HEADER

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>

typedef struct
{
  char *b;
  // begin is index of first char (unless begin >= end)
  // end is index of \0
  // size should be >= end+1 to allow for \0
  // (end-size) is the number of bytes in buffer
  size_t begin, end, size;
} buffer_t;

// Buffer functions
#define ROUNDUP2POW(x) (0x1 << (64 - __builtin_clzl(x)))

static inline char buffer_init(buffer_t *b, size_t s)
{
  if((b->b = malloc(sizeof(char)*s)) == NULL) return 0;
  b->size = s;
  b->begin = b->end = 0;
  return 1;
}

static inline buffer_t* buffer_alloc(size_t s)
{
  buffer_t *b = (buffer_t*)malloc(sizeof(buffer_t));
  if(b == NULL) return NULL;
  else if(buffer_init(b,s)) return b;
  free(b); return NULL; /* couldn't malloc */
}

#define buffer_destroy(buf) do{free((buf)->b); free(buf); } while(0)

// size_t s is the number of bytes you want to be able to store
// the actual buffer is created with s+1 bytes to allow for the \0
static inline void buffer_ensure_capacity(buffer_t *buf, size_t s)
{
  if(buf->size < ++s) {
    buf->size = ROUNDUP2POW(s);
    buf->b = realloc(buf->b, buf->size);
  }
}

static inline void buffer_append_str(buffer_t *buf, char *str)
{
  size_t len = buf->end + strlen(str);
  buffer_ensure_capacity(buf, len);
  memcpy(buf->b+buf->end, str, len);
  buf->b[buf->end = len] = 0;
}

static inline void buffer_append_char(buffer_t *buf, char c)
{
  buffer_ensure_capacity(buf, buf->end+1);
  buf->b[buf->end++] = c;
  buf->b[buf->end] = '\0';
}

#define buffer_terminate(buf) ((buf)->b[(buf)->end] = 0)

#define buffer_chomp(buf) do { \
    if((buf)->end > 0 && (buf)->b[(buf)->end-1] == '\n') {                     \
      (buf)->end--;                                                            \
      if((buf)->end > 0 && (buf)->b[(buf)->end-1] == '\r') (buf)->end--;       \
      (buf)->b[(buf)->end] = 0;                                                \
    }                                                                          \
  } while(0)

/* 
Unbuffered

fgetc(f)
gzgetc(gz)
gzread2(gz,buf,len)
fread2(f,buf,len)
gzgets2(gz,buf,len)
fgets2(f,buf,len)
gzreadline(gz,out)
freadline(f,out)
*/

// Define read for gzFile and FILE (unbuffered)
#define gzread2(gz,buf,len) gzread(gz,buf,(unsigned int)len)
#define fread2(f,buf,len) fread(buf,sizeof(char),len,file)

#define gzgets2(gz,buf,len) gzgets(gz,buf,(int)(len))
#define fgets2(f,buf,len) fgets(buf,(int)(len),f)

// fgetc(f), gzgetc(gz) are already good to go

// Define readline for gzFile and FILE (unbuffered)
#define _func_readline(name,type_t,__gets,s_name,chars,len_field,size_field)   \
  static inline size_t name(type_t file, s_name *buf)                          \
  {                                                                            \
    _ENSURE_CAPACITY(buf, buf->len_field+1, chars, size_field);                \
    size_t n, total_read = 0;                                                  \
    while(__gets(file, buf->chars+buf->len_field, buf->size_field-buf->len_field)) \
    {                                                                          \
      n = strlen(buf->chars+buf->len_field);                                   \
      buf->len_field += n; total_read += n;                                    \
      if(buf->chars[buf->len_field-1] == '\n') return total_read;              \
      else buf->chars = realloc(buf->chars, buf->size_field <<= 1);            \
    }                                                                          \
    return total_read;                                                         \
  }

// _func_readline(gzreadline,gzFile,gzgets2)
// _func_readline(freadline,FILE*,fgets2)

// Define skipline
#define _func_skipline(fname,ftype,readc) \
  static inline size_t fname(ftype file)                                       \
  {                                                                            \
    int c;                                                                     \
    size_t skipped_bytes = 0;                                                  \
    while((c = readc(file)) != -1) {                                           \
      skipped_bytes++;                                                         \
      if(c == '\n') break;                                                     \
    }                                                                          \
    return skipped_bytes;                                                      \
  }

_func_skipline(gzskipline,gzFile,gzgetc)
_func_skipline(fskipline,FILE*,fgetc)

/* Buffered */

/*
fgetc_buf(f,in)
gzgetc_buf(gz,in)
gzreadline_buf(gz,in,out)
freadline_buf(f,in,out)
*/

#define _ENSURE_CAPACITY(buf,s,chars,size) do {                                \
    size_t ss = s+1;                                                           \
    if(buf->size < ss) {                                                       \
      buf->size = ROUNDUP2POW(ss);                                             \
      buf->chars = realloc(buf->chars, buf->size);                             \
    }                                                                          \
  } while(0)

// __read is either gzread2 or fread2
#define _READ_BUFFER(file,in,__read) do {                                      \
    long input = __read(file,in->b,in->size);                                  \
    (in)->end = input < 0 ? 0 : input;                                         \
    (in)->begin = 0;                                                           \
  } while(0)

// Define getc for gzFile and FILE (buffered)
#define _func_getc_buf(fname,type_t,__read)                                    \
  static inline int fname(type_t file, buffer_t *in)                           \
  {                                                                            \
    if(in->begin >= in->end) {                                                 \
      _READ_BUFFER(file,in,__read);                                            \
      return in->end == 0 ? -1 : in->b[in->begin++];                           \
    }                                                                          \
    return in->b[in->begin++];                                                 \
  }

_func_getc_buf(gzgetc_buf,gzFile,gzread2)
_func_getc_buf(fgetc_buf,FILE*,fread2)

// Define readline for gzFile and FILE (buffered)
#define _func_readline_buf(fname,type_t,__read,s_name,chars,len_field,size_field) \
  static inline size_t fname(type_t file, buffer_t *in, s_name *buf)           \
  {                                                                            \
    if(in->begin >= in->end) { _READ_BUFFER(file,in,__read); }                 \
    size_t offset, buffered, total_read = 0;                                   \
    while(in->end != 0)                                                        \
    {                                                                          \
      for(offset = in->begin; offset < in->end && in->b[offset++] != '\n'; );  \
      buffered = offset - in->begin;                                           \
      _ENSURE_CAPACITY(buf, buf->len_field+buffered, chars, size_field);       \
      memcpy(buf->chars+buf->len_field, in->b+in->begin, buffered);            \
      buf->len_field += buffered;                                              \
      in->begin = offset;                                                      \
      total_read += buffered;                                                  \
      if(buf->chars[buf->len_field-1] == '\n') break;                          \
      _READ_BUFFER(file,in,__read);                                            \
    }                                                                          \
    buf->chars[buf->len_field] = 0;                                            \
    return total_read;                                                         \
  }

// _func_readline_buf(gzreadline_buf,gzFile,gzread2)
// _func_readline_buf(freadline_buf,FILE*,fread2)

// Define buffered skipline
#define _func_skipline_buf(fname,ftype,__read) \
  static inline size_t fname(ftype file, buffer_t *in)                         \
  {                                                                            \
    if(in->begin >= in->end) { _READ_BUFFER(file,in,__read); }                 \
    size_t offset, skipped_bytes = 0;                                          \
    while(in->end != 0)                                                        \
    {                                                                          \
      for(offset = in->begin; offset < in->end && in->b[offset++] != '\n'; )   \
      skipped_bytes += offset - in->begin;                                     \
      in->begin = offset;                                                      \
      if(in->b[offset-1] == '\n') break;                                       \
      _READ_BUFFER(file,in,__read);                                            \
    }                                                                          \
    return skipped_bytes;                                                      \
  }

_func_skipline_buf(gzskipline_buf,gzFile,gzread2)
_func_skipline_buf(fskipline_buf,FILE*,fread2)

// Define buffered gzgets_buf, fgets_buf

// Does not resize buf;  Reads upto len bytes or to the first \n if before then
// Read len-1 bytes
#define _func_gets_buf(fname,type_t,__read) \
  static inline char* fname(type_t file, buffer_t *in, char* str,              \
                            unsigned int len)                                  \
  {                                                                            \
    if(len == 0) return NULL;                                                  \
    if(len == 1) {str[0] = 0; return str; }                                    \
    if(in->begin >= in->end) { _READ_BUFFER(file,in,__read); }                 \
    size_t i, buffered, limit, total_read = 0, remaining = len-1;              \
    while(in->end != 0)                                                        \
    {                                                                          \
      limit = (in->begin+remaining < in->end ? in->begin+remaining : in->end); \
      for(i = in->begin; i < limit && in->b[i++] != '\n'; );                   \
      buffered = i - in->begin;                                                \
      memcpy(str+total_read, in->b+in->begin, buffered);                       \
      in->begin += buffered;                                                   \
      total_read += buffered;                                                  \
      remaining -= buffered;                                                   \
      if(remaining == 0 || str[total_read-1] == '\n') break;                   \
      _READ_BUFFER(file,in,__read);                                            \
    }                                                                          \
    str[total_read] = 0;                                                       \
    return total_read == 0 ? NULL : str;                                       \
  }

_func_gets_buf(gzgets_buf,gzFile,gzread2)
_func_gets_buf(fgets_buf,FILE*,fread2)


#define BUFFERED_INPUT_SETUP(s_name,cfield,lfield,sfield)                      \
  _func_readline(gzreadline,gzFile,gzgets2,s_name,cfield,lfield,sfield)        \
  _func_readline(freadline,FILE*,fgets2,s_name,cfield,lfield,sfield)           \
  _func_readline_buf(gzreadline_buf,gzFile,gzread2,s_name,cfield,lfield,sfield)\
  _func_readline_buf(freadline_buf,FILE*,fread2,s_name,cfield,lfield,sfield)

// e.g.
// BUFFERED_INPUT_SETUP(buffer_t, b, end, size);

#endif
