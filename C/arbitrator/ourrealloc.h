#ifndef __IP_NOISE_OURREALLOC_H
#define __IP_NOISE_OURREALLOC_H

#define ourrealloc(orig, orig_size, new_size) (realloc((orig), (new_size)))
#endif
