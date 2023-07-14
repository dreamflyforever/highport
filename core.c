#include <pthread.h>
#include <stdio.h>

typedef void (*TASK_ENTRY) (void *p_arg);

typedef struct HADNLE {
	pthread_t * context;
	TASK_ENTRY cb;
	void * data;

} HANDLE;

int task_cleate(HANDLE *task_obj, TASK_ENTRY handle_cb, void *data);

int patch_handle(int sum, TASK_ENTRY handle_cb, void *data);

int main()
{
	int ret;
	//int ret = patch_handle(100, handle_cb, patch_picture);
	return ret;
}

int task_cleate(HANDLE *task_obj, TASK_ENTRY handle_cb, void *data)
{
	int ret;
	ret = pthread_create(task_obj->context, NULL, handle_cb, data);

	return ret;
}

