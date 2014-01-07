/*
 stream_buffer.h
 project: string_buffer
 url: https://github.com/noporpoise/StringBuffer
 author: Isaac Turner <turner.isaac@gmail.com>
 license: Public Domain
 Jan 2014
*/

#ifndef STRING_BUFFER_FILE_SEEN
#define STRING_BUFFER_FILE_SEEN

#include <stdio.h> // needed for FILE
#include <zlib.h> // needed for gzFile
#include <stdarg.h> // needed for va_list

#include "stream_buffer.h"

typedef struct
{
  char *buff;
  size_t len; // length of the string
  size_t capacity; // buffer size - includes '\0' (size >= len+1)
} StrBuf;

// MACROS
#define strbuf_new()    strbuf_init(256)
#define strbuf_len(sb)  ((sb)->len)
#define strbuf_dup(sb)  strbuf_as_str(sb)
// Copy a StrBuf to the end of this StrBuf
#define strbuf_append(s1,s2)      strbuf_append_strn(s1, (s2)->buff, (s2)->len)
// Copy a character array to the end of this StrBuf
#define strbuf_append_str(s1,s2)  strbuf_append_strn(s1, s2, strlen(s2))
#define strbuf_append_buff(s1,s2) strbuf_append(s1,s2)

//
// Creation, reset, free and memory expansion
//

// Constructors
StrBuf* strbuf_init(size_t size);
StrBuf* strbuf_create(const char* str);

// Destructor
void strbuf_free(StrBuf* sbuf);

// Place a string buffer into existing memory. Example:
//   StrBuf buf;
//   strbuf_alloc(&buf, 100);
//   ...
//   strbuf_dealloc(&buf);
StrBuf* strbuf_alloc(StrBuf *sbuf, size_t capacity);
void strbuf_dealloc(StrBuf *sbuf);

// Clone a buffer (including content)
StrBuf* strbuf_clone(const StrBuf* sbuf);

// Clear the content of an existing StrBuf (sets size to 0)
void strbuf_reset(StrBuf* sbuf);

//
// Resizing
//

// Ensure capacity for len characters plus '\0' character - exits on FAILURE
void strbuf_ensure_capacity(StrBuf *sbuf, size_t size);

// Shorten a string
void strbuf_shrink(StrBuf *sbuf, size_t new_len);

// Change how much memory is used to hold the buffer.  Can be used to grow or
// shrink buffer size.  Always calls realloc. Returns 1 on success, 0 on failure.
char strbuf_resize(StrBuf *sbuf, size_t new_size);

//
// Useful String functions
//

// Get a char (index < strlen(sbuf))
char strbuf_get_char(const StrBuf *sbuf, size_t index);
// Can set a char <= strlen(sbuf->buff)
void strbuf_set_char(StrBuf *sbuf, size_t index, char c);

// Set string buffer to contain a given string
void strbuf_set(StrBuf *sbuf, const char *str);

// Get a copy of this StrBuf as a char array
// Returns NULL if not enough memory
char* strbuf_as_str(const StrBuf* sbuf);

// Add a character to the end of this StrBuf
void strbuf_append_char(StrBuf* sbuf, char c);
// Copy N characters from a character array to the end of this StrBuf
void strbuf_append_strn(StrBuf* sbuf, const char* txt, size_t len);

// Remove \r and \n characters from the end of this StrBuf
// Returns the number of characters removed
size_t strbuf_chomp(StrBuf *sbuf);

// Reverse a string
void strbuf_reverse(StrBuf *sbuf);

// Get a substring as a new null terminated char array
// (remember to free the returned char* after you're done with it!)
char* strbuf_substr(const StrBuf *sbuf, size_t start, size_t len);

// Change to upper or lower case
void strbuf_to_uppercase(StrBuf *sbuf);
void strbuf_to_lowercase(StrBuf *sbuf);

// Copy a string to this StrBuf, overwriting any existing characters
// Note: dst_pos + len can be longer the the current dst StrBuf
void strbuf_copy(StrBuf *dst, size_t dst_pos,
                 const char *src, size_t len);

// Insert: copy to a StrBuf, shifting any existing characters along
void strbuf_insert(StrBuf *dst, size_t dst_pos,
                   const char *src, size_t len);

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
                      const char *src, size_t src_len);

// Remove characters from the buffer
//   strbuf_set(sbuf, "aaaBBccc");
//   strbuf_delete(sbuf, 3, 2);
//   // sbuf is now "aaaccc"
void strbuf_delete(StrBuf *sbuf, size_t pos, size_t len);

//
// sprintf
//

// sprintf to a StrBuf (adds string terminator after sprint)
int strbuf_sprintf(StrBuf *sbuf, const char *fmt, ...)
  __attribute__ ((format(printf, 2, 3)));

int strbuf_sprintf_at(StrBuf *sbuf, size_t pos, const char *fmt, ...)
  __attribute__ ((format(printf, 3, 4)));

int strbuf_vsprintf(StrBuf *sbuf, size_t pos, const char *fmt, va_list argptr)
  __attribute__ ((format(printf, 3, 0)));

// sprintf without terminating character
// Does not prematurely end the string if you sprintf within the string
// (terminates string if sprintf to the end)
int strbuf_sprintf_noterm(StrBuf *sbuf, size_t pos, const char *fmt, ...)
  __attribute__ ((format(printf, 3, 4)));

//
// Reading files
//

// Reading a FILE
size_t strbuf_reset_readline(StrBuf *sbuf, FILE *file);
size_t strbuf_readline(StrBuf *sbuf, FILE *file);
size_t strbuf_skipline(FILE *file);
size_t strbuf_readline_buf(StrBuf *sbuf, FILE *file, buffer_t *in);
size_t strbuf_skipline_buf(FILE* file, buffer_t *in);
size_t strbuf_read(StrBuf *sbuf, FILE *file, size_t len);

// Reading a gzFile
size_t strbuf_reset_gzreadline(StrBuf *sbuf, gzFile gz_file);
size_t strbuf_gzreadline(StrBuf *sbuf, gzFile gz_file);
size_t strbuf_gzskipline(gzFile gz_file);
size_t strbuf_gzreadline_buf(StrBuf *sbuf, gzFile gz_file, buffer_t *in);
size_t strbuf_gzskipline_buf(gzFile file, buffer_t *in);
size_t strbuf_gzread(StrBuf *sbuf, gzFile gz_file, size_t len);

// Read a line that has at least one character that is not \r or \n
// these functions do not call reset before reading
// Returns the number of characters read
size_t strbuf_readline_nonempty(StrBuf *line, FILE *fh);
size_t strbuf_gzreadline_nonempty(StrBuf *line, gzFile gz);

//
// String functions
//

// Trim whitespace characters from the start and end of a string
void strbuf_trim(StrBuf *sbuf);

// Trim the characters listed in `list` from the left of `sbuf`
// `list` is a null-terminated string of characters
void strbuf_ltrim(StrBuf *sbuf, const char* list);

// Trim the characters listed in `list` from the right of `sbuf`
// `list` is a null-terminated string of characters
void strbuf_rtrim(StrBuf *sbuf, const char* list);

/**************************/
/* Other String functions */
/**************************/

void string_reverse_region(char *str, size_t length);
char string_is_all_whitespace(const char* s);
char* string_next_nonwhitespace(char* s);
char* string_trim(char* str);
// Chomp a string, returns new length
size_t string_chomp(char* str, size_t len);
size_t string_count_char(const char* str, int c);
size_t string_split(const char* split, const char* txt, char*** result);

#endif
