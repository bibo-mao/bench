#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#if 1
/* Type to use for aligned memory operations.
   This should normally be the biggest type supported by a single load
   and store.  */
#define	op_t	unsigned long
#define OPSIZ	(sizeof(op_t))

/* Threshold value for when to enter the unrolled loops.  */
#define	OP_T_THRES	16

static inline void memcpy_lt16(unsigned long dst, unsigned long src, size_t n)
{
	if (n & 0x08) {
		/* copy 8 ~ 15 bytes */
		*(uint64_t *)dst = *(const uint64_t *)src;
		*(uint64_t *)(dst - 8 + n) = *(const uint64_t *)(src - 8 + n);
	} else if (n & 0x04) {
		/* copy 4 ~ 7 bytes */
		*(uint32_t *)dst = *(const uint32_t *)src;
		*(uint32_t *)(dst - 4 + n) = *(const uint32_t *)(src - 4 + n);
	} else if (n & 0x02) {
		/* copy 2 ~ 3 bytes */
		*(uint16_t *)dst = *(const uint16_t *)src;
		*(uint16_t *)(dst - 2 + n) = *(const uint16_t *)(src - 2 + n);
	} else if (n & 0x01) {
		/* copy 1 byte */
		*(unsigned char*)dst = *(unsigned char*)src;
	}
}

void *rte_memcpy(void *dstpp, const void *srcpp, size_t len)
{
	register op_t a0, a1, a2, a3;
	unsigned long dstp = (unsigned long) dstpp;
	unsigned long srcp = (unsigned long) srcpp;
	int nbytes;

	/* If there not too few bytes to copy, use word copy.  */
	if (len >= OP_T_THRES) {
		/* Copy just a few bytes to make DSTP aligned.  */
		nbytes = (-dstp) % OPSIZ;
		if (nbytes) {
			a0 = ((op_t *) srcp)[0];
			len -= nbytes;
			srcp += nbytes;
                        ((op_t *) dstp)[0] = a0;
			dstp += nbytes;
		}

		while (len >=64) {
			a0 = ((op_t *) srcp)[0];
			a1 = ((op_t *) srcp)[1];
			a2 = ((op_t *) srcp)[2];
			a3 = ((op_t *) srcp)[3];
                        ((op_t *) dstp)[0] = a0;
                        ((op_t *) dstp)[1] = a1;
                        ((op_t *) dstp)[2] = a2;
                        ((op_t *) dstp)[3] = a3;
			len -= 64;
			__asm__ __volatile__("": :"r"(len));
			a0 = ((op_t *) srcp)[4];
			a1 = ((op_t *) srcp)[5];
			a2 = ((op_t *) srcp)[6];
			a3 = ((op_t *) srcp)[7];
			((op_t *) dstp)[4] = a0;
			((op_t *) dstp)[5] = a1;
			((op_t *) dstp)[6] = a2;
			((op_t *) dstp)[7] = a3;
			srcp += 8 * OPSIZ;
			dstp += 8 * OPSIZ;
		}
		if (len >= 48) {
                        a0 = ((op_t *) srcp)[0];
                        a1 = ((op_t *) srcp)[1];
                        a2 = ((op_t *) srcp)[2];
                        a3 = ((op_t *) srcp)[3];
                        ((op_t *) dstp)[0] = a0;
                        ((op_t *) dstp)[1] = a1;
                        ((op_t *) dstp)[2] = a2;
                        ((op_t *) dstp)[3] = a3;
			a0 = ((op_t *) srcp)[4];
			a1 = ((op_t *) srcp)[5];
			((op_t *) dstp)[4] = a0;
			((op_t *) dstp)[5] = a1;
			memcpy_lt16(dstp + 48, srcp + 48, len - 48);
			return dstpp;
		} else if (len >= 32) {
                        a0 = ((op_t *) srcp)[0];
                        a1 = ((op_t *) srcp)[1];
                        a2 = ((op_t *) srcp)[2];
                        a3 = ((op_t *) srcp)[3];
                        ((op_t *) dstp)[0] = a0;
                        ((op_t *) dstp)[1] = a1;
                        ((op_t *) dstp)[2] = a2;
                        ((op_t *) dstp)[3] = a3;
			memcpy_lt16(dstp + 32, srcp + 32, len - 32);
                        return dstpp;
		} else if (len >= 16) {
                        a0 = ((op_t *) srcp)[0];
                        a1 = ((op_t *) srcp)[1];
                        ((op_t *) dstp)[0] = a0;
                        ((op_t *) dstp)[1] = a1;
			memcpy_lt16(dstp + 16, srcp + 16, len - 16);
                        return dstpp;
		}
	}

	memcpy_lt16(dstp, srcp, len);
	return dstpp;
}

