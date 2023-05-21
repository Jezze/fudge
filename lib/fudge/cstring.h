unsigned int cstring_length(char *in);
unsigned int cstring_length_zero(char *in);
unsigned int cstring_match(char *in1, char *in2);
unsigned int cstring_match_substring(char *in1, char *in2);
unsigned int cstring_isalpha(char c);
unsigned int cstring_isdigit(char c);
unsigned int cstring_toint(char c);
unsigned int cstring_read_value(char *in, unsigned int count, unsigned int base);
unsigned int cstring_write(char *out, unsigned int count, char *in, unsigned int offset);
unsigned int cstring_write_zero(char *out, unsigned int count, unsigned int offset);
unsigned int cstring_write_value(char *out, unsigned int count, int value, unsigned int base, unsigned int padding, unsigned int offset);
unsigned int cstring_write_fmt(void *out, unsigned int count, char *fmt, unsigned int nargs, void **args, unsigned int offset);
unsigned int cstring_write_fmt0(void *out, unsigned int count, char *fmt, unsigned int offset);
unsigned int cstring_write_fmt1(void *out, unsigned int count, char *fmt, unsigned int offset, void *arg1);
unsigned int cstring_write_fmt2(void *out, unsigned int count, char *fmt, unsigned int offset, void *arg1, void *arg2);
unsigned int cstring_write_fmt3(void *out, unsigned int count, char *fmt, unsigned int offset, void *arg1, void *arg2, void *arg3);
unsigned int cstring_write_fmt4(void *out, unsigned int count, char *fmt, unsigned int offset, void *arg1, void *arg2, void *arg3, void *arg4);
unsigned int cstring_write_fmt6(void *out, unsigned int count, char *fmt, unsigned int offset, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6);
unsigned int cstring_write_fmt8(void *out, unsigned int count, char *fmt, unsigned int offset, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6, void *arg7, void *arg8);
