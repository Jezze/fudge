#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "file.h"
#include "job.h"

unsigned int job_parse(struct job_status *status, struct job *jobs, unsigned int n)
{

    unsigned int njobs = 0;

    memory_clear(jobs, sizeof (struct job) * n);

    while (status->start < status->end)
    {

        struct job *p = &jobs[njobs];

        switch (status->start[0])
        {

        case 'F':
            p->files[p->nfiles] = status->start + 2;
            p->nfiles++;

            break;

        case 'R':
            p->redirects[p->nredirects] = status->start + 2;
            p->nredirects++;

            break;

        case 'D':
            p->inputs[p->ninputs] = status->start + 2;
            p->ninputs++;

            break;

        case 'P':
            p->path = status->start + 2;

            njobs++;

            break;

        case 'E':
            status->start += ascii_length(status->start) + 1;

            return njobs;

        }

        status->start += ascii_length(status->start) + 1;

    }

    return njobs;

}

void job_run(struct channel *channel, struct job *jobs, unsigned int n)
{

    unsigned int i;

    if (!file_walk2(FILE_L0, "/bin"))
        return;

    for (i = 0; i < n; i++)
    {

        struct job *p = &jobs[i];

        if (!(file_walk(FILE_CP, FILE_L0, p->path) || file_walk2(FILE_CP, p->path)))
            continue;

        p->id = call_spawn(FILE_CP);

    }

    for (i = 0; i < n; i++)
    {

        struct job *p = &jobs[i];

        if (i < n - 1)
            channel_sendredirect(channel, p->id, EVENT_DATA, 1, jobs[i + 1].id);
        else
            channel_sendredirect(channel, p->id, EVENT_DATA, 2, 0);

    }

    for (i = 0; i < n; i++)
    {

        struct job *p = &jobs[i];
        unsigned int k;

        for (k = 0; k < p->nfiles; k++)
            channel_place(channel, p->id, EVENT_FILE, ascii_length(p->files[k]) + 1, p->files[k]);

        for (k = 0; k < p->ninputs; k++)
            channel_place(channel, p->id, EVENT_DATA, ascii_length(p->inputs[k]), p->inputs[k]);

    }

    for (i = 0; i < n; i++)
    {

        struct job *p = &jobs[i];

        channel_place(channel, p->id, EVENT_MAIN, 0, 0);

    }

}

