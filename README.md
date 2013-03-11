C String Buffer
===============
Library code for handling strings and reading from files  
project: string_buffer  
url: https://github.com/noporpoise/StringBuffer  
author: Isaac Turner <turner.isaac@gmail.com>  

About
=====

A string buffer library for C. Only has zlib as a dependency. Compiles with gcc
and clang.

Features:
- copying, inserting, appending, substring, chomp, trim
- reverse region, convert to upper/lower case
- sprintf into string buffer
- read a line at a time from a file
- buffered reading (10X faster for older versions of zlib)
- gzip file support

To build the test code:

    $ make
    $ ./strbuf_test

Calling
=======

To use in your code, include the following arguments in your gcc command:

    gcc ... -I$(STRING_BUF_PATH) -L$(STRING_BUF_PATH) ... -lstrbuf -lz

and include in your source code:

    include "string_buffer.h"

Example Code
============

    StrBuf* myBuff = strbuf_new()

    // Read from a file:

    gzFile fgz = gzopen("path/here.txt.gz")

    while(strbuf_gzreadline(myBuff, fgz))
    {
      // Do something with the line

      // e.g. chomp (remove newline)
      strbuf_chomp(myBuff)

      printf("%s\n", myBuff->buff)

      // Reset StrBuf so you're not just concatenating all the lines in memory
      strbuf_reset(myBuff)
    }

    // Close up
    gzclose(fgz)

    strbuf_free(myBuff)


String buffers can still be used as input to standard str functions by accessing
the char* in the StrBuf struct. e.g.:

Get the position of the first 'a' in a StrBuf

    char* ptr = strchr(strbuf->buff, 'a')
    int pos = (ptr == NULL ? -1 : ptr - strbuf->buff)

Test if the StrBuf contains 'hello' from index 12

    if(strncasecmp(strbuf->buff+12, "hello", 5) == 0)
      puts("world!\n")


API
===

Struct
------

    typedef struct
    {
      char *buff;
      size_t len; // length of the string
      size_t capacity; // buffer size - includes '\0' (size is always >= len+1)
    } StrBuf;

Creators, destructors etc.
--------------------------

Constructors.  Note capacity increases as needed.

    StrBuf* strbuf_new()
    StrBuf* strbuf_init(const size_t capacity)
    StrBuf* strbuf_create(const char* str)

Place a string buffer into existing memory

    StrBuf* strbuf_alloc(StrBuf *sbuf, size_t capacity)

    // Example:
    StrBuf buf;
    strbuf_alloc(&buf, 100);

Destructors

    void strbuf_free(StrBuf* sbuf)

Clone a string buffer (including content)

    StrBuf* strbuf_clone(const StrBuf* sbuf)

Clear the content of an existing StrBuf (sets size to 0)

    void strbuf_reset(StrBuf* sbuf)

Resizing
--------

Ensure capacity for len characters plus '\0' character - exits on FAILURE

    void strbuf_ensure_capacity(StrBuf *sbuf, const size_t len)

Resize the buffer to have capacity to hold a string of length new_len
(+ a null terminating character).  Can also be used to downsize the buffer's
memory usage.  Returns 1 on success, 0 on failure.

    char strbuf_resize(StrBuf *sbuf, const size_t new_size)

Useful functions
----------------

get/set chars

    char strbuf_get_char(const StrBuf *sbuf, const size_t index)
    void strbuf_set_char(StrBuf *sbuf, const size_t index, const char c)

Set string buffer to contain a given string
The string can be a string within the given string buffer

    void strbuf_set(StrBuf *sbuf, const char *str)

Get a copy of this StrBuf as a char array.
Returns NULL if not enough memory.
`strbuf_dup` is also provided as a shorthand.

    char* strbuf_as_str(const StrBuf* sbuf)

Add a character to the end of this StrBuf

    void strbuf_append_char(StrBuf* sbuf, const char txt)

Copy a StrBuf to the end of this StrBuf.
`strbuf_append` is also provided as a shorthand.

    void strbuf_append_buff(StrBuf* dst, StrBuf* src)

Copy a character array to the end of this StrBuf

    void strbuf_append_str(StrBuf* sbuf, const char* txt)

