/*
 sb_test.c
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

#include <stdlib.h>
#include <stdio.h>

#include "string_buffer.h"

void _test_split(char* split, char* txt)
{
  char** results;
  
  printf("split '%s' by '%s':\n", txt, split);
  
  long count = split_str(split, txt, &results);

  if(count > 0)
  {
    printf("'%s'", results[0]);
  
    int i;
    for(i = 1; i < count; i++)
    {
      printf(",'%s'", results[i]);
      free(results[i]);
    }
    free(results);
  }

  printf("\n");
}

void _test_add_char()
{
  STRING_BUFFER* sbuf = string_buff_init(100);
  
  string_buff_append_char(sbuf, 'a');
  string_buff_append_char(sbuf, 'b');
  printf("'%s' (length: %lu)\n", sbuf->buff, sbuf->len);
}

void _test_sprintf()
{
  printf("printf:\n");
  STRING_BUFFER* sbuf = string_buff_init(100);
  
  string_buff_sprintf(sbuf, "hi ello");
  printf("'%s' (length: %lu)\n", sbuf->buff, sbuf->len);
  
  string_buff_sprintf_noterm(sbuf, 0, "woot %i %s;", 12, "byebye");
  printf("'%s' (length: %lu)\n", sbuf->buff, sbuf->len);

  char *a = "wooo-%s-xx";
  char *b = "hihi";

  string_buff_sprintf_at(sbuf, string_buff_strlen(sbuf), a, b);
  printf("'%s' (length: %lu)\n", sbuf->buff, sbuf->len);

  string_buff_reset(sbuf);
  string_buff_resize(sbuf, 10);
  string_buff_append_str(sbuf, "asdfasdf");

  string_buff_sprintf(sbuf, a, b);
  printf("'%s' (length: %lu)\n", sbuf->buff, sbuf->len);
}

int main(int argc, char* argv[])
{
  /*
  _test_split("/", "a/b");
  _test_split("/", "/");
  _test_split("/", "/b");
  _test_split("/", "a/");
  _test_split("/", "asdf");
  _test_split("/", "");
  _test_split("", "asdf");
  _test_split("", "");
  */
  
  _test_add_char();
  
  _test_sprintf();
  
  return EXIT_SUCCESS;
}
