#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "coctx.h"

extern "C" 
{
    int save_context(coctx_t *from) asm("save_context");
};

extern "C"
{
    void restore_context(coctx_t *to) asm("restore_context");
}

int coctx_init(coctx_t* ctx)
{
    memset(ctx, 0, sizeof(*ctx));
    return 0;
}

int coctx_make(coctx_t *ctx, coctx_pfn_t pfn, const void* thread, const void* arg)
{
    char* sp = ctx->ss_sp + ctx->ss_size - sizeof(coctx_param_t);
    sp = (char *)((unsigned int)sp & ~16L);

    coctx_param_t *param = (coctx_param_t *)sp;
    param->thread = thread;
    param->arg = arg;

    memset(ctx->buffer, 0, sizeof(ctx->buffer));
    ctx->buffer[esp] = (int)((char *)sp - sizeof(void *));
    ctx->buffer[eip] = (int)(char *)pfn;

    return 0;
}

void coctx_swap(coctx_t *from, coctx_t *to)
{
    int ret = save_context(from);
    if (ret == 0) {
        restore_context(to);
    } else {
        /* restored from other coroutine, just return and continue */
    }
}
