unsigned int ascii_length(const char *in);
unsigned int ascii_toint(unsigned char c);
unsigned int ascii_fromint(void *out, unsigned int count, unsigned int value, unsigned int base);
unsigned int ascii_rvalue(const char *in, unsigned int count, unsigned int base);
unsigned int ascii_wvalue(void *out, unsigned int count, unsigned int value, unsigned int base, unsigned int offset);
unsigned int ascii_wzerovalue(void *out, unsigned int count, unsigned int value, unsigned int base, unsigned int padding, unsigned int offset);
