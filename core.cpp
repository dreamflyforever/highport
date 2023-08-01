#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <time.h>
#include <pre_process.hpp>
#include <sched.h>
#include <string.h>

#define PI_BOARD 0
typedef void * (*TASK_ENTRY) (void *p_arg);
#define BATCH 20000
typedef struct HADNLE {
       pthread_t ct;
       TASK_ENTRY cb;
       void * data;

} HANDLE;

int task_create(HANDLE *task_obj, TASK_ENTRY handle_cb, void *data);
int batch_handle(int sum, TASK_ENTRY handle_cb, void *data);
void core_set(int cpu_core);

int file_add;
/*get microsecond*/
unsigned long get_ms();
unsigned long g_start;
unsigned long g_end;
int g_flag;
char model_path[2048];
pthread_mutex_t mtx[DIVISOR];
int SUM;

void * task_logic(void * data)
{
	int n = SUM / DIVISOR;
	pthread_mutex_t mtx = *(pthread_mutex_t *)data;
	if (data != NULL)
		pthread_mutex_lock(&mtx);
	//int n = *(int *)data;
#if 1
#if PI_BOARD
	core_set(file_add%4);
#endif
	do {
		for (int i = 0; i < n; i++) {
			unsigned long start = get_ms();
			//hp_printf("%s\n", file_table[file_add]);
			//usleep(1);
			//hp_printf("%d %lu\n", gettid(), get_file_size(file_table[file_add]));
			picture_process(file_table[file_add++], i%4);
			unsigned long end = get_ms();
#if 0
			hp_printf("per picture process time : %lu ms, start time: %lu ms, end time: %lu ms, id: %d \n",
				(end - start), start, end, n);
#endif
		}
	} while (0);
	g_end = get_ms();
#endif
	if (data != NULL)
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

        g_start = get_ms();
	//pthread_mutex_init(&mtx, NULL);
	num_pthread = set_table(argv[2]);
	memcpy(model_path, argv[1], strlen(argv[1]));
	SUM = num_pthread;
	ret = batch_handle(num_pthread, task_logic, NULL);
	//task_create(&obj, task_logic, "test_argc");
	hp_printf("process  %d phtread time : %lu ms, start time: %lu ms, end time: %lu ms \n",
		DIVISOR, (g_end - g_start), g_start, g_end);
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
	unsigned long start = get_ms();
	int quotient = sum / DIVISOR;
	int remainder = sum % DIVISOR;
	for (i = 0; i < DIVISOR; i++) {
		session_init(model_path, i);
		pthread_mutex_init(&mtx[i], NULL);
		ret = task_create(&patch_obj[i], cb, (void *)&mtx[i]);

		if (ret != 0) {
			perror("create pthread error\n");
		}
	}
#if 1
	for (i = 0; i < DIVISOR; i++) {
		if (pthread_join(patch_obj[(i)].ct, NULL) != 0) {
			fprintf(stderr, "Failed to join thread.\n");
		}
	}
#endif
	//task_logic((void *)&remainder);
	hp_printf("enter main pthread handle\n");
	session_init(model_path, DIVISOR);
	for (i = 0; i < remainder; i++) {
		picture_process(file_table[file_add++], DIVISOR);
	}
	unsigned long end = get_ms();
	//hp_printf("create  %d phtread time : %ld ms, start time: %ld ms, end time: %ld ms \n",
	//		sum, (end - start), start, end);

	return ret;
}

int task_create(HANDLE *obj, TASK_ENTRY cb, void *data)
{
	int ret;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
#if 1
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	struct sched_param param;
	param.sched_priority = sched_get_priority_max(SCHED_RR); // 优先级设置,获取最大优先级
	//param.sched_priority = 10;
	pthread_attr_setschedparam(&attr, &param);
	//unsigned long start = get_ms();
#endif
	int n = *(int *)data;
	//hp_printf("data: %d\n", n);
	ret = pthread_create(&(obj->ct), &attr, cb, data);
	//ret = pthread_create(&(obj->ct), NULL, cb, data);
	//hp_printf("create phtread time : %ld ms\n", (get_ms() - start));
	if (ret != 0) {
		perror("create pthread error\n");
	}
	return ret;
}

unsigned long get_ms()
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

#if PI_BOARD
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
	//printf("Thread running on CPU core: %d\n", cpu_core);
}
#endif
inline void core_set(int num)
{
	int result;
	cpu_set_t mask;
	CPU_ZERO(&mask); // 将掩码清零
	CPU_SET(num, &mask); // 将num添加到掩码中，该进程绑定到num核心
	result = sched_setaffinity(0, sizeof(mask), &mask);
	if (result < 0) {
		printf("binding CPU fails\n");
	}
}

#endif
