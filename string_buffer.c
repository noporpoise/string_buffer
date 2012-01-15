/*
 string_buffer.c
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

#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN_SIZE 10

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // toupper() and tolower()

#include "string_buffer.h"

STRING_BUFFER* string_buff_init(const t_buf_pos size)
{
  t_buf_pos new_size = size < MIN_SIZE ? MIN_SIZE : size;

  STRING_BUFFER* sbuf = (STRING_BUFFER*) malloc(sizeof(STRING_BUFFER));

  sbuf->buff = (char*) malloc(new_size);
  sbuf->len = 0;
  sbuf->size = new_size;

  if(sbuf->buff == NULL)
  {
    fprintf(stderr, "Error: STRING_BUFFER couldn't be created with %lui bytes.",
            new_size);
    exit(-1);
  }

  return sbuf;
}

STRING_BUFFER* string_buff_create(const char* str)
{
  t_buf_pos str_len = strlen(str);

  STRING_BUFFER* sbuf = string_buff_init(str_len+1);

  strcpy(sbuf->buff, str);
  sbuf->buff[str_len] = '\0';

  sbuf->len = str_len;

  return sbuf;
}

void string_buff_reset(STRING_BUFFER* sbuf)
{
  sbuf->len = 0;
  
  if(sbuf->size > 0)
  {
    sbuf->buff[0] = '\0';
  }
}

void string_buff_free(STRING_BUFFER* sbuf)
{
  free(sbuf->buff);
  free(sbuf);
}

STRING_BUFFER* string_buff_clone(STRING_BUFFER* sbuf)
{
  // One byte for the string end / null char \0
  STRING_BUFFER* sbuf_cpy = string_buff_init(sbuf->len+1);
  
  strcpy(sbuf_cpy->buff, sbuf->buff);
  sbuf_cpy->buff[sbuf->len] = '\0';

  sbuf_cpy->len = sbuf->len;
  
  return sbuf_cpy;
}

inline t_buf_pos string_buff_strlen(STRING_BUFFER* sbuf)
{
  return sbuf->len;
}

inline t_buf_pos string_buff_size(STRING_BUFFER* sbuf)
{
  return sbuf->size;
}

inline char string_buff_get_char(STRING_BUFFER *sbuf, const t_buf_pos index)
{
  return sbuf->buff[index];
}

inline void string_buff_set_char(STRING_BUFFER *sbuf, const t_buf_pos index, const char c)
{
  sbuf->buff[index] = c;
}

char string_buff_resize(STRING_BUFFER *sbuf, const t_buf_pos new_size)
{
  char *new_buff = realloc(sbuf->buff, new_size);

  if(new_buff == NULL)
  {
    return 0;
  }

  sbuf->buff = new_buff;
  sbuf->size = new_size;

  if(sbuf->len+1 >= sbuf->size)
  {
    // Buffer was shrunk - add null byte
    sbuf->len = sbuf->size-1;
    sbuf->buff[sbuf->len] = '\0';
  }

  return 1;
}

void string_buff_resize_vital(STRING_BUFFER *sbuf, const t_buf_pos new_size)
{
  if(!string_buff_resize(sbuf, new_size))
  {
    fprintf(stderr, "Error: STRING_BUFFER couldn't be given more memory.  "
                    "Requested %lui bytes.  STRING_BUFFER begins '%s...'",
            new_size, string_buff_substr(sbuf, 0, 5));
    
    free(sbuf->buff);
    
    exit(EXIT_FAILURE);
  }
}

// Ensure capacity for len characters plus '\0' character
void string_buff_ensure_capacity(STRING_BUFFER *sbuf, const t_buf_pos len)
{
  if(sbuf->size > len+1)
  {
    return;
  }

  // Need to resize
  t_buf_pos new_size = 2*sbuf->size;

  while(len+1 >= new_size)
  {
    new_size = 2*new_size;
  }

  string_buff_resize_vital(sbuf, new_size);
}

void string_buff_shrink(STRING_BUFFER *sbuf, const t_buf_pos new_len)
{
  sbuf->len = new_len;
  sbuf->buff[new_len] = '\0';
}

/********************/
/* String functions */
/********************/

void string_buff_append_str(STRING_BUFFER* sbuf, const char* txt)
{
  size_t str_len = strlen(txt);
  string_buff_append_strn(sbuf, txt, str_len);
}

void string_buff_append_strn(STRING_BUFFER* sbuf, const char* txt,
                             const t_buf_pos len)
{
  // plus 1 for '\0'
  string_buff_ensure_capacity(sbuf, sbuf->len + len);

  strncpy(sbuf->buff+sbuf->len, txt, len);
  sbuf->len += len;

  sbuf->buff[sbuf->len] = '\0';
}

void string_buff_append_char(STRING_BUFFER* sbuf, const char c)
{
  // plus 1 for '\0'
  string_buff_ensure_capacity(sbuf, sbuf->len + 1);

  sbuf->buff[(sbuf->len)++] = c;
  sbuf->buff[sbuf->len] = '\0';
}

