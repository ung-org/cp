/*
 * UNG's Not GNU
 *
 * Copyright (c) 2011-2017, Jakob Kaivo <jkk@ung.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#define _XOPEN_SOURCE 700
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <unistd.h>

enum flags {
	NONE = 0,
	FORCE = (1 << 1),
	INTERACTIVE = (1 << 2),
	PRESERVE = (1 << 3),
	RECURSE = (1 << 4),
};

static int cp(char *src, const char *target, enum flags flags)
{
	(void)flags;

	FILE *in = fopen(src, "rb");
	if (in == NULL) {
		fprintf(stderr, "cp: Couldn't open %s: %s\n", src, strerror(errno));
		return 1;
	}

	char targetpath[strlen(src) + strlen(target) + 2];
	strcpy(targetpath, target);

	struct stat st;	
	if (stat(targetpath, &st) == 0) {
		if (S_ISDIR(st.st_mode)) {
			sprintf(targetpath, "%s/%s", target, basename(src));
		} else if (flags & INTERACTIVE) {
			char *response = NULL;
			size_t n = 0;
			fprintf(stderr, "cp: Overwrite %s? ", targetpath);
			fflush(stderr);
			getline(&response, &n, stdin);
			if (response[0] == 'y') {
				unlink(targetpath);
				free(response);
			} else {
				free(response);
				return 1;
			}
		} else if (flags & FORCE) {
			unlink(targetpath);
		}
	}

	FILE *out = fopen(targetpath, "wb");
	if (out == NULL) {
		fprintf(stderr, "cp: Couldn't open %s: %s\n", targetpath, strerror(errno));
		return 1;
	}

	int c;
	while ((c = fgetc(in)) != EOF) {
		fputc(c, out);
	}

	fclose(in);
	fclose(out);

	return 0;
}

int main(int argc, char *argv[])
{
	enum flags flags = NONE;

	int c;
	while ((c = getopt(argc, argv, "PfipRHL")) != -1) {
		switch (c) {
		case 'f':
			flags |= FORCE;
			break;

		case 'H':
			//followsymlinksoncommandline = true;
			break;

		case 'i':
			flags |= INTERACTIVE;
			break;

		case 'L':
			//followallsymlinks = true;
			break;

		case 'P':
			//actonsymlinks = true;
			break;

		case 'p':
			flags |= PRESERVE;
			break;

		case 'R':
			flags |= RECURSE;
			break;

		default:
			return 1;
		}
	}

	if (optind > argc - 2) {
		fprintf(stderr, "cp: At least one source and exactly one target required\n");
		return 1;
	}

	int r = 0;
	do {
		r |= cp(argv[optind++], argv[argc-1], flags);
	} while (optind < argc - 1);

	return r;
}
