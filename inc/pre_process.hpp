int picture_process(const char *path, int which);
int session_init(char * path, int which);
extern int set_table(const char * path);

extern int g_flag;
extern char g_buf[1024 * 1024 * 10];

#define FILE_NUM_MAX 20000
#define path_size 512
extern char file_table[FILE_NUM_MAX][path_size];

#define DEBUG 1

#if DEBUG
#define hp_printf(format, ...) \
       {printf("[%s : %s : %d] ", \
       __FILE__, __func__, __LINE__); \
       printf(format, ##__VA_ARGS__);}
#else
#define hp_printf(format, ...)
#endif

extern unsigned long get_file_size(const char *path);

#define DIVISOR 4
extern pthread_mutex_t buf_mtx;
