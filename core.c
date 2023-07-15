#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

typedef void * (*TASK_ENTRY) (void *p_arg);

typedef struct HADNLE {
	pthread_t ct;
	TASK_ENTRY cb;
	void * data;

} HANDLE;

int task_create(HANDLE *task_obj, TASK_ENTRY handle_cb, void *data);

int patch_handle(int sum, TASK_ENTRY handle_cb, void *data);

void * task_logic(void * data)
{
	while (1) {
		printf("%s %d: %s\n", __func__, __LINE__, (char *)data);
		sleep(5);
	}
}

int main()
{
	int ret = 0;
	HANDLE obj;
	//int ret = patch_handle(100, handle_cb, patch_picture);
	task_create(&obj, task_logic, "test_argc");

	while (1) {
		printf("%s %d\n", __func__, __LINE__);
		sleep(10);
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
