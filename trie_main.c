/* $Id: main.c.m4,v 1.7 2005/11/07 19:39:53 luis Exp $
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: Fri Dec 12 23:20:54 EET 2014
 *
 * Disclaimer:
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define IN_TRIE_C

/* Standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <avl.h>
#include "trie.h"

/* constants */

#define MAX 65536
#define N	512

void process_line(const char *n);
void process_file(const char *n);

/* variables */

char buffer[MAX]; /* buffer to store everything */
char *p; /* pointer to free space in buffer */
int bs; /* free space size */

char *strings[N]; /* strings table for the macros/strings */
int strings_n = 0; /* number of strings */

struct trie_node *main_trie = NULL;

static const char stdin_name[] = "<<stdin>>";

/* TODO: fill this function. */
void do_usage(void)
{
} /* do_usage */

/**
 * process a file reading its contents into a string.
 * @param n name of the file to open or NULL to read from stdin.
 */
void process_line(const char *n)
{
	FILE *fin = (n != stdin_name)
		? fopen(n, "r")
		: stdin;

	if (!fin) {
		fprintf(stderr,
			D("file %s: %s(errno=%d) fin=%p; stdin=%p\n"),
			n, strerror(errno), errno, fin, stdin);
		exit(EXIT_FAILURE);
	} /* if */

#if DEBUG
	printf(D("Processing [%s]: by lines BEGIN.\n"), n ? n : stdin_name);
#endif

	for (; (strings_n < N) && fgets(buffer, sizeof buffer, fin); strings_n++) {
		buffer[strlen(buffer)-1] = '\0'; /* eliminate the last \n char. */
		strings[strings_n] = strdup(buffer);
#if DEBUG
		printf(D("  line[%5d] = [%s]\n"), strings_n, strings[strings_n]);
#endif
	} /* while */

	if (n) fclose(fin);
#if DEBUG
	printf(D("Processing [%s]: by lines END.\n"), n ? n : stdin_name);
#endif
} /* process_line */

void process_file(const char *n)
{
	int in = (n != stdin_name)
		? open(n, O_RDONLY)
		: 0; /* stdin */
	int rd;

	if (in < 0) {
		fprintf(stderr,
			D("file %s: %s(errno=%d)\n"),
			n, strerror(errno), errno);
		exit(EXIT_FAILURE);
	} /* if */

#if DEBUG
	printf(D("Processing [%s]: in block BEGIN.\n"), n);
#endif

	for (; (strings_n < N)
		&& ((rd = read(in, buffer, sizeof buffer - 1)) > 0);
		strings_n++)
	{
		buffer[rd] = 0;
		strings[strings_n] = strdup(buffer);
		printf(D("  string[%5d] = [%s]\n"), strings_n, strings[strings_n]);
	} /* while */

	if (rd < 0) {
		fprintf(stderr,
			D("read %s: %s(errno=%d)\n"),
			n, strerror(errno), errno);
		exit(EXIT_FAILURE);
	} /* if */

	if (n) close(in);
#if DEBUG
	printf(D("Processing [%s]: in block END\n"), n);
#endif
} /* process_file */

/**
 * main program.  Processes all files and returns one string
 * for each file in strings[]. In the process, all strings are
 * used to construct the trie we use to get the substring with
 * great f value.
 */
int main (int argc, char **argv)
{

	extern int optind;
	extern char *optarg;
	int opt;
	void (*process)(const char *) = process_line;
	int i, mark;
	int n_passes = 64;

	while ((opt = getopt(argc, argv, "hflp:")) != EOF) {
		switch(opt) {
		case 'h':
		  do_usage(); exit(0);
		case 'f': process = process_file; break;
		case 'l': process = process_line; break;
		case 'p': n_passes = atol(optarg); break;
		} /* switch */
	} /* while */

	argc -= optind; argv += optind;

	if (argc) {
		for (i = 0; i < argc; i++)
			process(argv[i]);
	} else	process(stdin_name);

	mark = strings_n;

	for(i = 0x80; i < 0x80 + n_passes; i++) {
		struct trie_node *root_trie, *max;
		int j;
		char *o;
		struct ref_buff *ref;

#if DEBUG
		printf(D("PASS #0x%02x:\n"), i);
#endif
		assert(root_trie = new_trie());
		for (j = 0; j < strings_n; j++) {
			char *s;
			for (s = strings[j]; *s; s++)
				add_string(s, root_trie, strings[j]);
		} /* for */
		max = walk_trie(root_trie);

		if (max == root_trie) break;

#if DEBUG
		printf(D("  max: [%.*s], l=%d, n=%d\n"),
			max->l, max->refs->b, max->l, max->n);
#endif

		/* copy the string macro */
		o = strings[strings_n++] = malloc(max->l + 1);
		memcpy(o, max->refs->b, max->l);
		o += max->l;
		/* substitute the strings */
		for (ref = max->refs; ref; ref = ref->nxt) {
			char *s = ref->b + max->l;
			char *t = ref->b;
			*t++ = i;
			while (*s) *t++ = *s++;
			*t++ = '\0';
		} /* for */

		/* delete the trie as it is no more needed */
		del_trie(root_trie);
	} /* for */

	for (i = mark; i < strings_n; i++) {
#if DEBUG
		printf(D("  macro[0x%02x] = [%s]\n"), 0x80+i-mark, strings[i]);
#else
		printf("%s\n", strings[i]);
#endif
	} /* for */

#if !DEBUG
	puts("");
#endif

	for(i = 0; i < mark; i++) {
#if DEBUG
		printf(D("  string[%d] = [%s]\n"), i, strings[i]);
#else
		printf("%s\n", strings[i]);
#endif
	} /* for */

} /* main */

/* $Id: main.c.m4,v 1.7 2005/11/07 19:39:53 luis Exp $ */
