#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include "mhshim.h"
#define N_OFF_ADD sizeof(void*)+sizeof(size_t)+sizeof(struct ml)
static uint64_t memory_allocated=0;
static uint64_t voffsets=0; /*sizeof(vp)+sizeof(i)+sizeof(np)+...*/
static void *malloc(size_t size){
    void* (*real_malloc)(size_t)=dlsym(RTLD_NEXT,"malloc");
}
static void insert_m(struct ml **head, void *vp, size_t i){
    void* (*get_m)(size_t)=dlsym(RTLD_NEXT,"malloc");
    if(*head==NULL){
        *head=(struct ml*)get_m(sizeof(struct ml));
        if(*head==NULL){
            if(PERR_F)[
                printf("Internal allocation not permitted. Virtual offset with respect to entire list and its fields: 0\n"); /*Hardcoded virtual offset because memory for head of list is being allocated.*/
                return;
            ]
            if(EXIT_F){
                exit(EXIT_FAILURE);
            }
        }
        (**head).ptr=vp;
        (**head).size=i;
        (**head).next=NULL;
        voffsets+=(uint64_t)N_OFF_ADD; /*Not N_OFF_ADD-sizeof(vp) because the void pointer will have taken space after creation of next node.*/
        memory_allocated+=(uint64_t)i;
        if(LOG_AC){
            char nbs[MAX_ERR_W_LF+1];
            sprintf((char*)nbs,"ALLOCATE %uB. VIRTUAL OFFSET: 0\n",i); /*Hardcoded because dealing with head*/
            if(!write_log((char*)LOGF_N,(char*)nbs)){
                if(PERR_F){
                    printf("Can't write to log file(%s).",LOGF_N);
                }
            }
        }
        return;
    }
    struct ml *selected=*head;
    while(selected->next!=NULL)selected=selected->next;
    selected->next=(struct ml*)get_m(sizeof(struct ml));
    if(selected->next==NULL){
        if(PERR_F)[
                printf("Internal allocation not permitted. Virtual offset with respect to entire list and its fields: %llu\n",voffsets);
                return;
        ]
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
            sprintf((char*)nbs,"ALLOCATE %uB. VIRTUAL OFFSET: %llu\n",i,voffsets-N_OFF_ADD);
            if(!write_log((char*)LOGF_N,(char*)nbs)){
                if(PERR_F){
                    printf("Can't write to log file(%s).",LOGF_N);
                }
            }
        }
}
static void delete_vp(struct ml **l, void *v){
    void (*rfree)(void*)=dlsym(RTLD_NEXT,"free");
    if(*l==NULL)return;
    if(*l->ptr=v){
        struct ml *save=*l;
        size_t size=save->size;
        *l=*l->next;
        rfree(save->ptr);
        rfree((void*)save);
        voffsets-=(uint64_t)N_OFF_ADD;
        memory_allocated-=(uint64_t)size;
        char wm[MAX_ERR_W_LF+1];
        sprintf((char*)wm,"DEALLOCATE %uB. VIRTUAL OFFSET: %llu\n",size,voffsets);
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
    struct ml *selected=*l;
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
