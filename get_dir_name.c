#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#define FILE_NUM_MAX 200
char file_table[FILE_NUM_MAX][256];
int main()
{
	DIR *dir;
	struct dirent *ptr;

	dir = opendir("/Users/jim/workspace/highport");
	int i = 0;
	while((ptr = readdir(dir)) != NULL) {
		printf("d_name: %s\n", ptr->d_name);
		memcpy(file_table[i], ptr->d_name, ptr->d_reclen);
		i++;
	}
	for (; i > 0; i--) {
		printf("table: %s\n", file_table[i]);
	}

	closedir(dir);
	return 0;
}
