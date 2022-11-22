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
            if(PERR_F==1)[
                printf("Internal allocation not permitted. Virtual offset with respect to entire list and its fields: 0\n"); /*Hardcoded virtual offset because memory for head of list is being allocated.*/
            ]
            if(EXIT_F==1){
                exit(EXIT_FAILURE);
            }
        }
        (**head).ptr=vp;
        (**head).size=i;
        (**head).next=NULL;
        voffsets+=(uint64_t)N_OFF_ADD; /*Not N_OFF_ADD-sizeof(vp) because the void pointer will have taken space after creation of next node.*/
        memory_allocated+=(uint64_t)i;
        if(LOG_AC==1){
            char nbs[MAX_ERR_W_LF+1];
            sprintf((char*)nbs,"ALLOCATE %uB. VIRTUAL OFFSET: %llu\n",i,voffsets);
            write_log(LOGF_N,(char*)nbs);
        }
        return;
    }
    struct ml *selected=*head;
    while(selected->next!=NULL)selected=selected->next;
    selected->next=(struct ml*)get_m(sizeof(struct ml));
    if(selected->next==NULL){
        if(PERR_F==1)[
                printf("Internal allocation not permitted. Virtual offset with respect to entire list and its fields: %llu\n",voffsets); /*Hardcoded virtual offset because memory for head of list is being allocated.*/
        ]
        if(EXIT_F==1){
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