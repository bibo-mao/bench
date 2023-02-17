#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define ___constant_swab32(x) ((unsigned int)(                         \
        (((unsigned int)(x) & (unsigned int)0x000000ffUL) << 24) |            \
        (((unsigned int)(x) & (unsigned int)0x0000ff00UL) <<  8) |            \
        (((unsigned int)(x) & (unsigned int)0x00ff0000UL) >>  8) |            \
        (((unsigned int)(x) & (unsigned int)0xff000000UL) >> 24)))

#define swab32(x) (unsigned int)___constant_swab32((unsigned int)(x))

/* Looks dumb, but generates nice-ish code */
static unsigned long accumulate(unsigned long sum, unsigned long data)
{
	sum += data;
	if (sum < data)
		sum += 1;
	return sum;
}

unsigned int do_csum_interleave(const unsigned char *buff, int len)
{
       unsigned int offset, shift, sum;
        const unsigned long *ptr;
        unsigned long sum64;
        unsigned long tmp0, tmp1, tmp2, tmp3, tmp4;

        if (len == 0)
                return 0;

        offset = (unsigned long)buff & 7;
        /*
         * This is to all intents and purposes safe, since rounding down cannot
         * result in a different page or cache line being accessed, and @buff
         * should absolutely not be pointing to anything read-sensitive. We do,
         * however, have to be careful not to piss off KASAN, which means using
         * unchecked reads to accommodate the head and tail, for which we'll
         * compensate with an explicit check up-front.
         */
        ptr = (unsigned long *)(buff - offset);
        len = len + offset - 8;

        /*
         * Head: zero out any excess leading bytes. Shifting back by the same
         * amount should be at least as fast as any other way of handling the
         * odd/even alignment, and means we can ignore it until the very end.
         */
        shift = offset * 8;
        sum64 = *ptr++;
        sum64 = (sum64 >> shift) << shift;

        if (len > 32) {
                tmp0 = *(unsigned long *)ptr;
                tmp1 = *(unsigned long *)(ptr + 1);
                tmp2 = *(unsigned long *)(ptr + 2);
                tmp3 = *(unsigned long *)(ptr + 3);
                sum64 += tmp0;
                tmp1  += tmp2;
                ptr   += 4;
                len   -= 32;
                if (sum64 < tmp0)
                        sum64 += 1;

                while (len >= 48) {
                        len -= 48;
		 	__asm__ __volatile__("": :"r"(ptr),"r"(len));
                        tmp4 = *(unsigned long *)ptr;
                        if (tmp1 < tmp2)
                                tmp1 += 1;
                        tmp3 += tmp4;

                        tmp0 = *(unsigned long *)(ptr + 1);
                        if (tmp3 < tmp4)
                                tmp3 += 1;
                        sum64 += tmp0;

                        tmp2 = *(unsigned long *)(ptr + 2);
                        if (sum64 < tmp0)
                                sum64 += 1;
                        tmp1 += tmp2;

                        tmp4 = *(unsigned long *)(ptr + 3);
                        if (tmp1 < tmp2)
                                tmp1 +=1 ;
                        tmp3 += tmp4;

                        tmp0 = *(unsigned long *)(ptr + 4);
                        if (tmp3 < tmp4)
                                tmp3 +=1;
                        sum64 += tmp0;

                        tmp2 = *(unsigned long *)(ptr + 5);
                        if (sum64 < tmp0)
                                sum64 += 1;
                        tmp1 += tmp2;
                        ptr += 6;
                }
        } else
                tmp1 = tmp2 = tmp3 = 0;

        if (len >= 24) {
                len -=24;
		 __asm__ __volatile__("": :"r"(ptr),"r"(len));
                tmp4 = *(unsigned long *)ptr;
                if (tmp1 < tmp2)
                        tmp1 += 1;
                tmp3 += tmp4;

                tmp0 = *(unsigned long *)(ptr + 1);
                if (tmp3 < tmp4)
                        tmp3 += 1;
                sum64 += tmp0;

                tmp2 = *(unsigned long *)(ptr + 2);
                if (sum64 < tmp0)
                        sum64 += 1;
                tmp1 += tmp2;
                ptr +=3;
        }

        if (len > 16) {
                tmp4 = *(unsigned long *)ptr;
                if (tmp1 < tmp2)
                        tmp1 += 1;
                tmp3 += tmp4;

                tmp0 = *(unsigned long *)(ptr + 1);
                if (tmp3 < tmp4)
                        tmp3 += 1;
                sum64 += tmp0;

                tmp2 = *(unsigned long *)(ptr + 2);
                shift = (24 - len) << 3;
                tmp2 = (tmp2 << shift) >> shift;
                if (sum64 < tmp0)
                        sum64 += 1;
                tmp1 += tmp2;
                len -= 24;
                ptr += 3;
        }

        if (tmp1 < tmp2)
                tmp1 += 1;
        tmp1  = accumulate(tmp1, tmp3);
        sum64 = accumulate(sum64, tmp1);
        if (len >= 8) {
                len -= 8;
                tmp0 = *(unsigned long *)ptr;
                sum64 = accumulate(sum64, tmp0);
                ptr += 1;
        }

        if (len > 0) {
                tmp0 = *(unsigned long *)ptr;
                shift = (8 -len) << 3;
                tmp0 = (tmp0 << shift) >> shift;
                sum64 = accumulate(sum64, tmp0);
        }

        /* Finally, folding */
        sum64 += (sum64 >> 32) | (sum64 << 32);
        sum = sum64 >> 32;
        sum += (sum >> 16) | (sum << 16);
        if (offset & 1)
                return (unsigned short)swab32(sum);

        return sum >> 16;
}

