#include <fudge.h>

void main()
{

    if (!call_walk(CALL_DW, CALL_DR, 5, "home/"))
        return;

    if (!call_walk(CALL_DP, CALL_DR, 9, "bin/slang"))
        return;

    if (!call_walk(CALL_I1, CALL_DR, 17, "config/init.slang"))
        return;

    call_spawn(CALL_DP);

}

