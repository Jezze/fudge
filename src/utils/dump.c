#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    call_open(CALL_PO);
    call_open(CALL_PI);

    while ((count = call_read(CALL_PI, FUDGE_BSIZE, buffer)))
    {

        unsigned int i;

        for (i = 0; i < count; i++)
        {

            unsigned char num[32];

            call_write(CALL_PO, ascii_wzerovalue(num, 32, buffer[i], 16, 2, 0), num);
            call_write(CALL_PO, 2, "  ");

        }

    }

    call_close(CALL_PI);
    call_close(CALL_PO);

}

