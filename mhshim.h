#define PERR_F 0 /*Set to 1 if call to perror is needed when the allocation functions fail.*/
#define EXIT_F 1 /*Set to 0 if termination of program is required when an error occurs.*/
#define LOG_AC 0 /*Set to 1 to one if logging of actions is needed. Specify logging file in LOGF_N*/
#define LOGF_N NULL /*Name of logging file. Only needed if LOG_AC is 1. Surround with double quotes.*/
#define MAX_ERR_W_LF 500 /*Maximum buffer size for error message in log file*/
/*#define ATEXIT */ /*Uncomment if program should use atexit for displaying stats.*/
#pragma pack(1)
struct __ml_internal_store__ptrs{
    void *ptr;
    size_t size;
    struct __ml_internal_store__ptrs *next;
};
#pragma pack(0)
void *malloc(size_t);
void free(void*);
void *realloc(void *,size_t);
void *calloc(size_t,size_t);
static void general_handler(int);
static void insert_m(struct __ml_internal_store__ptrs**, void*, size_t);
static void delete_vp(struct __ml_internal_store__ptrs**,void *);
static int write_log(char *,char *);
static struct __ml_internal_store__ptrs *find_m(struct __ml_internal_store__ptrs *, void *);
void exit_handlerc();