int picture_process(const char *path);
int session_init(char * path);
extern int set_table(const char * path);

extern FILE * g_fp;
extern char g_buf[1024 * 1024 * 10];

#define FILE_NUM_MAX 2000
extern char file_table[FILE_NUM_MAX][256];
