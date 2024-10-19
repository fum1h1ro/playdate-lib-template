#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>

#include <pd_api.h>

PlaydateAPI *api;

int eventHandler(PlaydateAPI *pd, PDSystemEvent event, uint32_t arg)
{
    if (event == kEventInit) {
        api = pd;
    }

    if (event == kEventInitLua) {
    }

    if (event == kEventTerminate) {
    }

    if (event == kEventPause) {
    }

    return 0;
}

