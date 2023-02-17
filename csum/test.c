#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>

extern int do_csum_interleave(char *buf, int size);
extern int do_csum_128(char *buf, int size);
extern int do_csum_32(char *buf, int size);
extern int do_csum_64(char *buf, int size);
extern int csum_partial(char *buf, int size, unsigned int wsum);

static inline unsigned short csum_fold(unsigned int csum)
{
        unsigned int sum = (unsigned int)csum;
        sum += (sum >> 16) | (sum << 16);
        return ~(unsigned short)(sum >> 16);
}

unsigned int csum_partial_128( void *buf, int len, unsigned int wsum)
{
        unsigned int sum = ( unsigned int)wsum;
        unsigned int result = do_csum_128(buf, len);

        /* add in old sum, and carry.. */
        result += sum;
        if (sum > result)
                result += 1;
        return ( unsigned int)result;
}

unsigned int csum_partial_64( void *buf, int len, unsigned int wsum)
{
        unsigned int sum = ( unsigned int)wsum;
        unsigned int result = do_csum_64(buf, len);

        /* add in old sum, and carry.. */
        result += sum;
        if (sum > result)
                result += 1;
        return ( unsigned int)result;
}

unsigned short ip_compute_csum_128( void *buf, int len)
{
        return (unsigned short)~do_csum_128(buf, len);
}


unsigned short ip_compute_csum_64( void *buf, int len)
{
        return (unsigned short)~do_csum_64(buf, len);
}

unsigned short ip_compute_csum( void *buf, int len)
{
        return csum_fold(csum_partial(buf, len, 0));
}

unsigned short ip_compute_csum_interleave( void *buf, int len)
{
        return (unsigned short)~do_csum_interleave(buf, len);
}

void bench(char *buf, int size)
{
	struct timeval tv1, tv2;
	unsigned long elapse1, elapse2, elapse3;
	int i, loops;

	loops = 0x100000;
	gettimeofday(&tv1, 0);
	for (i=0; i<loops; i++) {
		ip_compute_csum_128(buf, size);
	}
	gettimeofday(&tv2, NULL);
	elapse1 = (((tv2.tv_usec-tv1.tv_usec)+((tv2.tv_sec-tv1.tv_sec)*1000000)));

	gettimeofday(&tv1, NULL);
	for (i=0; i<loops; i++) {
		ip_compute_csum(buf, size);
	}
	gettimeofday(&tv2, NULL);
	elapse2 = (((tv2.tv_usec-tv1.tv_usec)+((tv2.tv_sec-tv1.tv_sec)*1000000)));

	gettimeofday(&tv1, NULL);
	for (i=0; i<loops; i++) {
		ip_compute_csum_interleave(buf, size);
	}
	gettimeofday(&tv2, NULL);
	elapse3 = (((tv2.tv_usec-tv1.tv_usec)+((tv2.tv_sec-tv1.tv_sec)*1000000)));
	printf("buf size[%4d] loops[0x%x] times[us]: csum uint128 %ld asm method %ld interleave %ld \n", size, loops, elapse1, elapse2, elapse3);
}

void checkFunction(char *buf, int size)
{
    int i, len;
    int val1, val2, val3;

    for (i=0; i<16; i++) {
        len = 400 + i * 8;
        val1 = ip_compute_csum_128(buf +i, len);
        val2 = ip_compute_csum(buf + i, len);
        val3 = ip_compute_csum_interleave(buf+i, len);
        if ((val1 != val2) || (val2 != val3))
           printf("val1 %x val2 %x val3 %x\n", val1, val2, val3);
   }
}


void main(){
   char *buf;
   static struct timeval tv1, tv2;
   int i, size;

   size = 4096;
   buf = malloc(size);
   for (i=0; i<size; i++) {
	   buf[i] = (char)~i;
   }

   gettimeofday(&tv1, 0);
   checkFunction(buf, size);

   bench(buf, size);
   bench(buf, 1472);
   bench(buf, 250);
   bench(buf, 40);
}
