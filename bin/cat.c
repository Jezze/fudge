#include <call.h>
#include <file.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
    {

        file_write_string2(FILE_STDOUT, "You need to supply filename.\n");

        return;

    }

    int fd = call_open2(argv[1]);

    if (fd == -1)
    {

        file_write_string2(FILE_STDOUT, "File does not exist.\n");

        return;

    }

    char buffer[500];

    unsigned int count = call_read(fd, buffer, 500);

    call_write(FILE_STDOUT, buffer, count);

    call_close(fd);

}

