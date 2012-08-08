#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "htable.h"
#include "utf8util.h"

int main (void)
{
	char line_mbs[1000];

	while (fgets(line_mbs, sizeof(line_mbs), stdin) != NULL) {
		if (line_mbs[0] == '\0')
			continue;
		int len_mbs = strlen(line_mbs);
		if (line_mbs[len_mbs - 1] == '\n')
			line_mbs[-- len_mbs] = '\0';

		unsigned short line_wcs[500];
		int len_wcs = decode_utf8_str((const unsigned char *)line_mbs, len_mbs,
				line_wcs, sizeof(line_wcs)/sizeof(line_wcs[0]));

		printf("%s %d\n", line_mbs, hs_get(line_wcs, len_wcs));
	}

	return 0;
}
