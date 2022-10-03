#include <stdio.h>
#include <iostream>
#include <malloc.h>
#include <unwind.h>
#include <cstdlib>

#include "leakInspector.h"

using namespace std;

/* Prototypes for our hooks.  */
static void my_init_hook(void);
static void *my_malloc_hook(size_t, const void *);
static void my_free_hook(void *, const void *);


/* Variables to save original hooks. */
static void *(*old_malloc_hook) (size_t, const void *);
static void (*old_free_hook) (void *__ptr, const void *);

/* Override initializing hook from the C library. */
void (*volatile __malloc_initialize_hook) (void) = my_init_hook;

static void my_init_hook(void)
{
    old_malloc_hook = __malloc_hook;
    old_free_hook = __free_hook;

    __malloc_hook = my_malloc_hook;
    __free_hook = my_free_hook;

    atexit(dumpReport);
}

static void *my_malloc_hook(size_t size, const void *caller)
{
    void *addr;

    /* Restore all old hooks */
    __malloc_hook = old_malloc_hook;
    
    // Note: first allocation i.e. 72704 bytes will be done by the C++ compiler for STL purpose. So we might see that as well. So, ignore it.
    struct TraceInfo *traceInfo;

    traceInfo = getBackTraceInfo();

    // Allocate the memory requested by user
    addr = malloc(size);
    printf("allocating memory\n");
    insertNode(addr, size, traceInfo);

    /* Save underlying hooks */
    old_malloc_hook = __malloc_hook;

    /* Restore our own hooks */
    __malloc_hook = my_malloc_hook;
        //printf("4: __malloc_hook = %x  old_malloc_hook = %x\n", __malloc_hook, old_malloc_hook);

    return addr;
}

static void my_free_hook(void *addr, const void *caller)
{
    /* Restore all old hooks */
    __free_hook = old_free_hook;
    
    deleteNode(addr);
    free(addr);

    /* Save underlying hooks */
    old_free_hook = __free_hook;

    /* Restore our own hooks */
    __free_hook = my_free_hook;
}

void b(){
    int *p = new int[10];
}

void c(){
    b();
    int *p = (int*)malloc(100);
} 

void d(){
    c();
}

int main(void)
{
    d();
    return 0;
}