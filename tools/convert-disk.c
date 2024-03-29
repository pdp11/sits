/* Copyright (C) 2019, 2022 Lars Brinkhoff <lars@nocrew.org>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include <stdio.h>
#include <stdlib.h>

#include "libword.h"

int
main (int argc, char **argv)
{
  word_t word;
  FILE *f;

  if (argc != 2)
    {
      fprintf (stderr, "Usage: %s <file>\n", argv[0]);
      exit (1);
    }

  f = fopen (argv[1], "rb");
  input_word_format = &its_word_format;

  while ((word = get_word (f)) != -1)
    {
      putchar ((word >> 20) & 0377);
      putchar ((word >> 28) & 0377);
      putchar ((word >>  4) & 0377);
      putchar ((word >> 12) & 0377);
    }

  return 0;
}
