int decode_utf8 (const unsigned char *instr, unsigned int *punival);
size_t decode_utf8_str (const unsigned char *instr, size_t inlen, unsigned short *outstr, size_t outlen);
int encode_utf8 (unsigned char *outstr, unsigned int unival);
int encode_utf8_str (unsigned char *outstr, size_t outlen, const unsigned short *instr, size_t inlen);
