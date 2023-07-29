#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "pre_process.hpp"

char file_table[FILE_NUM_MAX][path_size];
#if 0
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
	//printf("file sum: %d\n", i);
	ret = i;
#if 0
	for (; i > 0; i--) {
		printf("table: %s\n", file_table[i]);
	}
#endif
	closedir(dir);
	return ret;
}

#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
 
// main 函数的 argv[1] char * 作为 所需要遍历的路径 传参数给 listDir
int set_table(const char *path)
{
	int index = 0;
	//定义一个 DIR 类的指针
	DIR *pDir;
	//定义一个结构体 dirent 的指针，dirent 结构体见上
	struct dirent *ent;
	int i = 0;
	//定义一个字符数组，用来存放读取的路径
	char childpath[512];

	pDir = opendir(path); //  opendir 方法打开 path 目录，并将地址付给 pDir 指针
	memset(childpath, 0, sizeof(childpath)); //将字符数组 childpath 的数组元素全部置零
						 //读取 pDir 打开的目录，并赋值给 ent, 同时判断是否目录为空，不为空则执行循环体
	while ((ent = readdir(pDir)) != NULL) {
		//读取 打开目录的文件类型 并与 DT_DIR 进行位与运算操作，即如果读取的 d_type 类型为 DT_DIR (=4 表示读取的为目录)
		if (ent->d_type & DT_DIR) {
			if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
				//如果读取的 d_name 为 . 或者.. 表示读取的是当前目录符和上一目录符, 用 contiue 跳过，不进行下面的输出
				continue;
			}
			//如果非. ..则将 路径 和 文件名 d_name 付给 childpath, 并在下一行 prinf 输出
			snprintf(childpath, 512, "%s/%s", path, ent->d_name);
			printf("path:%s\n", childpath);
			//递归读取下层的字目录内容， 因为是递归，所以从外往里逐次输出所有目录（路径+目录名），
			//然后才在 else 中由内往外逐次输出所有文件名
			set_table(childpath);
		}
		//如果读取的 d_type 类型不是 DT_DIR, 即读取的不是目录，而是文件，则直接输出 d_name, 即输出文件名
		else {
			//printf("%s\n", ent->d_name);
			snprintf(file_table[index], path_size, "%s/%s", path, ent->d_name);
			printf("d_name: %s\n", file_table[index]);
			index++;
		}
	}
	return index;
}

#if 0
int main(int argc, char *argv[])
{
    set_table(argv[1]); //第一个参数为 想要遍历的 linux 目录 例如，当前目录为 ./ ,上一层目录为../
    return 0;
}
#endif
