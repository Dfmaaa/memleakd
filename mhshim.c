/*Code could've been shorter but didn't shorten to make it less rigid.*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include "mhshim.h"
#define N_OFF_ADD sizeof(void*)+sizeof(size_t)+sizeof(struct __ml_internal_store__ptrs)
static uint64_t memory_allocated=0;
static uint64_t voffsets=0; /*sizeof(vp)+sizeof(i)+sizeof(np)+...*/
static short init=0;
static struct __ml_internal_store__ptrs **mlh=NULL;
static void general_handler(int sig){
    printf("Memory still not deallocated: %llu\nMemory list head pointer: %p\nVirtual offset of tail: %llu\nExit?(Y/N)\n",memory_allocated,*mlh,voffsets);
    if(getchar()=='Y'){
        exit(EXIT_SUCCESS);
    }
}
static void general_handlerv(void){
    printf("Memory still not deallocated: %llu\nMemory list head pointer: %p\nVirtual offset of tail: %llu\nExit?(Y/N)\n",memory_allocated,*mlh,voffsets);
    if(getchar()=='Y'){
        exit(EXIT_SUCCESS);
    }
}
void exit_handlerc(){
    #if defined(ATEXIT)
        atexit(general_handlerv);
    #else 
        signal(SIGINT,general_handler);
        signal(SIGSTOP,general_handler);
        signal(SIGHUP,general_handler);
        signal(SIGQUIT,general_handler);
    #endif
}
void *malloc(size_t size){
    if(init==0){
        exit_handlerc();
        init=1;
    }
    void* (*rmalloc)(size_t)=dlsym(RTLD_NEXT,"malloc");
    void *ret=rmalloc(size);
    insert_m(mlh,ret,size);
    return ret;
}
void *calloc(size_t nmemb, size_t size){
    if(init==0){
        exit_handlerc();
        init=1;
    }
    void* (*rcalloc)(size_t,size_t)=dlsym(RTLD_NEXT,"calloc");
    void *ret=rcalloc(nmemb,size);
    insert_m(mlh,ret,nmemb*size);
    return ret;
}
void free(void *ptr){
    if(init==0){
        exit_handlerc();
        init=1;
    }
    delete_vp(mlh,ptr);
}
void *realloc(void *ptr, size_t size){
    if(init==0){
        exit_handlerc();
        init=1;
    }
    void* (*rrealloc)(void*,size_t)=dlsym(RTLD_NEXT,"realloc");
    struct __ml_internal_store__ptrs *n;
    if((n=find_m(*mlh,ptr))==NULL){
        if(PERR_F){
            printf("Pointer(%p) to use realloc on not found in memory list.\n",ptr);
        }
        if(LOG_AC){
            char emsg[MAX_ERR_W_LF+1];
            sprintf((char*)emsg,"REALLOCATION OF %p FAILED. NOT FOUND IN MEMORY LIST.\n",ptr);
            if(!write_log(LOGF_N,(char*)emsg)){
                if(PERR_F){
                    printf("Can't write to log file(%s).",LOGF_N);
                }
                if(EXIT_F)exit(EXIT_FAILURE);
            }
        }
        if(EXIT_F)exit(EXIT_FAILURE);
        return NULL;
    }
    n->ptr=rrealloc(n->ptr,size);
    memory_allocated+=size-n->size;
    n->size=size;
    char cmsg[MAX_ERR_W_LF+1];
    sprintf((char*)cmsg,"REALLOCATE %p. NEW SIZE: %lu.\n",ptr,size);
    if(!write_log(LOGF_N,(char*)cmsg)){
                if(PERR_F)printf("Can't write to log file(%s).",LOGF_N);
                if(EXIT_F)exit(EXIT_FAILURE);
            }
    return n->ptr;
}
static struct __ml_internal_store__ptrs *find_m(struct __ml_internal_store__ptrs *l, void *ptr){
    struct __ml_internal_store__ptrs *selected=l;
    while(l->ptr!=ptr||l->next==NULL)selected=selected->next;
    if(l->ptr==ptr)return l;
    return NULL;
}
static void insert_m(struct __ml_internal_store__ptrs **head, void *vp, size_t i){
    void* (*get_m)(size_t)=dlsym(RTLD_NEXT,"malloc");
    if(*head==NULL){
        *head=(struct __ml_internal_store__ptrs*)get_m(sizeof(struct __ml_internal_store__ptrs));
        if(*head==NULL){
            if(PERR_F){
                printf("Internal allocation not permitted. Virtual offset with respect to entire list and its fields: 0\n"); /*Hardcoded virtual offset because memory for head of list is being allocated.*/
                return;
            }
            if(EXIT_F)exit(EXIT_FAILURE);
        }
        (**head).ptr=vp;
        (**head).size=i;
        (**head).next=NULL;
        voffsets+=(uint64_t)N_OFF_ADD; /*Not N_OFF_ADD-sizeof(vp) because the void pointer will have taken space after creation of next node.*/
        memory_allocated+=(uint64_t)i;
        if(LOG_AC){
            char nbs[MAX_ERR_W_LF+1];
            sprintf((char*)nbs,"ALLOCATE %luB. VIRTUAL OFFSET: 0\n",i); /*Hardcoded because dealing with head*/
            if(!write_log((char*)LOGF_N,(char*)nbs)){
                if(PERR_F)printf("Can't write to log file(%s).",LOGF_N);
                if(EXIT_F)exit(EXIT_FAILURE);
            }
        }
        return;
    }
    struct __ml_internal_store__ptrs *selected=*head;
    while(selected->next!=NULL)selected=selected->next;
    selected->next=(struct __ml_internal_store__ptrs*)get_m(sizeof(struct __ml_internal_store__ptrs));
    if(selected->next==NULL){
        if(PERR_F){
                printf("Internal allocation not permitted. Virtual offset with respect to entire list and its fields: %llu\n",voffsets);
                return;
        }
        if(EXIT_F){
            exit(EXIT_FAILURE);
        }
    }
        selected->next->ptr=vp;
        selected->next->size=i;
        selected->next->next=NULL;
        voffsets+=(uint64_t)N_OFF_ADD;
        memory_allocated+=(uint64_t)i;
        if(LOG_AC){
            char nbs[MAX_ERR_W_LF+1];
            sprintf((char*)nbs,"ALLOCATE %luB. VIRTUAL OFFSET: %llu\n",i,voffsets-N_OFF_ADD);
            if(!write_log((char*)LOGF_N,(char*)nbs)){
                if(PERR_F){
                    printf("Can't write to log file(%s).",LOGF_N);
                }
            }
        }
}
static void delete_vp(struct __ml_internal_store__ptrs **l, void *v){
    void (*rfree)(void*)=dlsym(RTLD_NEXT,"free");
    if(*l==NULL)return;
    if((*l)->ptr==v){
        struct __ml_internal_store__ptrs *save=*l;
        size_t size=save->size;
        *l=(*l)->next;
        rfree(save->ptr);
        rfree((void*)save);
        voffsets-=(uint64_t)N_OFF_ADD;
        memory_allocated-=(uint64_t)size;
        char wm[MAX_ERR_W_LF+1];
        sprintf((char*)wm,"DEALLOCATE %luB. VIRTUAL OFFSET: %llu\n",size,voffsets);
        if(!write_log((char*)LOGF_N,(char*)wm)){
            if(PERR_F){
                printf("Can't write to log file(%s).\n",LOGF_N);
            }
            if(EXIT_F){
                exit(EXIT_FAILURE);
            }
        }
        return;
    }
    struct __ml_internal_store__ptrs *selected=*l;
    while(selected->next->ptr!=v||selected->next!=NULL)selected=selected->next;
    if(selected->next==NULL){
        if(PERR_F){
            printf("%p not found in memory list(%p)\n",v,l);
        }
        if(LOG_AC){
            char erm[MAX_ERR_W_LF+1];
            sprintf((char*)erm,"LIST ACCESS ERROR. %p NOT FOUND.\n",v);
            if(!write_log((char*)LOGF_N,(char*)erm)){
                if(PERR_F){
                    printf("Can't write to log file(%s).\n",LOGF_N);
                }
            }
        }
        if(EXIT_F){
            exit(EXIT_FAILURE);
        }
        return;
    }
    struct __ml_internal_store__ptrs *s=selected->next;
    selected->next=s->next;
    size_t size=s->size;
    rfree(s->ptr);
    rfree(s);
    voffsets-=(uint64_t)N_OFF_ADD;
    memory_allocated-=(uint64_t)size;
    char wm[MAX_ERR_W_LF+1];
    sprintf((char*)wm,"DEALLOCATE %luB. VIRTUAL OFFSET: %llu\n",size,voffsets);
    if(!write_log((char*)LOGF_N,(char*)wm)){
        if(PERR_F){
            printf("Can't write to log file(%s).\n",LOGF_N);
        }
        if(EXIT_F){
            exit(EXIT_FAILURE);
        }
    }
}
static int write_log(char *lname,char *message){
    int f=open(lname,O_APPEND | O_CREAT);
    if(f==-1){
        return -1;
    }
    if(write(f,message,(size_t)MAX_ERR_W_LF)<0){
        close(f);
        return -1;
    }
    close(f);
    return 1;
}