#endif

#if 0
static inline
void rte_mov16(unsigned char *dst, const unsigned char *src)
{
	__uint128_t *dst128 = (__uint128_t *)dst;
	const __uint128_t *src128 = (const __uint128_t *)src;
	*dst128 = *src128;
}

static inline
void rte_mov32(unsigned char *dst, const unsigned char *src)
{
	__uint128_t *dst128 = (__uint128_t *)dst;
	const __uint128_t *src128 = (const __uint128_t *)src;
	const __uint128_t x0 = src128[0], x1 = src128[1];
	dst128[0] = x0;
	dst128[1] = x1;
}

static inline
void rte_mov48(unsigned char *dst, const unsigned char *src)
{
	__uint128_t *dst128 = (__uint128_t *)dst;
	const __uint128_t *src128 = (const __uint128_t *)src;
	const __uint128_t x0 = src128[0], x1 = src128[1], x2 = src128[2];
	dst128[0] = x0;
	dst128[1] = x1;
	dst128[2] = x2;
}

static inline
void rte_mov64(unsigned char *dst, const unsigned char *src)
{
	__uint128_t *dst128 = (__uint128_t *)dst;
	const __uint128_t *src128 = (const __uint128_t *)src;
	const __uint128_t
		x0 = src128[0], x1 = src128[1], x2 = src128[2], x3 = src128[3];
	dst128[0] = x0;
	dst128[1] = x1;
	dst128[2] = x2;
	dst128[3] = x3;
}

static inline
void rte_mov128(unsigned char *dst, const unsigned char *src)
{
	__uint128_t *dst128 = (__uint128_t *)dst;
	const __uint128_t *src128 = (const __uint128_t *)src;
	/* Keep below declaration & copy sequence for optimized instructions */
	const __uint128_t
		x0 = src128[0], x1 = src128[1], x2 = src128[2], x3 = src128[3];
	dst128[0] = x0;
	__uint128_t x4 = src128[4];
	dst128[1] = x1;
	__uint128_t x5 = src128[5];
	dst128[2] = x2;
	__uint128_t x6 = src128[6];
	dst128[3] = x3;
	__uint128_t x7 = src128[7];
	dst128[4] = x4;
	dst128[5] = x5;
	dst128[6] = x6;
	dst128[7] = x7;
}

static inline
void rte_mov256(unsigned char *dst, const unsigned char *src)
{
	rte_mov128(dst, src);
	rte_mov128(dst + 128, src + 128);
}

static inline void
rte_memcpy_lt16(unsigned char *dst, const unsigned char *src, size_t n)
{
	if (n & 0x08) {
		/* copy 8 ~ 15 bytes */
		*(uint64_t *)dst = *(const uint64_t *)src;
		*(uint64_t *)(dst - 8 + n) = *(const uint64_t *)(src - 8 + n);
	} else if (n & 0x04) {
		/* copy 4 ~ 7 bytes */
		*(uint32_t *)dst = *(const uint32_t *)src;
		*(uint32_t *)(dst - 4 + n) = *(const uint32_t *)(src - 4 + n);
	} else if (n & 0x02) {
		/* copy 2 ~ 3 bytes */
		*(uint16_t *)dst = *(const uint16_t *)src;
		*(uint16_t *)(dst - 2 + n) = *(const uint16_t *)(src - 2 + n);
	} else if (n & 0x01) {
		/* copy 1 byte */
		*dst = *src;
	}
}

