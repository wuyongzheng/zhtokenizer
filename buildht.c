#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "config.h"
#include "utf8util.h"

unsigned short *winfo; // high: weight; low: length
unsigned int *woffset;
unsigned short **words;
int words_num = 0, words_cap = 0, words_offset = 0;
unsigned int *ht_table;
int ht_size;

static int next_prime (int i)
{
	static const int smallprime [] = {
		3,5,7,11,13,17,19,23,29,31,
		37,41,43,47,53,59,61,67,71,
		73,79,83,89,97,0};
	assert(i > 0);
	if (i % 2 == 0)
		i ++;
	for (; ; i += 2) {
		int j;
		for (j = 0; smallprime[j] && i % smallprime[j] != 0; j ++)
			;
		if (smallprime[j]) {
			if (i == smallprime[j])
				return i;
			continue;
		}
		for (j = 101; j*j < i; j ++)
			if (i % j == 0)
				break;
		if (j*j > i)
			return i;
	}
}

/* manual sync with ht_calhash in htfunc.c */
static unsigned long ht_calhash (const unsigned short *wstr, int len)
{
	unsigned long hashval = sizeof(unsigned long) == 8 ? 14695981039346656037ul : 2166136261ul;
	while ((len --) > 0)
		hashval = (hashval * (sizeof(unsigned long) == 8 ? 1099511628211ul : 16777619ul)) ^
			(*(wstr ++));
	return hashval;
}

void load_words (void)
{
	char buffer[4096];
	while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
		if (words_cap == 0) {
			words_cap = 8192;
			winfo = calloc(words_cap, sizeof(unsigned short));
			woffset = calloc(words_cap, sizeof(unsigned int));
			words = calloc(words_cap, sizeof(unsigned short *));
		} else if (words_num >= words_cap) {
			words_cap *= 2;
			winfo = realloc(winfo, sizeof(unsigned short) * words_cap);
			woffset = realloc(woffset, sizeof(unsigned int) * words_cap);
			words = realloc(words, sizeof(unsigned short *) * words_cap);
		}

		size_t len = strlen(buffer);
		assert(len >= 2);
		assert(buffer[len-1] == '\n');
		assert(buffer[len-2] != '\r' && buffer[len-2] != '\n');
		buffer[-- len] = '\0';

		char *weistr = strchr(buffer, '\t');
		assert(weistr != NULL && weistr > buffer);
		*weistr = '\0';
		weistr ++;
		int weight = atoi(weistr);
		assert(weight > 0 && weight < 20);

		unsigned short word[1000];
		size_t wordlen = decode_utf8_str((unsigned char *)buffer, weistr - buffer - 1, word, 1000);
		if (wordlen < MINWORDLEN || wordlen > MAXWORDLEN)
			continue;
		{
			int i;
			for (i = 0; i < wordlen; i ++)
				if (word[i] < ZHLOW || word[i] > ZHHIGH)
					break;
			if (i != wordlen)
				continue;
		}

		winfo[words_num] = (weight << 8) | wordlen;
		words[words_num] = malloc(wordlen * sizeof(unsigned short));
		memcpy(words[words_num], word, wordlen * sizeof(unsigned short));
		woffset[words_num] = words_offset;
		words_num ++;
		words_offset += wordlen + 1;
	}
}

void build_ht (void)
{
	ht_size = next_prime(words_num * 2);
	ht_table = calloc(ht_size, sizeof(unsigned int));

	int i;
	for (i = 0; i < words_num; i ++) {
		int len = winfo[i] & 0xff;
		unsigned long hashval = ht_calhash(words[i], winfo[i] & 0xff);
		int step = (hashval / ht_size) % (ht_size - 1) + 1;
		int j;
		for (j = hashval % ht_size; ht_table[j]; j = (j + step) % ht_size) {
			if ((winfo[ht_table[j]] & 0xff) == len && memcmp(words[ht_table[j]], words[i], len * sizeof(unsigned short)) == 0) {
				printf("duplicated entry %d %d\n", ht_table[j], i);
				exit(1);
			}
		}
		ht_table[j] = i;
	}
}

void make_h (const char *path)
{
	FILE *fp = fopen(path, "w");
	assert(fp != NULL);

	fprintf(fp, "#define GENHT_SIZE %d\n", ht_size);
	fprintf(fp, "#define GENHT_USED %d\n", words_num);
	fprintf(fp, "extern const uint16_t genht_words [];\n");
	fprintf(fp, "extern const uint32_t genht_table [];\n");

	fclose(fp);
}

void make_c (const char *path)
{
	int i, j;

	FILE *fp = fopen(path, "w");
	assert(fp != NULL);

	fprintf(fp, "#include <stdint.h>\n");
	fprintf(fp, "\n");

	fprintf(fp, "const uint16_t genht_words [] = {\n");
	for (i = 0; i < words_num; i ++) {
		fprintf(fp, "\t%hu", winfo[i]);
		for (j = 0; j < (winfo[i] & 0xff); j ++)
			fprintf(fp, ", %hu", words[i][j]);
		fprintf(fp, ",\n");
	}
	fprintf(fp, "};\n");

	fprintf(fp, "const uint32_t genht_table [] = {\n");
	for (i = 0; i < ht_size; i ++) {
		if (i % 12 == 0) {
			fprintf(fp, "\t%u, ", woffset[ht_table[i]]);
		} else if (i % 12 == 11) {
			fprintf(fp, "%u,\n", woffset[ht_table[i]]);
		} else {
			fprintf(fp, "%u, ", woffset[ht_table[i]]);
		}
	}
	fprintf(fp, "};\n");

	fclose(fp);
}

int main (int argc, char *argv[])
{
	if (argc != 3)
		return 1;

	load_words();
	build_ht();
	make_h(argv[1]);
	make_c(argv[2]);
	return 0;
}
