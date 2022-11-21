#define PERROR_F 0 /*Set to 1 if call to perror is needed when the allocation functions fail.*/
#define LOG_AC 0 /*Set to 1 to one if logging of actions is needed. Specify logging file in LOGF_N*/
#define LOGF_N NULL /*Name of logging file. Only needed if LOG_AC is 1.*/
static typedef struct ml{
    void *ptr;
    size_t size;
}__attribute__((packed));
void *malloc(size_t);
void free(void*);
void realloc(void *,size_t);
void calloc(size_t,size_t);
static void general_handler();
static void insert_m(struct ml**, void*, size_t);
static void delete_vp(struct ml*,void *);