#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <time.h>
#include <pre_process.hpp>
#include <sched.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <vector>


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
	int pt = *(int *)data;
	if (data != NULL)
		pthread_mutex_lock(&mtx[pt]);
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
			picture_process(file_table[file_add++], pt);
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
		pthread_mutex_unlock(&mtx[pt]);
	return NULL;
}

#if 0
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

#endif

// 解密函数
void decryptModel(const std::string& inputFile, const std::string& outputFile, const std::string& password) {
	std::ifstream inFile(inputFile, std::ios::binary);
	std::ofstream outFile(outputFile, std::ios::binary);

	char ch;
	size_t i = 0;
	while (inFile.get(ch)) {
		// 使用密码进行异或解密
		ch ^= password[i % password.length()];
		outFile.put(ch);
		i++;
	}

	inFile.close();
	outFile.close();

	std::cout << "模型已成功解密为：" << outputFile << std::endl;
}

int MD5(const char * filepath, uint8_t *result);
int main(int argc, char *argv[])
{
	int ret = 0, num_pthread;
	FILE * g_fp = NULL;
	int file_flag = 0;

	//std::string modelFile = argv[1];
#if 1
	std::string encryptedModelFile = argv[1];
	std::string decryptedModelFile = "libdeos.so";

	std::string password = "myPassword123";  // 设置加密和解密的密码
#endif
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
#if 1
	uint8_t result[16];
	if (MD5(argv[1], result) == 1) {
		if (result[1] == 218) {
			//hp_printf("right modle\n");
			decryptModel(encryptedModelFile, decryptedModelFile, password);
			memcpy(model_path, "libdeos.so", 16);
			file_flag = 1;
		} else {
			//hp_printf("wrong modle\n");
			memcpy(model_path, argv[1], strlen(argv[1]));
		}
	} else {
			//hp_printf("wrong modle\n");
			memcpy(model_path, argv[1], strlen(argv[1]));
	}
	//hp_printf("%s\n", model_path);
#endif
        g_start = get_ms();
	pthread_mutex_init(&buf_mtx, NULL);
	//pthread_mutex_init(&mtx, NULL);
	num_pthread = set_table(argv[2]);
	SUM = num_pthread;
	ret = batch_handle(num_pthread, task_logic, NULL);
	//task_create(&obj, task_logic, "test_argc");
	hp_printf("process  %d phtread time : %lu ms, start time: %lu ms, end time: %lu ms \n",
		DIVISOR, (g_end - g_start), g_start, g_end);
	if (g_flag == 1) {
		fputs(g_buf, g_fp);
		fclose(g_fp);
	}
	if (file_flag == 1) {
		remove("libdeos.so");
		//hp_printf("remove file\n");
	}
error:
	return ret;
}

HANDLE patch_obj[BATCH];