Copy N characters from a character array to the end of this StrBuf

    void strbuf_append_strn(StrBuf* sbuf, const char* txt, const size_t len)

Remove \r and \n characters from the end of this StrBuf.
Returns the number of characters removed

    size_t strbuf_chomp(StrBuf *sbuf)

Reverse a string

    void strbuf_reverse(StrBuf *sbuf)

Get a substring as a new null terminated char array
(remember to free the returned char* after you're done with it!)

    char* strbuf_substr(StrBuf *sbuf, const size_t start, const size_t len)

Change to upper or lower case

    void strbuf_to_uppercase(StrBuf *sbuf)
    void strbuf_to_lowercase(StrBuf *sbuf)

Copy a string to this StrBuf, overwriting any existing characters
Note: dst_pos + len can be longer the the current dst StrBuf

    void strbuf_copy(StrBuf* dst, size_t dst_pos,
                     const char* src, size_t len)

Insert: copy to a StrBuf, shifting any existing characters along

    void strbuf_insert(StrBuf* dst, size_t dst_pos,
                       const char* src, size_t len)


Overwrite `dst_pos..(dst_pos+dst_len-1)` with `src_len` chars from `src`.
If `dst_len != src_len`, content to the right of `dst_len` is shifted

    void strbuf_overwrite(StrBuf *dst, size_t dst_pos, size_t dst_len,
                          const char *src, size_t src_len)

    // Example:
    strbuf_set(sbuf, "aaabbccc");
    char *data = "xxx";
    strbuf_overwrite(sbuf, 3, 2, data, strlen(data));
    // sbuf is now "aaaxxxccc"
    strbuf_overwrite(sbuf, 3, 3, "_", 1);
    // sbuf is now "aaa_ccc"

Remove characters from the buffer

    void strbuf_delete(StrBuf *sbuf, size_t pos, size_t len)

    // Example:
    strbuf_set(sbuf, "aaaBBccc");
    strbuf_delete(sbuf, 3, 2);
    // sbuf is now "aaaccc"


Formatted strings (sprintf)
---------------------------

sprintf to a StrBuf (adds string terminator after sprint)

    int strbuf_sprintf(StrBuf *sbuf, const char* fmt, ...)
    int strbuf_sprintf_at(StrBuf *sbuf, const size_t pos, const char* fmt, ...)
    int strbuf_vsprintf(StrBuf *sbuf, const size_t pos,
                        const char* fmt, va_list argptr)

sprintf without terminating character.
Does not prematurely end the string if you sprintf within the string
(terminates string if sprintf to the end)

    int strbuf_sprintf_noterm(StrBuf *sbuf, const size_t pos,
                              const char* fmt, ...)

Reading files
-------------

Reading a FILE

    size_t strbuf_reset_readline(StrBuf *sbuf, FILE *file)
    size_t strbuf_readline(StrBuf *sbuf, FILE *gz_file)

Reading a gzFile

    size_t strbuf_reset_gzreadline(StrBuf *sbuf, gzFile gz_file)
    size_t strbuf_gzreadline(StrBuf *sbuf, gzFile gz_file)

Skip a line and return how many characters were skipped

    size_t strbuf_skipline(FILE *file)
    size_t strbuf_gzskipline(gzFile gz_file)

Read a line but no more than len bytes

    size_t strbuf_read(StrBuf *sbuf, FILE *file, size_t len)
    size_t strbuf_gzread(StrBuf *sbuf, gzFile file, size_t len)

Buffered reading

    size_t strbuf_gzreadline_buf(StrBuf *sbuf, gzFile gz_file, buffer_t *in);
    size_t strbuf_gzskipline_buf(gzFile file, buffer_t *in);

    size_t strbuf_readline_buf(StrBuf *sbuf, FILE *file, buffer_t *in);
    size_t strbuf_skipline_buf(FILE* file, buffer_t *in);

Example of buffered reading:

    gzFile gzf = gzopen("input.txt.gz", "r");
    buffer_t *in = buffer_new(1024); // pass buffer size in bytes
    StrBuf *line = strbuf_new();

    while(strbuf_gzreadline_buf(line, gzf, in) > 0)
    {
      strbuf_chomp(line);
      printf("read: %s\n", line->buff);
    }

    strbuf_free(line);
    buffer_free(in);
    gzclose(gzf);

Read a line that has at least one character that is not \r or \n.
These functions do not call reset before reading.
Returns the number of characters read.

    size_t strbuf_readline_nonempty(StrBuf *line, FILE *fh)
    size_t strbuf_gzreadline_nonempty(StrBuf *line, gzFile gz)

Trim characters
---------------

Trim whitespace characters from the start and end of a string

    void strbuf_trim(StrBuf *sbuf)

Trim the characters listed in `list` from the left of `sbuf`.
`list` is a null-terminated string of characters

    void strbuf_ltrim(StrBuf *sbuf, const char* list)

Trim the characters listed in `list` from the right of `sbuf`.
`list` is a null-terminated string of characters

    void strbuf_rtrim(StrBuf *sbuf, const char* list)

Use with sscanf
---------------

To read strings into a string buffer using `sscanf`, first you must ensure the
buffer is big enough, and afterwards you must ensure the length is stored
correctly.

    StrBuf *sbuf = strbuf_new();
    char *input = "I'm sorry Dave I can't do that";
    
    strbuf_ensure_capacity(sbuf, strlen(input));
    sscanf(input, "I'm sorry %s I can't do that", sbuf->buff);
    sbuf->len = strlen(sbuf->buff);

    printf("Name: '%s'\n", sbuf->buff);

    strbuf_free(sbuf);

Buffered input
--------------

buffered_input.h also provides generic buffered input functions

    static inline buffer_t* buffer_new(size_t s)
    static inline char buffer_init(buffer_t *b, size_t s)
    void buffer_free(buffer_t *b)
    static inline void buffer_ensure_capacity(buffer_t *buf, size_t s)
    static inline void buffer_append_str(buffer_t *buf, char *str)
    static inline void buffer_append_char(buffer_t *buf, char c)
    void buffer_terminate(buffer_t *b)
    void buffer_chomp(buffer_t *b)

    // Standardized gzFile and FILE versions of stream functions

    // Return number of bytes read (0 -> EOF; gzread2 returns -1 on error)
    int gzread2(gz,buf,len)
    size_t fread2(f,buf,len)

    // Return pointer to buffer read into or NULL if EOF
    char* gzgets2(gz,buf,len)
    char* fgets2(f,buf,len)

    // Writes the given null-terminated string to a stream, excluding the
    // terminating null character.
    // Returns a non-negative number, or –1 in case of error. 
    int gzputs2(gz,buf)
    int fputs2(f,buf)

    // FILE readline
    static inline size_t freadline(FILE* file, char **buf, size_t *len, size_t *size)
    static inline size_t fskipline(FILE* file)

    // FILE Buffered reading
    static inline int fgetc_buf(FILE* file, buffer_t *in)
    static inline char* fgets_buf(FILE* file, buffer_t *in, char* str, unsigned int len)
    static inline size_t freadline_buf(FILE* file, buffer_t *in, char **buf, size_t *len, size_t *size)
    static inline size_t fskipline_buf(FILE* file, buffer_t *in)

    // gzFile readline
    static inline size_t gzreadline(gzFile file, char **buf, size_t *len, size_t *size)
    static inline size_t fskipline(FILE* file)

    // gzFile Buffered reading
    static inline int gzgetc_buf(gzFile file, buffer_t *in)
    static inline char* gzgets_buf(gzFile file, buffer_t *in, char* str, unsigned int len)
    static inline size_t gzreadline_buf(gzFile file, buffer_t *in, char **buf, size_t *len, size_t *size)
    static inline size_t gzskipline_buf(gzFile file, buffer_t *in)

Other string functions
----------------------

These work on `char*` not `StrBuf`, but they're here because they're useful. 

    void string_reverse_region(char *str, size_t length)
    char string_is_all_whitespace(const char* s)
    char* string_next_nonwhitespace(char* s)
    char* string_trim(char* str)
    size_t string_chomp(char* str)
    size_t string_count_char(const char* str, const int c)
    long string_split(const char* split, const char* txt, char*** result)


License
=======

    Copyright (c) 2011-3, Isaac Turner
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

Development
===========

short term goals: none - please suggest some!

I like to hear about how you're using it, what bugs you've found and what
features you'd like to see!  Contact me: Isaac Turner <turner.isaac@gmail>
