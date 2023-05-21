unsigned int call_create(unsigned int descriptor, unsigned int pdescriptor, char *name, unsigned int length);
unsigned int call_debug(void);
unsigned int call_destroy(unsigned int descriptor);
unsigned int call_despawn(void);
unsigned int call_kill(unsigned int id);
unsigned int call_link(unsigned int descriptor, unsigned int port);
unsigned int call_list(unsigned int descriptor, unsigned int cdescriptor, unsigned int count, struct record *records);
unsigned int call_load(unsigned int descriptor);
unsigned int call_notify(unsigned int descriptor, unsigned int event, unsigned int count, void *data);
unsigned int call_pick(struct message *message, void *data);
unsigned int call_place(unsigned int id, struct message *message, void *data);
unsigned int call_read(unsigned int descriptor, void *buffer, unsigned int count, unsigned int offset);
unsigned int call_read_all(unsigned int descriptor, void *buffer, unsigned int count, unsigned int offset);
unsigned int call_spawn(unsigned int pdescriptor, unsigned int wdescriptor);
unsigned int call_spawn_absolute(unsigned int pdescriptor, char *path);
unsigned int call_stat(unsigned int descriptor, struct record *record);
unsigned int call_unlink(unsigned int descriptor);
unsigned int call_unload(unsigned int descriptor);
unsigned int call_walk(unsigned int descriptor, unsigned int pdescriptor, char *path, unsigned int length);
unsigned int call_walk_duplicate(unsigned int descriptor, unsigned int pdescriptor);
unsigned int call_walk_relative(unsigned int descriptor, unsigned int pdescriptor, char *path);
unsigned int call_walk_absolute(unsigned int descriptor, char *path);
unsigned int call_write(unsigned int descriptor, void *buffer, unsigned int count, unsigned int offset);
unsigned int call_write_all(unsigned int descriptor, void *buffer, unsigned int count, unsigned int offset);