void string_buff_chomp(STRING_BUFFER *sbuf)
{
  while(sbuf->len >= 1)
  {
    char last_char = sbuf->buff[sbuf->len-1];

    if(last_char == '\n' || last_char == '\r')
    {
      sbuf->buff[--(sbuf->len)] = '\0';
    }
    else
    {
      break;
    }
  }
}

char* string_buff_substr(STRING_BUFFER *sbuf, const t_buf_pos start,
                         const t_buf_pos len)
{
  char* mem_start = sbuf->buff + start;
  t_buf_pos mem_length = MIN(len, sbuf->buff + sbuf->len - mem_start);

  if (mem_length < 0)
  {
    fprintf(stderr, "string_buff_substr(%lui, %lui) end < start on "
                    "STRING_BUFFER with length %lui\n",
            start, len, sbuf->len);
    exit(-1);
  }

  char* new_string = (char*) malloc(mem_length+1);
  strncpy(new_string, mem_start, mem_length);
  new_string[mem_length] = '\0';

  return new_string;
}

void string_buff_to_uppercase(STRING_BUFFER *sbuf)
{
  char* pos;
  char* end = sbuf->buff + sbuf->len;

  for(pos = sbuf->buff; pos < end; pos++)
  {
    *pos = toupper(*pos);
  }
}

void string_buff_to_lowercase(STRING_BUFFER *sbuf)
{
  char* pos;
  char* end = sbuf->buff + sbuf->len;

  for(pos = sbuf->buff; pos < end; pos++)
  {
    *pos = tolower(*pos);
  }
}

void string_buff_copy(STRING_BUFFER* dst, const t_buf_pos dst_pos,
                      STRING_BUFFER* src, const t_buf_pos src_pos,
                      const t_buf_pos len)
{
  // Check if dest buffer can handle string plus \0
  string_buff_ensure_capacity(dst, dst_pos + len);

  strncpy(dst->buff+dst_pos, src->buff+src_pos, len);

  if(dst_pos + len > dst->len)
  {
    // Extended string - add '\0' char
    dst->len = dst_pos + len;
    dst->buff[dst->len] = '\0';
  }
}

/*****************/
/* File handling */
/*****************/

// returns number of characters read
// or -1 if EOF
t_buf_pos string_buff_reset_readline(STRING_BUFFER *sbuf, gzFile *gz_file)
{
  string_buff_reset(sbuf);
  return string_buff_readline(sbuf, gz_file);
}

// returns number of characters read
// or 0 if EOF
t_buf_pos string_buff_readline(STRING_BUFFER *sbuf, gzFile *gz_file)
{
  t_buf_pos init_str_len = sbuf->len;

  // Enlarge *str allocated mem if needed
  // Need to be able to read it AT LEAST one character
  string_buff_ensure_capacity(sbuf, sbuf->len+1);

  // max characters to read = sbuf.size - sbuf.len
  while(gzgets(gz_file,
               (char*)(sbuf->buff + sbuf->len),
               sbuf->size - sbuf->len) != Z_NULL)
  {
    // Check if we hit the end of the line
    t_buf_pos num_of_chars_read = (t_buf_pos)strlen(sbuf->buff + sbuf->len);
    char* last_char = (char*)(sbuf->buff + sbuf->len + num_of_chars_read - 1);

    // Get the new length of the string buffer
    // count should include the return chars
    sbuf->len += num_of_chars_read;
    
    if(*last_char == '\n' || *last_char == '\r')
    {
      // Return characters read
      return sbuf->len - init_str_len;
    }
    else
    {
      // Hit end of buffer - double buffer size
      string_buff_resize_vital(sbuf, 2*sbuf->size);
    }
  }

  t_buf_pos total_chars_read = sbuf->len - init_str_len;

  return total_chars_read;
}


/**************************/
/* Other String Functions */
/**************************/
long split_str(const char* split, const char* txt, char*** result)
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
      arr = (char**) malloc(txt_len * sizeof(char*));
    
      t_buf_pos i;

      for(i = 0; i < txt_len; i++)
      {
        arr[i] = (char*) malloc(2 * sizeof(char));
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
  arr = (char**) malloc(count * sizeof(char*));
  
  count = 0;
  const char* last_position = txt;

  while((find = strstr(last_position, split)) != NULL)
  {
    long str_len = find - last_position;

    arr[count] = (char*) malloc((str_len+1) * sizeof(char));
    strncpy(arr[count], last_position, str_len);
    arr[count][str_len] = '\0';
    
    count++;
    last_position = find + split_len;
  }

  // Copy last item
  long str_len = txt + txt_len - last_position;
  arr[count] = (char*) malloc((str_len+1) * sizeof(char));

  if(count == 0)
  {
    strcpy(arr[count], txt);
  }
  else
  {
    strncpy(arr[count], last_position, str_len);
  }

  arr[count][str_len] = '\0';
  count++;
  
  *result = arr;
  
  return count;
}