/*
 * come from linux/arch/arm64/lib/csum.c
 * We over-read the buffer and this makes KASAN unhappy. Instead, disable
 * instrumentation and call kasan explicitly.
 */
unsigned int do_csum_128(const unsigned char *buff, int len)
{
        unsigned int offset, shift, sum;
        const unsigned long *ptr;
        unsigned long data, sum64 = 0;

        if (len == 0)
                return 0;

        offset = (unsigned long)buff & 7;
        /*
         * This is to all intents and purposes safe, since rounding down cannot
         * result in a different page or cache line being accessed, and @buff
         * should absolutely not be pointing to anything read-sensitive. We do,
         * however, have to be careful not to piss off KASAN, which means using
         * unchecked reads to accommodate the head and tail, for which we'll
         * compensate with an explicit check up-front.
         */
        ptr = (unsigned long *)(buff - offset);
        len = len + offset - 8;

        /*
         * Head: zero out any excess leading bytes. Shifting back by the same
         * amount should be at least as fast as any other way of handling the
         * odd/even alignment, and means we can ignore it until the very end.
         */
        shift = offset * 8;
        data = *ptr++;
#ifdef __LITTLE_ENDIAN
        data = (data >> shift) << shift;
#else
        data = (data << shift) >> shift;
#endif

        /*
         * Body: straightforward aligned loads from here on (the paired loads
         * underlying the quadword type still only need dword alignment). The
         * main loop strictly excludes the tail, so the second loop will always
         * run at least once.
         */
        while (len > 64) {
                __uint128_t tmp1, tmp2, tmp3, tmp4;

                tmp1 = *(__uint128_t *)ptr;
                tmp2 = *(__uint128_t *)(ptr + 2);
                tmp3 = *(__uint128_t *)(ptr + 4);
                tmp4 = *(__uint128_t *)(ptr + 6);

                len -= 64;
                ptr += 8;
		 __asm__ __volatile__("": :"r"(ptr),"r"(len));

                /* This is the "don't dump the carry flag into a GPR" idiom */
                tmp1 += (tmp1 >> 64) | (tmp1 << 64);
                tmp2 += (tmp2 >> 64) | (tmp2 << 64);
                tmp3 += (tmp3 >> 64) | (tmp3 << 64);
                tmp4 += (tmp4 >> 64) | (tmp4 << 64);
                tmp1 = ((tmp1 >> 64) << 64) | (tmp2 >> 64);
                tmp1 += (tmp1 >> 64) | (tmp1 << 64);
                tmp3 = ((tmp3 >> 64) << 64) | (tmp4 >> 64);
                tmp3 += (tmp3 >> 64) | (tmp3 << 64);
                tmp1 = ((tmp1 >> 64) << 64) | (tmp3 >> 64);
                tmp1 += (tmp1 >> 64) | (tmp1 << 64);
                tmp1 = ((tmp1 >> 64) << 64) | sum64;
                tmp1 += (tmp1 >> 64) | (tmp1 << 64);
                sum64 = tmp1 >> 64;
        }
        while (len > 8) {
                __uint128_t tmp;

                sum64 = accumulate(sum64, data);
                tmp = *(__uint128_t *)ptr;

                len -= 16;
                ptr += 2;
		__asm__ __volatile__("": :"r"(ptr),"r"(len));

#ifdef __LITTLE_ENDIAN
                data = tmp >> 64;
                sum64 = accumulate(sum64, tmp);
#else
                data = tmp;
                sum64 = accumulate(sum64, tmp >> 64);
#endif
        }
        if (len > 0) {
                sum64 = accumulate(sum64, data);
                data = *ptr;
                len -= 8;
        }
        /*
         * Tail: zero any over-read bytes similarly to the head, again
         * preserving odd/even alignment.
         */
        shift = len * -8;
#ifdef __LITTLE_ENDIAN
        data = (data << shift) >> shift;
#else
        data = (data >> shift) << shift;
#endif
        sum64 = accumulate(sum64, data);

        /* Finally, folding */
        sum64 += (sum64 >> 32) | (sum64 << 32);
        sum = sum64 >> 32;
        sum += (sum >> 16) | (sum << 16);
        if (offset & 1)
                return (unsigned short)swab32(sum);

        return sum >> 16;
}

