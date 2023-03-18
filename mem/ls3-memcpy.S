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

LEAF(kmemcpy)				/* a0=dst a1=src a2=len */
kmemcpy:
        beqz    a2, 19f

        ori     a3, zero, 64
        blt     a2, a3, 17f

        /* copy 64 bytes at a time */
1:      ld.d    t0, a1, 0
2:      ld.d    t1, a1, 8
3:      ld.d    t2, a1, 16
4:      ld.d    t3, a1, 24
5:      ld.d    t4, a1, 32
6:      ld.d    t5, a1, 40
7:      ld.d    t6, a1, 48
8:      ld.d    t7, a1, 56
9:      st.d    t0, a0, 0
10:     st.d    t1, a0, 8
11:     st.d    t2, a0, 16
12:     st.d    t3, a0, 24
13:     st.d    t4, a0, 32
14:     st.d    t5, a0, 40
15:     st.d    t6, a0, 48
16:     st.d    t7, a0, 56

        addi.d  a0, a0, 64
        addi.d  a1, a1, 64
        addi.d  a2, a2, -64
        bge     a2, a3, 1b

        beqz    a2, 19f

        /* copy the remaining bytes */
17:     ld.b    t0, a1, 0
18:     st.b    t0, a0, 0
        addi.d  a0, a0, 1
        addi.d  a1, a1, 1
        addi.d  a2, a2, -1
        bgt     a2, zero, 17b

        /* return */
19:     move    a0, a2
        jr      ra
