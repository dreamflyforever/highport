#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

typedef void * (*TASK_ENTRY) (void *p_arg);
#define BATCH 100
typedef struct HADNLE {
	pthread_t ct;
	TASK_ENTRY cb;
	void * data;

} HANDLE;

int task_create(HANDLE *task_obj, TASK_ENTRY handle_cb, void *data);

int batch_handle(int sum, TASK_ENTRY handle_cb, void *data);

void * task_logic(void * data)
{
	while (1) {
		printf("%s %d: %d\n", __func__, __LINE__, *(int *)data);
		sleep(10);
	}
}

int main()
{
	int ret = 0;
	//HANDLE obj;
	ret = batch_handle(BATCH, task_logic, "picture_path");
	//task_create(&obj, task_logic, "test_argc");
	while (1) {
		printf("%s %d\n", __func__, __LINE__);
		sleep(10);
	}
	return ret;
}

HANDLE patch_obj[BATCH];

/*data maybe the per picture path*/
int batch_handle(int sum, TASK_ENTRY cb, void *data)
{
	int ret = 0, i;
	for (i = 0; i <= sum; i++) {
		ret = task_create(&patch_obj[i], cb, (void *)&i);

		if (ret != 0) {
			perror("create pthread error\n");
		}
	}
	return ret;
}

int task_create(HANDLE *obj, TASK_ENTRY cb, void *data)
{
	int ret;
	ret = pthread_create(&(obj->ct), NULL, cb, data);
	if (ret != 0) {
		perror("create pthread error\n");
	}
	
	return ret;
}
