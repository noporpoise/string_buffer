/*
 string_buffer.h
 project: string_buffer
 url: https://github.com/noporpoise/StringBuffer
 author: Isaac Turner <turner.isaac@gmail.com>

 Copyright (c) 2011, Isaac Turner
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

#ifndef STRING_BUFFER_FILE_SEEN
#define STRING_BUFFER_FILE_SEEN

typedef unsigned long t_buf_pos;

//#include <stdio.h> // needed for FILE*

#include <zlib.h> // needed for gzFile

typedef struct STRING_BUFFER STRING_BUFFER;

struct STRING_BUFFER {
  char *buff;
  t_buf_pos len; // length of the string
  t_buf_pos size; // buffer size
};

// Creation, reset, free and memory expansion
STRING_BUFFER* string_buff_init(const t_buf_pos size);
STRING_BUFFER* string_buff_create(const char* str);
void string_buff_reset(STRING_BUFFER* sbuf);
void string_buff_free(STRING_BUFFER* sbuf);
void string_buff_grow(STRING_BUFFER *sbuf, const t_buf_pos new_size);
void string_buff_shrink(STRING_BUFFER *sbuf, const t_buf_pos new_len);
STRING_BUFFER* string_buff_copy(STRING_BUFFER* sbuf);

// Useful String functions
void string_buff_add(STRING_BUFFER* sbuf, const char* txt);
void string_buff_addn(STRING_BUFFER* sbuf, const char* txt, const t_buf_pos len);
void string_buff_add_char(STRING_BUFFER* sbuf, const char txt);
void string_buff_chomp(STRING_BUFFER *sbuf);
char* string_buff_substr(STRING_BUFFER *sbuf, const t_buf_pos start, const t_buf_pos len);
void string_buff_to_uppercase(STRING_BUFFER *sbuf);
void string_buff_to_lowercase(STRING_BUFFER *sbuf);

// Reading a file
t_buf_pos string_buff_reset_readline(STRING_BUFFER *sbuf, gzFile *gz_file);
t_buf_pos string_buff_readline(STRING_BUFFER *sbuf, gzFile *gz_file);

// Other String functions
long split_str(const char* split, const char* txt, char*** result);

#endif
