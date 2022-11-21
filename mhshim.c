#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <signal.h>
#include "mhshim.h"
static int memory_allocated=0; //Can be removed in future commits.
void *malloc(size_t size){
    void (*real_malloc)(size_t)=dlsym(RTLD_NEXT,"malloc");
}
