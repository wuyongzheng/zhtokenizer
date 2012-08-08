#include <stdio.h>
#include <assert.h>

int decode_utf8 (const unsigned char *instr, unsigned int *punival)
{
	const unsigned char firstchar = instr[0];
	if (firstchar < 0x80) {
		*punival = firstchar;
		return 1;
	} else if (firstchar < 0xc0) {
		return 0;
	} else if (firstchar < 0xe0) {
		if ((instr[1] & 0xc0) != 0x80)
			return 0;
		*punival = ((firstchar & 0x1f) << 6) | (instr[1] & 0x3f);
		return 2;
	} else if (firstchar < 0xf0) {
		if ((instr[1] & 0xc0) != 0x80 || (instr[2] & 0xc0) != 0x80)
			return 0;
		*punival = ((firstchar & 0x0f) << 12) |
			((instr[1] & 0x3f) << 6) |
			(instr[2] & 0x3f);
		return 3;
	} else if (firstchar < 0xf8) {
		if ((instr[1] & 0xc0) != 0x80 || (instr[2] & 0xc0) != 0x80 ||
				(instr[3] & 0xc0) != 0x80)
			return 0;
		*punival = ((firstchar & 0x07) << 18) |
			((instr[1] & 0x3f) << 12) |
			((instr[2] & 0x3f) << 6) |
			(instr[3] & 0x3f);
		return 4;
	}
	return 0;
}

/* decode all instr, ignore error by skipping bytes, ignore unicode character > 0xffff
 * decoding stopps if outlen reached.
 * outstr is not null-termed.
 * return number of character in outstr. */
size_t decode_utf8_str (const unsigned char *instr, size_t inlen, unsigned short *outstr, size_t outlen)
{
	size_t inptr = 0, outptr = 0;
	while (inptr < inlen && outptr < outlen) {
		unsigned int unival;
		int len = decode_utf8(instr+inptr, &unival);
		if (len == 0) {
			inptr ++;
		} else {
			inptr += len;
			if (unival <= 0xffff)
				outstr[outptr ++] = unival;
		}
	}
	return outptr;
}

/* given a unicode character "unival", generate the UTF-8
 * string in "outstr", return the length of it.
 * The output string is NOT null terminated. */
int encode_utf8 (unsigned char *outstr, unsigned int unival)
{
	if (unival < 0x80) {
		outstr[0] = unival;
		return 1;
	}
	if (unival < 0x800) {
		outstr[0] = 0xc0 | (unival >> 6);
		outstr[1] = 0x80 | (unival & 0x3f);
		return 2;
	}
	if (unival < 0x10000) {
		outstr[0] = 0xe0 | (unival >> 12);
		outstr[1] = 0x80 | ((unival >> 6) & 0x3f);
		outstr[2] = 0x80 | (unival & 0x3f);
		return 3;
	}
	if (unival < 0x110000) {
		outstr[0] = 0xf0 | (unival >> 18);
		outstr[1] = 0x80 | ((unival >> 12) & 0x3f);
		outstr[2] = 0x80 | ((unival >> 6) & 0x3f);
		outstr[3] = 0x80 | (unival & 0x3f);
		return 4;
	}
	assert(0);
}

/* return strlen of outstr.
 * The output string is null terminated. */
int encode_utf8_str (unsigned char *outstr, size_t outlen, const unsigned short *instr, size_t inlen)
{
	int inptr, outptr;
	for (inptr = outptr = 0; inptr < inlen && outptr + 6 < outlen; ) {
		outptr += encode_utf8(outstr + outptr, instr[inptr ++]);
	}
	outstr[outptr] = '\0';
	return outptr;
}

