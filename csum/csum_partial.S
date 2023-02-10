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

#define LOAD   ld.d
#define LOAD32 ld.wu
#define ADD    add.d
#define SUB    sub.d
#define SLL    slli.d
#define SRL    srli.d
#define SLLV   sll.d
#define SRLV   srl.d

#define NBYTES 8
#define LOG_NBYTES 3
#define UNIT(unit)  ((unit)*NBYTES)
#define ADDRMASK (NBYTES-1)

#define _ASM_EXTABLE(from, to)			\
	.section __ex_table, "a";		\
	PTR	from, to;			\
	.previous

#define ADDC(sum,reg)						\
	ADD	sum, sum, reg;					\
	sltu	t8, sum, reg;					\
	ADD	sum, sum, t8;					\

#define ADDC32(sum,reg)						\
	add.w	sum, sum, reg;					\
	sltu	t8, sum, reg;					\
	add.w	sum, sum, t8;					\

#define CSUM_BIGCHUNK(src, offset, sum, _t0, _t1, _t2, _t3)	\
	LOAD	_t0, src, (offset + UNIT(0));			\
	LOAD	_t1, src, (offset + UNIT(1));			\
	LOAD	_t2, src, (offset + UNIT(2));			\
	LOAD	_t3, src, (offset + UNIT(3));			\
	ADDC(_t0, _t1);						\
	ADDC(_t2, _t3);						\
	ADDC(sum, _t0);						\
	ADDC(sum, _t2)

/*
 * a0: source address
 * a1: length of the area to checksum
 * a2: partial checksum
 */

#define src a0
#define sum t6

	.text
	.align	5
.globl  csum_partial;     
csum_partial:
	or	sum, zero, zero
	or	t7, zero, zero

	sltui	t8, a1, 0x8
	or	t2, a1, zero
	bnez	t8, .Lsmall_csumcpy		/* < 8 bytes to copy */

	andi	t7, src, 0x1			/* odd buffer? */

.Lhword_align:
	andi	t8, src, 0x2
	beqz	t7, .Lword_align

	ld.bu	t0, src, 0x0
	addi.d	a1, a1, -1
	slli.w	t0, t0, 8
	ADDC(sum, t0)
	addi.d	src, src, 1
	andi	t8, src, 0x2

.Lword_align:
	sltui	t4, a1, 56
	beqz	t8, .Ldword_align

	ld.hu	t0, src, 0x0
	addi.d	a1, a1, -2
	ADDC(sum, t0)
	sltui	t4, a1, 56
	addi.d	src, src, 0x2

.Ldword_align:
	or	t8, a1, zero
	bnez	t4, .Ldo_end_words

	andi	t4, src, 0x4
	andi	t8, src, 0x8
	beqz	t4, .Lqword_align

	LOAD32	t0, src, 0x0
	addi.d	a1, a1, -4
	ADDC(sum, t0)
	addi.d	src, src, 4
	andi	t8, src, 0x8

.Lqword_align:
	andi	t4, src, 0x10
	beqz	t8, .Loword_align

	ld.d	t0, src, 0
	addi.d	a1, a1, -8
	ADDC(sum, t0)
	addi.d	src, src, 8
	andi	t4, src, 0x10

.Loword_align:
	srli.d	t8, a1, 0x7
	beqz	t4, .Lbegin_movement

	ld.d	t0, src, 0x00
	ld.d	t1, src, 0x08
	ADDC(sum, t0)
	ADDC(sum, t1)
	addi.d	a1, a1, -16
	addi.d	src, src, 16
	srli.d	t8, a1, 0x7

.Lbegin_movement:
	andi	t2, a1, 0x40
	beqz	t8, 1f

	or	t5, t8, zero
.Lmove_128bytes:
	CSUM_BIGCHUNK(src, 0x00, sum, t0, t1, t3, t4)
	CSUM_BIGCHUNK(src, 0x20, sum, t0, t1, t3, t4)
	CSUM_BIGCHUNK(src, 0x40, sum, t0, t1, t3, t4)
	CSUM_BIGCHUNK(src, 0x60, sum, t0, t1, t3, t4)
	addi.d	t5, t5, -1
	addi.d	src, src, 0x80
	bnez	t5, .Lmove_128bytes

1:
	or	t4, t2, zero
	andi	t2, a1, 0x20
	beqz	t4, 1f

.Lmove_64bytes:
	CSUM_BIGCHUNK(src, 0x00, sum, t0, t1, t3, t4)
	CSUM_BIGCHUNK(src, 0x20, sum, t0, t1, t3, t4)
	addi.d	src, src, 64

1:
	andi	t8, a1, 0x1c
	beqz	t2, .Ldo_end_words

.Lmove_32bytes:
	CSUM_BIGCHUNK(src, 0x00, sum, t0, t1, t3, t4)
	andi	t8, a1, 0x1c
	addi.d	src, src, 32

.Ldo_end_words:
	andi	t2, a1, 0x3
	beqz	t8, .Lsmall_csumcpy
	srli.d	t8, t8, 0x2

.Lend_words:
	or	t4, t8, zero
1:	LOAD32	t0, src, 0x0
	addi.d	t4, t4, -1
	ADDC(sum, t0)
	addi.d	src, src, 4
	bnez	t4, 1b

/* unknown src alignment and < 8 bytes to go  */
.Lsmall_csumcpy:
	or	a1, t2, zero

	andi	t4, a1, 4
	andi	t0, a1, 2
	beqz	t4, 1f

	/* Still a full word to go  */
	ld.w	t1, src, 0x0
	addi.d	src, src, 4
	slli.d	t1, t1, 32			/* clear lower 32bit */
	ADDC(sum, t1)

1:	or	t1, zero, zero
	or	t4, t0, zero
	andi	t0, a1, 1
	beqz	t4, 1f

	/* Still a halfword to go  */
	ld.hu	t1, src, 0x0
	addi.d	src, src, 2

1:	slli.w	t1, t1, 16
	beqz	t0, 1f

	ld.bu	t2, src, 0

	or	t1, t1, t2

1:	ADDC(sum, t1)

	/* fold checksum */
	slli.d	t4, sum, 32
	add.d	sum, sum, t4
	sltu	t4, sum, t4
	srai.d	sum, sum, 32
	add.w	sum, sum, t4

	/* odd buffer alignment? */
	beqz	t7, 1f
	revb.2h	sum, sum
	/* Add the passed partial csum.	 */
1:	ADDC32(sum, a2)
	or	a0, sum, zero
	jirl	zero, ra, 0x0
