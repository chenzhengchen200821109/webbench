#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#include "co_routine_inner.h"
#include <stdlib.h>

class CoRoutine
{
    public:                     
        CoRoutine()    
        {              
            coroutine = (struct stCoRoutine_t *)calloc(1, sizeof(struct stCoRoutine_t));
        }              
        ~CoRoutine()   
        {              
            free(coroutine);
        }              
        struct stCoRoutine_t *coroutine;
};

#endif
