/*
    fcs_dm.c - Freecell Solver's data management routines.
    
    Written by Shlomi Fish, 2000
    
    This file is distributed under the public domain.
    (It's not copyrighted)
*/

#include "fcs_dm.h"

/*
    SFO_bsearch - an improved binary search function. Highlights:
    
    * The comparison function accepts a common context argument that 
    is passed to SFO_bsearch.
    * If the item was not found the function returns the place in which
    it should be placed, while setting *found to 0. If it was found
      *found is set to 1.
*/
void * SFO_bsearch
(
    void * key, 
    void * void_array, 
    size_t len, 
    size_t width,
    int (* compare)(const void *, const void *, void *),
    void * context,
    int * found
)
{
    int low = 0;
    int high = len-1;
    int mid;
    int result;

    char * array = void_array;

    while (low <= high)
    {
        mid = ((low+high)>>1);

        result = compare(key, (void*)(array+mid*width), context);

        if (result < 0)
        {
            high = mid-1;
        }
        else if (result > 0)
        {
            low = mid+1;
        }
        else
        {
            *found = 1;
            return (void*)(array+mid*width);
        }
    }

    *found = 0;
    return ((void*)(array+(high+1)*width));
}
