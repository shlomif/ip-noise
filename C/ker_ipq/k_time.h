#ifndef __IP_NOISE_K_TIME_H
#define __IP_NOISE_K_TIME_H

#include <linux/time.h>

#define gettimeofday(tv, tz) (do_gettimeofday(tv))

#endif /* #ifndef __IP_NOISE_K_TIME_H */