static inline
void rte_memcpy_ge16_lt128(unsigned char *dst, const unsigned char *src, size_t n)
{
	if (n < 64) {
		if (n == 16) {
			rte_mov16(dst, src);
		} else if (n <= 32) {
			rte_mov16(dst, src);
			rte_mov16(dst - 16 + n, src - 16 + n);
		} else if (n <= 48) {
			rte_mov32(dst, src);
			rte_mov16(dst - 16 + n, src - 16 + n);
		} else {
			rte_mov48(dst, src);
			rte_mov16(dst - 16 + n, src - 16 + n);
		}
	} else {
		rte_mov64((unsigned char *)dst, (const unsigned char *)src);
		if (n > 48 + 64)
			rte_mov64(dst - 64 + n, src - 64 + n);
		else if (n > 32 + 64)
			rte_mov48(dst - 48 + n, src - 48 + n);
		else if (n > 16 + 64)
			rte_mov32(dst - 32 + n, src - 32 + n);
		else if (n > 64)
			rte_mov16(dst - 16 + n, src - 16 + n);
	}
}

static inline
void rte_memcpy_ge128(unsigned char *dst, const unsigned char *src, size_t n)
{
	do {
		rte_mov128(dst, src);
		src += 128;
		dst += 128;
		n -= 128;
	} while (n >= 128);

	if (n) {
		if (n <= 16)
			rte_mov16(dst - 16 + n, src - 16 + n);
		else if (n <= 32)
			rte_mov32(dst - 32 + n, src - 32 + n);
		else if (n <= 48)
			rte_mov48(dst - 48 + n, src - 48 + n);
		else if (n <= 64)
			rte_mov64(dst - 64 + n, src - 64 + n);
		else
			rte_memcpy_ge16_lt128(dst, src, n);
	}
}

static inline
void rte_memcpy_ge16_lt64(unsigned char *dst, const unsigned char *src, size_t n)
{
	if (n == 16) {
		rte_mov16(dst, src);
	} else if (n <= 32) {
		rte_mov16(dst, src);
		rte_mov16(dst - 16 + n, src - 16 + n);
	} else if (n <= 48) {
		rte_mov32(dst, src);
		rte_mov16(dst - 16 + n, src - 16 + n);
	} else {
		rte_mov48(dst, src);
		rte_mov16(dst - 16 + n, src - 16 + n);
	}
}

static inline
void rte_memcpy_ge64(unsigned char *dst, const unsigned char *src, size_t n)
{
	do {
		rte_mov64(dst, src);
		src += 64;
		dst += 64;
		n -= 64;
	} while ((n >= 64));

	if ((n)) {
		if (n <= 16)
			rte_mov16(dst - 16 + n, src - 16 + n);
		else if (n <= 32)
			rte_mov32(dst - 32 + n, src - 32 + n);
		else if (n <= 48)
			rte_mov48(dst - 48 + n, src - 48 + n);
		else
			rte_mov64(dst - 64 + n, src - 64 + n);
	}
}

void* rte_memcpy(void *dst, const void *src, size_t n)
{
	if (n < 16) {
		rte_memcpy_lt16((unsigned char *)dst, (const unsigned char *)src, n);
		return dst;
	}
	if (n < 64) {
		rte_memcpy_ge16_lt64((unsigned char *)dst, (const unsigned char *)src, n);
		return dst;
	}
	rte_memcpy_ge64((unsigned char *)dst, (const unsigned char *)src, n);
	return dst;
}
#endif
