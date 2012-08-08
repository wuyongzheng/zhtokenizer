#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include "config.h"
#include "generated_ht.h"

/* manual sync with ht_calhash in buildht.c */
static unsigned long ht_calhash (const unsigned short *wstr, int len)
{
	unsigned long hashval = sizeof(unsigned long) == 8 ? 14695981039346656037ul : 2166136261ul;
	while ((len --) > 0)
		hashval = (hashval * (sizeof(unsigned long) == 8 ? 1099511628211ul : 16777619ul)) ^
			(*(wstr ++));
	return hashval;
}

int hs_get (const unsigned short *str, size_t len)
{
	assert(len >= MINWORDLEN && len <= MAXWORDLEN);
	assert(str[0] != 0);
	unsigned long hashval = ht_calhash(str, len);
	int step = (hashval / GENHT_SIZE) % (GENHT_SIZE - 1) + 1;
	int i;
	for (i = hashval % GENHT_SIZE; genht_table[i]; i = (i + step) % GENHT_SIZE) {
		if ((genht_words[genht_table[i]] & 0xff) == len &&
				memcmp(&genht_words[genht_table[i] + 1], str, len * sizeof(unsigned short)) == 0)
			return genht_words[genht_table[i]] >> 8;
	}
	return 0;
}

