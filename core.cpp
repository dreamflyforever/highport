#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <time.h>
#include <pre_process.hpp>
#include <sched.h>
#include <string.h>

typedef void * (*TASK_ENTRY) (void *p_arg);
#define BATCH 200
typedef struct HADNLE {
       pthread_t ct;
       TASK_ENTRY cb;
       void * data;

} HANDLE;

int task_create(HANDLE *task_obj, TASK_ENTRY handle_cb, void *data);
int batch_handle(int sum, TASK_ENTRY handle_cb, void *data);
void core_set(int cpu_core);

//int file_add;
/*get microsecond*/
long get_ms();
long g_start;
long g_end;
int g_flag;
pthread_mutex_t mtx;

void * task_logic(void * data)
{
	pthread_mutex_lock(&mtx);
	int n = *(int *)data;
#if 1
	//core_set(n%3);
	do {
		for (int i = 0; i < 1; i++) {
			long start = get_ms();
			hp_printf("%s\n", file_table[n]);
			picture_process(file_table[n]);
			long end = get_ms();
			hp_printf("per picture process time : %ld ms, start time: %ld ms, end time: %ld ms, id: %d \n",
				(end - start), start, end, n);
		}
	} while (0);
	g_end = get_ms();
#endif
	pthread_mutex_unlock(&mtx);
	return NULL;
}

int main(int argc, char *argv[])
{
	int ret = 0, num_pthread;
	FILE * g_fp = NULL;
	if (argc != 4) {
		hp_printf("usage : ./pmnn modle dir save_file");
		ret = -1;
		goto error;
	}
	if (strcmp("save", argv[3]) == 0) {
		g_flag = 1;
		g_fp = fopen("./result.txt", "w");
		if (g_fp == NULL) {
			perror("fopen error\n");
			ret = -1;
			goto error;
		}
	}

	//HANDLE obj;
        g_start = get_ms();
	pthread_mutex_init(&mtx, NULL);
	num_pthread = set_table(argv[2]);
	//CPU_ZERO(&g_cpuset); 
	session_init(argv[1]);
	ret = batch_handle(num_pthread, task_logic, NULL);
	//task_create(&obj, task_logic, "test_argc");
	hp_printf("process  %d phtread time : %ld ms, start time: %ld ms, end time: %ld ms \n",
		num_pthread, (g_end - g_start), g_start, g_end);
	if (g_flag == 1) {
		fputs(g_buf, g_fp);
		fclose(g_fp);
	}
error:
	return ret;
}

HANDLE patch_obj[BATCH];

int batch_handle(int sum, TASK_ENTRY cb, void *data)
{
	int ret = 0, i;
	long start = get_ms();
	for (i = 0; i < sum; i++) {
		ret = task_create(&patch_obj[i], cb, (void *)&i);

		if (ret != 0) {
			perror("create pthread error\n");
		}
		// 等待线程结束
		if (pthread_join(patch_obj[i].ct, NULL) != 0) {
			fprintf(stderr, "Failed to join thread.\n");
			return 1;
		}
	}
	long end = get_ms();
	hp_printf("create  %d phtread time : %ld ms, start time: %ld ms, end time: %ld ms \n",
			sum, (end - start), start, end);

	return ret;
}

int task_create(HANDLE *obj, TASK_ENTRY cb, void *data)
{
	int ret;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	struct sched_param param;
	param.sched_priority = sched_get_priority_max(SCHED_RR); // 优先级设置,获取最大优先级
	//param.sched_priority = 10;
	pthread_attr_setschedparam(&attr, &param);
	//long start = get_ms();
	int n = *(int *)data;
	hp_printf("data: %d\n", n);
	ret = pthread_create(&(obj->ct), &attr, cb, data);
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

#if 0
void core_set(int cpu_core)
{
	// 创建线程绑定的CPU核心集合
	cpu_set_t cpu_set;
	CPU_ZERO(&cpu_set);
	CPU_SET(cpu_core, &cpu_set);

	// 将当前线程绑定到指定的CPU核心上
	if (pthread_setaffinity_np(pthread_self(),
		sizeof(cpu_set_t), &cpu_set) != 0) {
		fprintf(stderr, "Failed to set thread affinity.\n");
	}

	// 在绑定的CPU核心上执行一些工作
	printf("Thread running on CPU core: %d\n", cpu_core);
}
#endif
