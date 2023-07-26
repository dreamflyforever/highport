#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define FILE_NUM_MAX 200
char file_table[FILE_NUM_MAX][256];
int set_table()
{
	DIR *dir;
	struct dirent *ptr;

	dir = opendir("/Users/jim/workspace/highport");
	int i = 0;
	while((ptr = readdir(dir)) != NULL) {
		if(strcmp(".",ptr->d_name)!=0 && strcmp("..", ptr->d_name)
			&& strcmp(" ", ptr->d_name)!= 0) {
			printf("d_name: %s\n", ptr->d_name);
			memcpy(file_table[i], ptr->d_name, ptr->d_reclen);
			i++;
		}
	}
#if 0
	printf("i: %d\n", i);
	for (; i > 0; i--) {
		printf("table: %s\n", file_table[i]);
	}
#endif
	closedir(dir);
	return 0;
}
