#define PERR_F 0 /*Set to 1 if call to perror is needed when the allocation functions fail.*/
#define EXIT_F 1 /*Set to 0 if termination of program is required when an error occurs.*/
#define LOG_AC 0 /*Set to 1 to one if logging of actions is needed. Specify logging file in LOGF_N*/
#define LOGF_N NULL /*Name of logging file. Only needed if LOG_AC is 1. Surround with double quotes.*/
#define MAX_ERR_W_LF 500 /*Maximum buffer size for error message in log file*/
#pragma pack(1)
static struct ml{
    void *ptr;
    size_t size;
    struct ml *next;
};
#pragma pack(0)
void *malloc(size_t);
void free(void*);
void realloc(void *,size_t);
void calloc(size_t,size_t);
static void general_handler();
static void insert_m(struct ml**, void*, size_t);
static void delete_vp(struct ml**,void *);
static int write_log(char *,char *);