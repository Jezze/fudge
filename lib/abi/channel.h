void channel_dispatch(struct message_header *header, void *data);
unsigned int channel_send(unsigned int event);
unsigned int channel_sendto(unsigned int target, unsigned int event);
unsigned int channel_sendbuffer(unsigned int event, unsigned int count, void *data);
unsigned int channel_sendbufferto(unsigned int target, unsigned int event, unsigned int count, void *data);
unsigned int channel_sendstringz(unsigned int event, char *string);
unsigned int channel_sendstringzto(unsigned int target, unsigned int event, char *string);
unsigned int channel_sendfmt0(unsigned int event, char *fmt);
unsigned int channel_sendfmt1(unsigned int event, char *fmt, void *arg1);
unsigned int channel_sendfmt2(unsigned int event, char *fmt, void *arg1, void *arg2);
unsigned int channel_sendfmt3(unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3);
unsigned int channel_sendfmt4(unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4);
unsigned int channel_sendfmt6(unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6);
unsigned int channel_sendfmt8(unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6, void *arg7, void *arg8);
unsigned int channel_sendmessage(struct message *message);
unsigned int channel_sendmessageto(unsigned int target, struct message *message);
unsigned int channel_redirecttarget(unsigned int target, unsigned int event, unsigned int id);
unsigned int channel_redirectback(unsigned int target, unsigned int event);
unsigned int channel_pick(struct message_header *header, void *data);
unsigned int channel_process(void);
unsigned int channel_pollfrom(unsigned int source, struct message_header *header, void *data);
unsigned int channel_pollevent(unsigned int event, struct message_header *header, void *data);
unsigned int channel_polleventfrom(unsigned int source, unsigned int event, struct message_header *header, void *data);
unsigned int channel_kpoll(struct message_header *header, void *data);
unsigned int channel_kpollevent(unsigned int event, struct message_header *header, void *data);
unsigned int channel_read(unsigned int event, struct message_header *header, void *data);
unsigned int channel_readfrom(unsigned int source, unsigned int event, struct message_header *header, void *data);
unsigned int channel_wait(unsigned int source, unsigned int event);
void channel_bind(unsigned int event, void (*callback)(unsigned int source, void *mdata, unsigned int msize));
void channel_route(unsigned int event, unsigned int mode, unsigned int id, unsigned int source);
void channel_open(void);
void channel_close(void);
void channel_error(char *description);
void channel_warning(char *description);
