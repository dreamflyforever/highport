#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define FILE_NUM_MAX 2000
char file_table[FILE_NUM_MAX][256];
int set_table(const char * path)
{
	DIR *dir;
	struct dirent *ptr;

	//dir = opendir("/Users/jim/workspace/highport");
	dir = opendir(path);
	int ret, i = 0;
	while((ptr = readdir(dir)) != NULL) {
		if(strcmp(".",ptr->d_name)!=0 && strcmp("..", ptr->d_name)
			&& strcmp(" ", ptr->d_name)!= 0) {
			snprintf(file_table[i], 256, "%s/%s", path, ptr->d_name);
			printf("d_name: %s\n", file_table[i]);
			i++;
			//memcpy(file_table[i], ptr->d_name, ptr->d_reclen);
		}
	}
	printf("file sum: %d\n", i);
	ret = i;
#if 0
	for (; i > 0; i--) {
		printf("table: %s\n", file_table[i]);
	}
#endif
	closedir(dir);
	return ret;
}
