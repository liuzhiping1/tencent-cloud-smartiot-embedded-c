#include "tc_iot_inc.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

long tc_iot_hal_timestamp(void* zone) {
    return time(NULL);
}

int tc_iot_hal_sleep_ms(long sleep_ms) { return usleep(sleep_ms * 1000); }

long tc_iot_hal_random() {
    return random();
}

void tc_iot_hal_srandom(unsigned int seed) { return srand(seed); }