int batch_handle(int sum, TASK_ENTRY cb, void *data)
{
	int ret = 0, i;

	pthread_mutex_t l_mtx;
	pthread_mutex_init(&l_mtx, NULL);
	unsigned long start = get_ms();
	int quotient = sum / DIVISOR;
	int remainder = sum % DIVISOR;
	for (i = 0; i < DIVISOR; i++) {
		session_init(model_path, i);
		pthread_mutex_init(&mtx[i], NULL);

		pthread_mutex_lock(&l_mtx);
		ret = task_create(&patch_obj[i], cb, (void *)&i);
		pthread_mutex_unlock(&l_mtx);
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
#if 1
	//task_logic((void *)&remainder);
	session_init(model_path, DIVISOR);
	for (i = 0; i < remainder; i++) {
		picture_process(file_table[file_add++], DIVISOR);
	}
#endif
	//unsigned long end = get_ms();
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
	usleep(20000);
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

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <vector>

// MD5压缩函数4轮循环中使用的生成函数，每轮不同
#define F(b, c, d) (((b) & (c)) | ((~b) & (d)))
#define G(b, c, d) (((b) & (d)) | ((c) & (~d)))
#define H(b, c, d) ((b) ^ (c) ^ (d))
#define I(b, c, d) ((c) ^ ((b) | (~d)))

// 循环左移
#define LEFTROTATE(num, n) (((num) << n) | ((num >> (32 - n))))

// T表，32位字，一共有64个元素，对应64次迭代，也成为加法常数
const uint32_t T[64] = { 0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
						0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
						0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
						0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
						0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
						0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
						0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
						0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
						0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
						0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
						0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
						0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
						0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
						0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
						0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
						0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 };

// 64次迭代运算采用的左循环移位的s值
const uint32_t S[64] = { 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
						 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
						 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
						 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21 };

// 两个工具函数
void int2byte(uint32_t val, uint8_t *bytes)
{
	bytes[0] = (uint8_t)val;
	bytes[1] = (uint8_t)(val >> 8);
	bytes[2] = (uint8_t)(val >> 16);
	bytes[3] = (uint8_t)(val >> 24);
}

uint32_t byte2int(const uint8_t *bytes)
{
	return (uint32_t)bytes[0]
		| ((uint32_t)bytes[1] << 8)
		| ((uint32_t)bytes[2] << 16)
		| ((uint32_t)bytes[3] << 24);
}

// MD5主函数
int MD5(const char * filepath, uint8_t *result) {
	FILE *fp = NULL;
	uint8_t buffer[64];
	uint8_t* temp = NULL;
	size_t count = 0, offset, i; // count用于记录总长度，补位的时候需要用到
	uint32_t X[16];
	int flag = 0;

	if ((fp = fopen(filepath, "rb+")) == NULL) {
		printf("[ERROR] File in %s not found.", filepath);
		return 0;
	}

	// MD缓冲区CV，迭代在缓冲区进行
	uint32_t A, B, C, D;
	// 初始向量IV，采用小端存储（Intel x86系列原本就采用了Little Endian方式存储）
	A = 0x67452301;
	B = 0xEFCDAB89;
	C = 0x98BADCFE;
	D = 0X10325476;

	int iter = 1;
	//while (!feof(fp)) {
	while (iter--) {
		memset(buffer, 0, sizeof(buffer));
		// fread函数返回读取的次数，设定每次读取一个字符，就可以知道字符长度了
		int len = fread(buffer, 1, 64, fp);
		// 更新文件总长度
		count += len;
		// 当读取文件到末尾时，意味着需要进行补位操作了，此时读到的len可能不足512bit，也可能刚好等于512bit
		if (feof(fp)) {
			flag = 1;

			// 因为恰好等于448bit不行，所以new_len直接等于len+1
			int new_len;
			for (new_len = len + 1; new_len % 64 != 56; new_len++)
				;

			// 还要增加64bit
			temp = (uint8_t*)malloc(new_len + 8);
			memcpy(temp, buffer, len);

			// 填充1000...0
			temp[len] = 0x80;
			for (offset = len + 1; offset < new_len; offset++)
				temp[offset] = 0;

			// 在末尾再附加总长度count的低64位，由于这里的count单位是byte，所以要乘以8
			int2byte(count * 8, temp + new_len);
			int2byte(count >> 29, temp + new_len + 4); //参考了其他代码，count>>29相当于count*8>>32，但可以避免值溢出
			len = new_len;
		}

		// 虽然每次只读取512bit，但是还是采用这样的方式，可以防止最后的一次由于补位导致可能出现的 len > 512bit 的情况（此时就要分两次了）
		for (offset = 0; offset < len; offset += 64) {
			// 读到结尾时，我们把补位后的数据存在了temp中，为了处理的统一，将temp中的数据保存到buffer上
			if (flag == 1) {
				memcpy(buffer, temp + offset, 64);
			}

			// 保存512位的每32位分组，在X[k]时会用到
			for (int i = 0; i < 16; i++) {
				X[i] = byte2int(buffer + i * 4);
			}

			uint32_t a, b, c, d, temp, g, k;
			a = A;
			b = B;
			c = C;
			d = D;

			// 主循环，共四轮，每轮16次迭代，共64次迭代
			for (i = 0; i < 64; i++) {
				if (i < 16) {
					g = F(b, c, d);
					k = i;
				}
				else if (i < 32) {
					g = G(b, c, d);
					k = (1 + 5 * i) % 16;
				}
				else if (i < 48) {
					g = H(b, c, d);
					k = (5 + 3 * i) % 16;
				}
				else {
					g = I(b, c, d);
					k = (7 * i) % 16;
				}
				temp = d;
				d = c;
				c = b;
				b = b + LEFTROTATE((a + g + X[k] + T[i]), S[i]);
				a = temp;
			}

			A += a;
			B += b;
			C += c;
			D += d;

		}
	}
	
	// 文件读取结束，释放内存
	free(temp);

	// 把128位的最终结果转化为字节形式
	int2byte(A, result);
	int2byte(B, result + 4);
	int2byte(C, result + 8);
	int2byte(D, result + 12);

	return 1;
}