static inline unsigned short from32to16(unsigned int x)
{
        /* add up 16-bit and 16-bit for 16+c bit */
        x = (x & 0xffff) + (x >> 16);
        /* add up carry.. */
        x = (x & 0xffff) + (x >> 16);
        return x;
}


unsigned int do_csum_32(const unsigned char *buff, int len)
{
        int odd;
        unsigned int result = 0;

        if (len <= 0)
                goto out;
        odd = 1 & (unsigned long) buff;
        if (odd) {
#ifdef __LITTLE_ENDIAN
                result += (*buff << 8);
#else
                result = *buff;
#endif
                len--;
                buff++;
        }
        if (len >= 2) {
                if (2 & (unsigned long) buff) {
                        result += *(unsigned short *) buff;
                        len -= 2;
                        buff += 2;
                }
                if (len >= 4) {
                        const unsigned char *end = buff + ((unsigned)len & ~3);
                        unsigned int carry = 0;
                        do {
                                unsigned int w = *(unsigned int *) buff;
                                buff += 4;
                                result += carry;
                                result += w;
                                carry = (w > result);
                        } while (buff < end);
                        result += carry;
                        result = (result & 0xffff) + (result >> 16);
                }
                if (len & 2) {
                        result += *(unsigned short *) buff;
                        buff += 2;
                }
        }
        if (len & 1)
#ifdef __LITTLE_ENDIAN
                result += *buff;
#else
                result += (*buff << 8);
#endif
        result = from32to16(result);
        if (odd)
                result = ((result >> 8) & 0xff) | ((result & 0xff) << 8);
out:
        return result;
}


static inline unsigned short from64to16(unsigned long x)
{
        /* Using extract instructions is a bit more efficient
           than the original shift/bitmask version.  */

        union {
                unsigned long   ul;
                unsigned int    ui[2];
                unsigned short  us[4];
        } in_v, tmp_v, out_v;

        in_v.ul = x;
        tmp_v.ul = (unsigned long) in_v.ui[0] + (unsigned long) in_v.ui[1];

        /* Since the bits of tmp_v.sh[3] are going to always be zero,
           we don't have to bother to add that in.  */
        out_v.ul = (unsigned long) tmp_v.us[0] + (unsigned long) tmp_v.us[1]
                        + (unsigned long) tmp_v.us[2];

        /* Similarly, out_v.us[2] is always zero for the final add.  */
        return out_v.us[0] + out_v.us[1];
}

/*
 * come from linux/arch/alpha/lib/checksum.c
 */
int do_csum_64(const unsigned char * buff, int len)
{
        int odd, count;
        unsigned long result = 0;

        if (len <= 0)
                goto out;
        odd = 1 & (unsigned long) buff;
        if (odd) {
                result = *buff << 8;
                len--;
                buff++;
        }
        count = len >> 1;               /* nr of 16-bit words.. */
        if (count) {
                if (2 & (unsigned long) buff) {
                        result += *(unsigned short *) buff;
                        count--;
                        len -= 2;
                        buff += 2;
                }
                count >>= 1;            /* nr of 32-bit words.. */
#if 1		
                if (count) {
                        if (4 & (unsigned long) buff) {
                                result += *(unsigned int *) buff;
                                count--;
                                len -= 4;
                                buff += 4;
                        }
                        count >>= 1;    /* nr of 64-bit words.. */
                        if (count) {
                                unsigned long carry = 0;
                                do {
                                        unsigned long w = *(unsigned long *) buff;
                                        count--;
                                        buff += 8;
                                        result += carry;
                                        result += w;
                                        carry = (w > result);
                                } while (count);
                                result += carry;
                                result = (result & 0xffffffff) + (result >> 32);
                        }
                        if (len & 4) {
                                result += *(unsigned int *) buff;
                                buff += 4;
                        }
                }
#else		
			while (count) {
                                result += *(unsigned int *) buff;
                                count--;
                                len -= 4;
                                buff += 4;
                        }
			result = (result & 0xffffffff) + (result >> 32);

#endif
                if (len & 2) {
                        result += *(unsigned short *) buff;
                        buff += 2;
                }
        }
        if (len & 1)
                result += *buff;
        result = from64to16(result);
        if (odd)
                result = ((result >> 8) & 0xff) | ((result & 0xff) << 8);
out:
        return result;
}
