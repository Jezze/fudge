struct vfs_backend
{

    struct resource resource;
    unsigned int id;
    unsigned int (*read)(unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(unsigned int offset, unsigned int count, void *buffer);
    unsigned long (*getphysical)(void);

};

struct vfs_protocol
{

    struct resource resource;
    unsigned int (*match)(struct vfs_backend *backend);
    unsigned int (*root)(struct vfs_backend *backend);
    unsigned int (*parent)(struct vfs_backend *backend, unsigned int id);
    unsigned int (*child)(struct vfs_backend *backend, unsigned int id, unsigned int count, char *path);
    unsigned int (*create)(struct vfs_backend *backend, unsigned int id, unsigned int count, char *name);
    unsigned int (*destroy)(struct vfs_backend *backend, unsigned int id, unsigned int count, char *name);
    unsigned int (*open)(struct vfs_backend *backend, unsigned int id);
    unsigned int (*close)(struct vfs_backend *backend, unsigned int id);
    unsigned int (*read)(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int size, unsigned int count, void *buffer);
    unsigned int (*write)(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int size, unsigned int count, void *buffer);
    unsigned long (*getphysical)(struct vfs_backend *backend, unsigned int id);

};

struct vfs_channel
{

    struct resource resource;
    struct vfs_backend *backend;
    struct vfs_protocol *protocol;

};

struct vfs_mount
{

    struct resource resource;
    struct {struct vfs_channel *channel; unsigned int id;} parent;
    struct {struct vfs_channel *channel; unsigned int id;} child;

};

struct vfs_descriptor
{

    struct resource resource;
    struct vfs_channel *channel;
    unsigned int id;

};

struct vfs_backend *vfs_findbackend(unsigned int id);
struct vfs_protocol *vfs_findprotocol(struct vfs_backend *backend);
void vfs_initbackend(struct vfs_backend *backend, unsigned int id, unsigned int (*read)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(unsigned int offset, unsigned int count, void *buffer), unsigned long (*getphysical)(void));
void vfs_initprotocol(struct vfs_protocol *protocol, unsigned int (*match)(struct vfs_backend *backend), unsigned int (*root)(struct vfs_backend *backend), unsigned int (*parent)(struct vfs_backend *backend, unsigned int id), unsigned int (*child)(struct vfs_backend *backend, unsigned int id, unsigned int count, char *path), unsigned int (*create)(struct vfs_backend *backend, unsigned int id, unsigned int count, char *name), unsigned int (*destroy)(struct vfs_backend *backend, unsigned int id, unsigned int count, char *name), unsigned int (*open)(struct vfs_backend *backend, unsigned int id), unsigned int (*close)(struct vfs_backend *backend, unsigned int id), unsigned int (*read)(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int size, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int size, unsigned int count, void *buffer), unsigned long (*getphysical)(struct vfs_backend *backend, unsigned int id));
void vfs_initchannel(struct vfs_channel *channel);
void vfs_initmount(struct vfs_mount *mount);
void vfs_initdescriptor(struct vfs_descriptor *descriptor);
void vfs_setupcpio(void);
