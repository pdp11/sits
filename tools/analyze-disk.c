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
#include <string.h>

#include "libword.h"

#define BLOCK_OCTETS 1024

/* Directory entry types. */
#define PRENT   000
#define DIREC   002
#define FILEX   004
#define LAST    006	
#define SELF    010
#define LINK    012
#define ALFORQ  014
#define BITS    016

/* Directory entry flags. */
#define MFISTB      1
#define TYPEM 0177761
#define SHRB     0020
#define ACCB     0100
#define EOFB     0200

/* Descriptor byte types. */
#define DTYPEF 0300	/* Type-of-descriptor-byte field. */
#define DTSKP  0000	/* Skip N and grab 1 block. */
#define DTCNT  0100	/* Grab N+1 blocks. */
#define DTHOL  0140	/* Hole of N+1 blocks. */
#define DTSADR 0200	/* Grab N+1 blocks starting at X (set addr). */
#define DTSKCT 0300	/* Skip N1 and grab N2+1 blocks. */
#define DTCNTF 0077	/* For first three types, the N field. */
#define DTSCSK 0070	/* For skip/count, the skip field N1. */
#define DTSCCT 0007	/*  "      "    , the count field N2. */

typedef unsigned char octet;

/* Size of an RK05. */
octet image[2436 * BLOCK_OCTETS];
int blocks;

typedef struct {
  char *path;
  int block[1000];
  int blocks;
} dir_t;

static int
get_byte (FILE *f)
{
  int c = fgetc (f);
  return c == EOF ? -1 : c;
}

static int read_block (FILE *f, octet *buffer)
{
  int i;

  for (i = 0; i < BLOCK_OCTETS; i++)
    {
      int x = get_byte (f);
      if (x == -1)
	return -1;
      buffer[i] = x;
    }

  return 0;
}

static octet *
get_block (int block)
{
  return &image[BLOCK_OCTETS * block];
}

static int
get_pdp11_word (octet *buffer)
{
  return (buffer[1] << 8) + buffer[0];
}

static void
show_block (octet *block)
{
  int i, j;

  for (i = 0; i < 512; i += 8 * 2)
    {
      printf ("%06o %06o %06o %06o %06o %06o %06o %06o ",
              get_pdp11_word (block + i),
              get_pdp11_word (block + i + 2),
              get_pdp11_word (block + i + 4),
              get_pdp11_word (block + i + 6),
              get_pdp11_word (block + i + 8),
              get_pdp11_word (block + i + 10),
              get_pdp11_word (block + i + 12),
              get_pdp11_word (block + i + 14));
      for (j = 0; j < 16; j++)
        {
          if (block[i+j] < 040 || block[i+j] > 0176)
            putchar ('.');
          else
            putchar (block[i+j]);
        }
      putchar ('\n');
    }
}

static void
show_flags (octet flags)
{
  printf ("Flags %03o ", flags);
  switch (flags & ~TYPEM)
    {
    case PRENT: printf ("parent "); break;
    case DIREC: printf ("directory "); break;
    case FILEX: printf ("file "); break;
    case LAST: printf ("(last) "); break;
    case SELF: printf ("self "); break;
    case LINK: printf ("link "); break;
    case ALFORQ: printf ("alforq "); break;
    case BITS: printf ("bits "); break;
    }
  if (flags & MFISTB)
    printf ("exists ");
  if (flags & SHRB)
    printf ("shared ");
  if (flags & ACCB)
    printf ("access ");
  if (flags & EOFB)
    printf ("eof ");
  putchar ('\n');
}

static int
show_name (char *name, octet *block, int i)
{
  do
    *name++ = block[i] & 0177;
  while ((block[i++] & 0200) == 0);
  name[0] = 0;
  return i;
}

static void
xxx (int i, int n)
{
  if (n > 1)
    printf ("{%o-%o}", i, i + n - 1);
  else
    printf ("{%o}", i);
}

static void
show_file (int x, octet *d, int n)
{
  int i;
  printf ("[file:");
  for (i = 0; i < n; i++)
    {
      switch (d[i] & DTYPEF)
	{
	case DTSKP:
	  x += d[i] & DTCNTF;
	  xxx (x, 1);
	  x++;
	  break;
	case DTCNT:
	  xxx (x, (d[i] & DTCNTF) + 1);
	  x += (d[i] & DTCNTF) + 1;
	  break;
	case DTSADR:
	  xxx (d[i+1] + (d[i+2] << 8), (d[i] & DTCNTF) + 1);
	  i += 2;
	  break;
	case DTSKCT:
	  x += (d[i] & DTSCSK) >> 3;
	  xxx (x, (d[i] & DTSCCT) + 1);
	  x += (d[i] & DTSCCT) + 1;
	  break;
	}
    }
  printf ("]");
}

