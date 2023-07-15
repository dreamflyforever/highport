#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <time.h>

#define DEBUG 1

#if DEBUG
#define hp_printf(format, ...) \
	{printf("[%s : %s : %d] ", \
	__FILE__, __func__, __LINE__); \
	printf(format, ##__VA_ARGS__);}
#else
#define hp_printf(format, ...) 
#endif

typedef void * (*TASK_ENTRY) (void *p_arg);
#define BATCH 100
typedef struct HADNLE {
	pthread_t ct;
	TASK_ENTRY cb;
	void * data;

} HANDLE;

int task_create(HANDLE *task_obj, TASK_ENTRY handle_cb, void *data);

int batch_handle(int sum, TASK_ENTRY handle_cb, void *data);
/*get microsecond*/
long get_ms();

void * task_logic(void * data)
{
	int n = *(int *)data;
	while (1) {
		hp_printf("%d\n", n);
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
		hp_printf("working");
		sleep(10);
	}
	return ret;
}

HANDLE patch_obj[BATCH];

/*data maybe the per picture path*/
int batch_handle(int sum, TASK_ENTRY cb, void *data)
{
	int ret = 0, i;
	long start = get_ms();
	for (i = 0; i <= sum; i++) {
		ret = task_create(&patch_obj[i], cb, (void *)&i);

		if (ret != 0) {
			perror("create pthread error\n");
		}
	}
	long end = get_ms();
	hp_printf("create phtread time : %ld ms, start: %ld, end: %ld\n", (end - start), start, end);
	return ret;
}

int task_create(HANDLE *obj, TASK_ENTRY cb, void *data)
{
	int ret;
	//long start = get_ms();
	ret = pthread_create(&(obj->ct), NULL, cb, data);
	//hp_printf("create phtread time : %ld ms\n", (get_ms() - start));
	if (ret != 0) {
		perror("create pthread error\n");
	}
	
	return ret;
}

long get_ms()
{
#if 0
	struct timeval timestamp = {};
	if (0 == gettimeofday(&timestamp, NULL))
	    return timestamp.tv_sec * 1000000 + timestamp.tv_usec;
	else
	    return 0;
#endif
	struct timeb timestamp = {};
	
	if (0 == ftime(&timestamp))
	    return timestamp.time * 1000 + timestamp.millitm;
	else
	    return 0;
}
