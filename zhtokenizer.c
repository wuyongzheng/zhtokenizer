#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include "config.h"
#include "generated_ht.h"
#include "htable.h"
#include "utf8util.h"

#define OF_LINE 1
#define OF_WORD 2
static int option_format = OF_WORD;
static int option_unmatch = 1;

static void process_sentence (const unsigned short *sentence, int length)
{
	/*{
		char buffer[6000];
		int len = encode_utf8_str((unsigned char *)buffer, sizeof(buffer), sentence, length);
		printf("%d %d %s\n", length, len, buffer);
	}*/

	assert(length > 0);
	if (length < MINWORDLEN)
		return;
	if (length > MAXSENTENCE)
		length = MAXSENTENCE;
	int score[MAXSENTENCE + MAXWORDLEN - MINWORDLEN + 1];
	int jump[MAXSENTENCE + MAXWORDLEN - MINWORDLEN + 1];

	int i;
	for (i = 1; i <= MAXWORDLEN; i ++) {
		score[length - MINWORDLEN + i] = 0;
		jump[length - MINWORDLEN + i] = 1;
	}
	for (i = length - MINWORDLEN; i >= 0; i --) {
		int max_score = score[i + 1];
		int max_jump = 1;
		int j;
		for (j = MINWORDLEN; j <= MAXWORDLEN; j++) {
			int weight = hs_get(sentence + i, j);
			if (weight == 0)
				continue;
			int thescore = score[i + j] + WORDSCORE(j, weight);
			if (thescore > max_score) {
				max_score = thescore;
				max_jump = j;
			}
		}
		score[i] = max_score;
		jump[i] = max_jump;
	}

	for (i = 0; i < length; i += jump[i]) {
		if (score[i] == score[i + jump[i]] && option_unmatch == 0)
			continue;
		char wordmbs[MAXWORDLEN * 6 + 1];
		encode_utf8_str((unsigned char *)wordmbs, sizeof(wordmbs), &sentence[i], jump[i]);
		if (option_format == OF_WORD)
			printf("%s ", wordmbs);
		else
			puts(wordmbs);
	}
	if (option_format == OF_WORD)
		puts("");
}

static void process_line (const unsigned short *line, int length)
{
	int start = -1;
	int i;
	for (i = 0; i < length; i ++) {
		if (line[i] >= ZHLOW && line[i] <= ZHHIGH) {
			if (start == -1)
				start = i;
		} else {
			if (start != -1) {
				process_sentence(line + start, i - start);
				start = -1;
			}
		}
	}
	if (start != -1)
		process_sentence(line + start, length - start);
}

static void process_file (FILE *fp)
{
	char line_mbs[65536];

	while (fgets(line_mbs, sizeof(line_mbs), stdin) != NULL) {
		if (line_mbs[0] == '\0')
			continue;
		int len_mbs = strlen(line_mbs);
		if (line_mbs[len_mbs - 1] == '\n')
			line_mbs[-- len_mbs] = '\0';

		unsigned short line_wcs[32 * 1024];
		int len_wcs = decode_utf8_str((const unsigned char *)line_mbs, len_mbs,
				line_wcs, sizeof(line_wcs)/sizeof(line_wcs[0]));

		process_line(line_wcs, len_wcs);
	}
}

void usage (void)
{
	printf("usage ...\n");
}

int main (int argc, char *argv[])
{
	int i;
	for (i = 1; i < argc; i ++) {
		if (strcmp(argv[i], "-h") == 0 ||
				strcmp(argv[i], "--help") == 0) {
			usage();
			return 0;
		} else if (strcmp(argv[i], "-l")) {
			option_format = OF_LINE;
		} else if (strcmp(argv[i], "-w")) {
			option_format = OF_WORD;
		} else {
			printf("unknown option %s\n", argv[i]);
			return 1;
		}
	}

	process_file(stdin);

	return 1;
}
