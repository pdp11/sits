/* Copyright (C) 2022 Lars Brinkhoff <lars@nocrew.org>

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

#define EOT 066
#define SIZE 020000
#define O_SYMS -100

static int csize = SIZE;
static int finflg;
static int bcount;
static int tapes = 1;

static word_t buff1[] =
{
  1,   //0
  106,
  O_SYMS, //3
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0,
  0127570, /* .C */
  0,
  025,
  0130600, /* .P */
  0,
  011,
  0130410, /* .M */
  0,
  012,
  0130770, /* .S */
  0,
  010,
  0127520, /* .B */
  0,
  015,
  0131542, /* ..B */
  0,
  0,
  0127400, /* . */
  0,
  0,
  1,
  0174
};

static word_t *bf1st = buff1 + 29;
static word_t *__b = buff1 + 47;
static word_t *r1 = buff1 + 52;

static word_t buff2[] =
{
  1,
  106,
  O_SYMS,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0,
  0,
  0,
  0
};

static word_t *ldad = buff2 + 2;
static word_t *buffst = buff2 + 50;
static word_t *rflgs = buff2 + 52;

static word_t fbuff[] =
{
  1,
  6,
  0100000 /* jumpt to debugger */
};

static void
send (int x)
{
  putchar (x & 0377);
  csize--;
}

static int
gsend (FILE *f)
{
  int x = get_word (f);
  bcount--;
  send (x);
  return x;
}

static int
gswrd (FILE *f)
{
  int x = gsend (f);
  int y = gsend (f);
  if (x == -1 || y == -1)
    return -1;
  return ((y & 0377) << 8) | (x & 0377);
}

static void
leader (int n)
{
  while (n-- >= 0)
    send (0);
}

static void
newblk ()
{
  if (csize >= 0)
    return;

  send (EOT);
  leader (80);
  /* Mount a fresh paper tape. */
  tapes++;
  leader (100);
  csize = SIZE;
}

static void
program (FILE *f)
{
  word_t data;

  for (;;)
    {
      newblk ();

      do
        {
          data = get_word (f);
          if (data == -1)
            exit (1);
        }
      while (data != 1);

      leader (5);
      send (1);
      gsend (f);
      bcount = gswrd (f);
      bcount -= 3;
      if (bcount == 3)
        {
          gswrd (f);
          gsend (f);
          return;
        }
      while (bcount > 0)
        gsend (f);
    }
}

static int
cnvt (int s)
{
  if (s >= 041 && s <= 072)
    return s - 040;
  if (s == 4)
    return 033;
  if (s == 016)
    return 034;
  if (s <= 031 && s >= 020)
    return s + 016;
  return 0;
}

static int
loop50 (word_t *a)
{
  int f = 0;
  bcount = 3;
  do
    {
      f = 050 * f + cnvt ((*a >> 30) & 077);
      *a <<= 6;
    }
  while (--bcount > 0);
  return f;
}

static void
rad50 (word_t a, word_t *b)
{
  word_t f;
  f = loop50 (&a);
  b[-2] = f & 0777777;
  f = loop50 (&a);
  b[-1] = f;
}

static int
setup (FILE *f, int c, word_t *b, word_t *reg)
{
  word_t a;
  int d;
  do
    {
      a = get_word (f);
      if (a == 0 || a == -1)
        return c;
      rad50 (a, b);
      a = get_word (f);
      *b = a & 0777777;
      d = 1 << (c - 1);
      if (a & 04000000000LL)
        reg[0] |= d;
      if (a & 020000000000LL)
        reg[-1] |= d;
      b -= 3;
      c++;
    }
  while (c <= 16);
  return 0;
}

static void
sndb (word_t *c)
{
  int a;
  int cksm = 0;
  newblk ();
  bcount = c[1];
  cksm = 0;
  do
    {
      a = *c;
      cksm += a;
      send (a);
      a >>= 8;
      cksm += a;
      send (a);
      bcount -= 2;
      c++;
    }
  while (bcount > 0);
  send (-cksm & 0377);
  leader (10);
}

static void
setup1 (FILE *f)
{
  word_t c;
  finflg = 0;
  c = setup (f, 8, bf1st, r1);
  if (c == 0)
    return;
  c = 6 * (16 - c);
  *ldad += c;
  *__b = c;
  finflg = -1;
}

static void
setupu (FILE *f)
{
  word_t c;
  do
    {
      rflgs[0] = 0;
      rflgs[-1] = 0;
      *ldad -= 100;
      c = setup (f, 1, buffst, rflgs);
      *__b = *ldad;
      if (c != 0)
        {
          *__b +=  6 * (16 - c);
          finflg = -1;
        }
      sndb (buff2);
    }
  while (finflg == 0);
}

static void
symbols (FILE *f)
{
  word_t data;

  do
    {
      data = get_word (f);
      if (data == -1)
        exit (0);
    }
  while (data != 2);

  setup1 (f);
  if (finflg == 0)
    setupu (f);
  sndb (buff1);
  sndb (fbuff);
}

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

  program (f);
  leader (100);
  symbols (f);
  leader (10);
  fprintf (stderr, "Punched %d tapes.\n", tapes);
  return 0;
}