static void
parse_descriptor (int *block, int *blocks, int x, octet *d, int n)
{
  int i, j;
  *blocks = 0;
  for (i = 0; i < n; i++)
    {
      switch (d[i] & DTYPEF)
	{
	case DTSKP:
	  x += d[i] & DTCNTF;
	  block[(*blocks)++] = x;
	  x++;
	  break;
	case DTCNT:
	  for (j = 0; j <= (d[i] & DTCNTF); j++)
	    block[(*blocks)++] = x++;
	  break;
	case DTSADR:
	  x = d[i+1] + (d[i+2] << 8);
	  for (j = 0; j <= (d[i] & DTCNTF); j++)
	    block[(*blocks)++] = x++;
	  i += 2;
	  break;
	case DTSKCT:
	  x += ((d[i] & DTSCSK) >> 3);
	  for (j = 0; j <= (d[i] & DTSCCT); j++)
	    block[(*blocks)++] = x++;
	  break;
	}
    }
}

static void
write_block (FILE *f, octet *block, unsigned long size)
{
  if (size > BLOCK_OCTETS)
    size = BLOCK_OCTETS;
  fwrite (block, 1, size, f);
}

static void
write_file (const char *path, int *block, int blocks, unsigned long size)
{
  int i;
  FILE *f = fopen (path, "wb");
  for (i = 0; i < blocks; i++, size -= BLOCK_OCTETS)
    write_block (f, get_block (block[i]), size);
  fclose (f);
}

static void
print_timestamp (FILE *f, int date, int time)
{
  if (date != 0 && date != 0177777)
    fprintf (f, "%4d-%02d-%02d ",
	     1900 + ((date >> 9) & 0177),
	     (date >> 5) & 017,
	     date & 037);
  else
    fprintf (f, "- ");
  if (time != 0 && time != 0177777)
    fprintf (f, "%02d:%02d:%02d",
	     (time >> 11) & 037,
	     (time >> 5) & 077,
	     2 * (time & 037));
  else
    fprintf (f, "-");
  if (date != 0 && date != 0177777
      && ((date >> 9) < 75 || (date >> 9) > 78))
    fprintf (f, " [%06o]", date);
}

static void
show_directory (dir_t *dir)
{
  dir_t subdirs[100];
  dir_t *subdir = subdirs;
  char path[200];
  char name[100];
  octet *block;
  unsigned long size;
  int flags;
  int dirend;
  int i = 0, j;
  int n, x;

  x = dir->block[0];
  block = get_block (x);

  dirend = get_pdp11_word (block + 6);
  printf ("Directory %s ------------------\n", dir->path);

  while (i < dirend)
    {
      int vern;

      n = block[i];
      flags = block[i+1];
      show_flags (flags);

      vern = get_pdp11_word (block + i + 2);
      printf ("Version %o\n", vern);
      j = i + 4;
      size = ~0UL;
      if (flags & EOFB)
	{
	  printf ("EOF page pointer %d\n", get_pdp11_word (block + j));
	  printf ("EOF byte pointer %d\n", get_pdp11_word (block + j + 2));
	  size = 8192 * get_pdp11_word (block + j);
	  size += get_pdp11_word (block + j + 2);
	  printf ("Timestamp: ");
	  print_timestamp (stdout,
			   get_pdp11_word (block + j + 4),
			   get_pdp11_word (block + j + 6));
	  putchar ('\n');
	  j += 8;
	}

      if (flags & ACCB)
	{
	  do
	    j +=3;
	  while ((block[j-1] & 0200) == 0);
	}

      j = show_name (name, block, j);
      printf ("Name: %s", name);
      if (vern != 0 && vern != 0177777)
	printf ("#%d", vern);
      putchar ('\n');

      x = 0;
      parse_descriptor (subdir->block, &subdir->blocks, x, &block[j], i+n-j);
      {
	int k;
	printf ("<<");
	for (k = 0; k < subdir->blocks; k++)
	  printf (" %o", subdir->block[k]);
	printf (">>");
      }

      switch (flags & ~TYPEM)
	{
	case FILEX:
	  if (vern != 0 && vern != 0177777)
	    sprintf (path, "%s %s#%d", dir->path, name, vern);
	  else
	    sprintf (path, "%s %s", dir->path, name);
	  write_file (path, subdir->block, subdir->blocks, size);
	  break;
	case DIREC:
	  sprintf (path, "%s %s", dir->path, name);
	  subdir->path = strdup (path);
	  subdir++;
	  break;
	}

      n = (n + 1) & ~1;
      i += n;
    }

  for (dir = subdirs; dir != subdir; dir++)
    {
      printf ("Subdir: %s\n", dir->path);
      show_directory (dir);
    }
}

int
main (int argc, char **argv)
{
  static dir_t root = { NULL, { 046 }, 1};
  octet *buffer;
  FILE *f;

  if (argc != 3)
    {
      fprintf (stderr, "Usage: %s <file> <name>\n", argv[0]);
      exit (1);
    }

  f = fopen (argv[1], "rb");

  root.path = argv[2];

  buffer = image;
  blocks = 0;
  for (;;)
    {
      int n = read_block (f, buffer);
      if (n == -1)
	break;
      buffer += BLOCK_OCTETS;
      blocks++;
    }

  fprintf (stderr, "%o blocks in image\n", blocks);

  printf ("Block: %03o\n", 046);
  show_directory (&root);

  return 0;
}
