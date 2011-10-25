#include <stdlib.h>
#include <pthread.h>
#include <syslog.h>

#include "log.h"
#include "adec.h"
#include "adec_mpeg.h"

static adec_feeder_t *adec_feeder;

static pthread_t thread_id;

static void *dec_thread(void *args)
{
    unsigned pop_data;
    adec_feeder = (adec_feeder_t *)args;

    lp(LOG_INFO, "mpeg audio running thread starts.\n");

    /* bare popping for all data */
    while (1) {
        while (adec_feeder->get_data(&pop_data, 16) == 0){
            /* sleep 5s before next pop */
            sleep(5);

            lp(LOG_INFO, "0x%x", pop_data);
        }

        pthread_testcancel();
    }

    return NULL;
}

int adec_mpeg_init(adec_feeder_t *feeder)
{
    pthread_create(&thread_id, NULL, &dec_thread, feeder);

    return 0;
}

void adec_mpeg_stop(void)
{
    pthread_cancel(thread_id);

    pthread_join(thread_id, NULL);
}

