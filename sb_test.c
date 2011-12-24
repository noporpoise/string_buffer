/*
  sb_test.c
  project: string_buffer
  author: Isaac Turner <isaac.turner@dtc.ox.ac.uk>
  Copyright (C) 08-June-2011

  string_buffer test code
  build: gcc -o string_buffer_test -Wall -lz string_buffer.c sb_test.c
 
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
  
  string_buff_add_char(sbuf, 'a');
  string_buff_add_char(sbuf, 'b');
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
  
  return EXIT_SUCCESS;
}
