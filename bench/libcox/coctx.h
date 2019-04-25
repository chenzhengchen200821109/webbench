#ifndef __CO_CTX_H__
#define __CO_CTX_H__

enum registers
{
    eip = 0, // program counter
    esp,     // 8 general-purpose registers 
    ebx, 
    ecx,
    edx,
    edi,
    esi,
    ebp,
    eax
};

typedef void* (*coctx_pfn_t)(void* s1, void* s2);

struct coctx_param_t
{
    const void* thread;
    const void* arg;
};

struct coctx_t
{
    int buffer[9];
    int ss_size;
    char* ss_sp;
};

int coctx_init(coctx_t *ctx);
int coctx_make(coctx_t *ctx, coctx_pfn_t pfn, const void *s, const void *s1);
void coctx_swap(coctx_t *, coctx_t *);

#endif
