#include <call.h>
#include <file.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
    {

        file_write("You need to supply filename.\n");

        return;

    }

    struct vfs_node *node = call_open(argv[1]);

    if (!node)
    {

        file_write("File does not exist.\n");

        return;

    }

    char buffer[5000];

    unsigned int size = vfs_read(node, 0, 5000, buffer);

    struct vfs_node *out = call_open("dev/tty");

    vfs_write(out, 0, size, buffer);

}

