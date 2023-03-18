#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/times.h>
#include <time.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/time.h>

#define KB              (1024)
#define MB              (KB*KB)

extern void* ls_memcpy(void *dest, const void *src, size_t n);
extern void* kmemcpy(void *dest, const void *src, size_t n);
extern void* rte_memcpy(void *dest, const void *src, size_t n);
void *memcpy(void *dest, const void *src, size_t n);


typedef void* (*memcpyFunc)(void*, const void *, size_t);


struct instr_patch {
	unsigned long instrp;
	int offset;
	unsigned int reserved;
};

struct membench_env {
	void *src;
	void *dest;
	unsigned long src_offset;
	unsigned long dest_offset;
	unsigned long size;
	unsigned long loops;
	memcpyFunc func;
	int funcType;
};

char *getFuncName(int type) {
	if (type == 1) {
		return "rte_memcpy";
	} else if (type == 2) {
		return "modified kernel memcpy";
	} else if (type == 3) {
		return "kernel memcpy";
	} else if (type == 4) {
		return "memcpy";
	} else
		return "memcpy";
}

int check(void *dest, const void *src, size_t n)
{
	int i, ret;

	ret = 0;
	for (i=0; i<n; i++) {
		if (*(char*)(dest + i) != *(char*)(src + i))
			ret = 1;
		*(char*)(dest + i) = 0;
	}
	return ret;
}

void bench(struct membench_env *pEnv) {
	struct timeval start, stop;
	void *src, *dest;
	unsigned long size, loops, i, src_offset, elapsed;
	unsigned long dest_offset;
	memcpyFunc pFunc;
	double speed;
	char *name;

	src = pEnv->src;
	dest = pEnv->dest;
	loops = pEnv->loops;
	src_offset = pEnv->src_offset;
	dest_offset = pEnv->dest_offset;
	size = pEnv->size;
	pFunc = pEnv->func;
	name = getFuncName(pEnv->funcType);
	
        gettimeofday(&start, NULL);
        for (i = 0; i < loops; i++) {
                pFunc(dest + dest_offset,  src + src_offset,  size - src_offset - dest_offset);
#if 0		
		if (check(dest + dest_offset,  src + src_offset,  size - src_offset - dest_offset))
			printf("%s size %d memchk error \n",  name, size);
#endif		
	}
        gettimeofday(&stop, NULL);
        elapsed = (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;
        speed = (double)(((size*loops)/MB)) / elapsed * 1000000;
        printf("%s size %d dest_offset %d src_offset %d speed  %5.3f MB/s\n", name, size, dest_offset, src_offset, speed);
}

void benchFunc(struct membench_env *pEnv) {
	memcpyFunc pFunc;
	int type;

	pFunc = pEnv->func;
	type = pEnv->funcType;

        pEnv->func = rte_memcpy;
        pEnv->funcType = 1;
        bench(pEnv);
        pEnv->func = ls_memcpy;
        pEnv->funcType = 2;
        bench(pEnv);
        pEnv->func = kmemcpy;
        pEnv->funcType = 3;
        bench(pEnv);
        pEnv->func = memcpy;
        pEnv->funcType = 4;
        bench(pEnv);
        printf("\n");

	pEnv->func = pFunc;
	pEnv->funcType = type;
}

void main() {
	void *src, *dest, *src_org, *dest_org;
	unsigned long size, loops, i, offset, elapsed;
	unsigned long align, align_mask;
	struct tms tms;
	struct timeval start, stop;
	double speed;
	struct instr_patch *patch;
	unsigned int *instr, org, new;
	unsigned long pagesize, addr;
	struct membench_env env;

	loops = 100000;
	size = 16 * 1024;
	offset = 0;
	align = 64;
	align_mask = ~(align - 1);

	src_org = src = malloc(size);
	for (i=0; i<size; i++)
		*(char*)(src + i) = i;
	dest_org = dest = malloc(size);

	gettimeofday(&start, NULL);
        memcpy(dest,  src,  size);

	env.loops = loops;
	env.src = src;
	env.dest = dest;
	env.src_offset = 0;
	env.dest_offset = 0;
	env.size = size;
	env.func = memcpy;
	env.funcType = 1;


	env.src_offset = 0;
	benchFunc(&env);
	env.src_offset = 8;
	benchFunc(&env);
        env.src_offset = 6;
        benchFunc(&env);

        env.size = 92;
        env.loops = loops * 16;
        env.src_offset = 0;
        env.dest_offset = 0;
        benchFunc(&env);
        env.dest_offset = 1;
        benchFunc(&env);
        env.dest_offset = 6;
        benchFunc(&env);
        env.dest_offset = 6;
        env.src_offset = 6;
        benchFunc(&env);

        env.size = 160;
        env.loops = loops * 16;
        env.src_offset = 0;
        env.dest_offset = 0;
        benchFunc(&env);
        env.dest_offset = 1;
        benchFunc(&env);
        env.dest_offset = 6;
        benchFunc(&env);
        env.dest_offset = 6;
        env.src_offset = 6;
        benchFunc(&env);


	env.size = 1488;
	env.loops = loops * 16;
	env.src_offset = 0;
	env.dest_offset = 0;
        benchFunc(&env);
	env.dest_offset = 1;
        benchFunc(&env);
	env.dest_offset = 6;
        benchFunc(&env);
	env.dest_offset = 6;
	env.src_offset = 6;
        benchFunc(&env);

        env.size = 4096;
	env.loops = loops * 4;
        env.src_offset = 0;
	env.dest_offset = 0;
        benchFunc(&env);

	env.dest_offset = 1;
        benchFunc(&env);
	env.dest_offset = 6;
        benchFunc(&env);
	env.dest_offset = 6;
	env.src_offset = 6;
        benchFunc(&env);


	free(dest_org);
	free(src_org);
}
