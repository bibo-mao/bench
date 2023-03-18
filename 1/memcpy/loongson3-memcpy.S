#define zero    $r0     /* wired zero */
#define ra      $r1     /* return address */
#define tp      $r2
#define sp      $r3     /* stack pointer */
#define a0      $r4     /* argument registers */
#define a1      $r5
#define a2      $r6
#define a3      $r7
#define a4      $r8
#define a5      $r9
#define a6      $r10
#define a7      $r11
#define t0      $r12    /* caller saved */
#define t1      $r13
#define t2      $r14
#define t3      $r15
#define t4      $r16
#define t5      $r17
#define t6      $r18
#define t7      $r19
#define t8      $r20
#define u0      $r21
#define fp      $r22    /* frame pointer */
#define s0      $r23    /* callee saved */
#define s1      $r24
#define s2      $r25
#define s3      $r26
#define s4      $r27
#define s5      $r28
#define s6      $r29
#define s7      $r30
#define s8      $r31

/*
 * LEAF - declare leaf routine
 */
#define LEAF(symbol)                                    \
                .globl  symbol;                         \
                .align  2

/*
 * END - mark end of function
 */
#define END(function)                                   \
                .end    function

/* void * memcpy (void *s1, const void *s2, size_t n); */
	.text
	.align	5
LEAF(ls_memcpy)				/* a0=dst a1=src a2=len */
ls_memcpy:

        ori     a3, zero, 64
        blt     a2, a3, less_64bytes
        andi    t1, a0, 7
        beqz    t1, 1f
        addi.d  t2, zero, 8
20:     ld.d    t0, a1, 0
        sub.d   t2, t2, t1
21:     st.d    t0, a0, 0
        sub.d   a2, a2, t2
        add.d   a1, a1, t2
        add.d   a0, a0, t2
        blt     a2, a3, less_64bytes

        /* copy 64 bytes at a time */
1:      ld.d    t0, a1, 0
2:      ld.d    t1, a1, 8
3:      ld.d    t2, a1, 16
4:      ld.d    t3, a1, 24

9:      st.d    t0, a0, 0
10:     st.d    t1, a0, 8
11:     st.d    t2, a0, 16
12:     st.d    t3, a0, 24

        addi.d  a1, a1, 64
        addi.d  a0, a0, 64
        addi.d  a2, a2, -64

5:      ld.d    t0, a1, 32 - 64
6:      ld.d    t1, a1, 40 - 64
7:      ld.d    t2, a1, 48 -64
8:      ld.d    t3, a1, 56 - 64

13:     st.d    t0, a0, 32 - 64
14:     st.d    t1, a0, 40 - 64
15:     st.d    t2, a0, 48 -64
16:     st.d    t3, a0, 56 - 64

        bge     a2, a3, 1b

less_64bytes:
        /* copy the remaining bytes */
	andi    t1, a2, 0x38
	andi    t2, a2, 7
	beqz    t1, less_8bytes
40:     ld.d    t0, a1, 0
        addi.d  a2, a2, -8
41:     st.d    t0, a0, 0
        addi.d  a0, a0, 8
        addi.d  a1, a1, 8
        bne     a2, t2,  40b

less_8bytes:
        beqz    a2, 19f
17:     ld.b    t0, a1, 0
        addi.d  a2, a2, -1
18:     st.b    t0, a0, 0
        addi.d  a0, a0, 1
        addi.d  a1, a1, 1
        bgt     a2, zero, 17b

        /* return */
19:     move    a0, a2
        jr      ra
