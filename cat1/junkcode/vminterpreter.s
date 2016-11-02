
/*
 * Assembly1.s
 *
 * Created: 5/19/2014 10:33:54 PM
 *  Author: mark
 */ 


.global interpreter

#include "vminstructions.h"




/*Define register pair 'halfs' */
#define XL r26
#define XH r27

#define YL r28
#define YH r29

#define ZL r30
#define ZH r31


/*Define VM registers */

/*virtual stack*/
#define VSP Y
#define VSPH YH
#define VSPL YL

/* Instruction pointer */
#define IP	X
#define IPL XL
#define IPH XH

#define AL  r2
#define AH	r3
#define BL	r4
#define BH	r5
#define CL	r6
#define CH	r7
#define DL	r8
#define DH	r9


#define INST_ADDR(num)  (num * 256 * 2)

#define SETZL     ldi ZL, lo8(pm(instr0))
#define DISPATCH  ld  ZH, IP+	  $ 	ijmp

.global interpreter

/* Load immediates to A, B, C, or D */
.org INST_ADDR(VM_LDI_A)
instr0:
	LD AL, IP+
	LD AH, IP+
	DISPATCH

.org INST_ADDR(VM_LDI_B)
	LD BL, IP+
	LD BH, IP+
	DISPATCH

.org INST_ADDR(VM_LDI_C)
	LD CL, IP+
	LD CH, IP+
	DISPATCH
   
.org INST_ADDR(VM_LDI_D)
	LD DL, IP+
	LD DH, IP+
	DISPATCH

	

.org INST_ADDR(VM_ADD_B)
	add AL, BL
	adc AH, BH
	DISPATCH

	/*ADD B C or D to A */
.org INST_ADDR(VM_ADD_C)
	add AL, CL
	adc AH, CH
	DISPATCH

.org INST_ADDR(VM_ADD_D)
	add AL, DL
	adc AH, DH
	DISPATCH

	/* SUB B C or D from A */
.org INST_ADDR(VM_SUB_B)
	sub AL, BL
	sbc AH, BH
	DISPATCH

.org INST_ADDR(VM_SUB_C)
	sub AL, CL
	sbc AH, CH
	DISPATCH

.org INST_ADDR(VM_SUB_D)
	sub AL, DL
	sbc AH, DH
	DISPATCH

	/*AND B C or D to A */
.org INST_ADDR(VM_AND_B)
	and AL, BL
	and AH, BH
	DISPATCH

.org INST_ADDR(VM_AND_C)
	and AL, CL
	and AH, CH
	DISPATCH

.org INST_ADDR(VM_AND_D)
	and AL, DL
	and AH, DH
	DISPATCH

	/* XOR B C or D to A */
.org INST_ADDR(VM_XOR_B)
	eor AL, BL
	eor AH, BH
	DISPATCH

.org INST_ADDR(VM_XOR_C)
	eor AL, CL
	eor AH, CH
	DISPATCH

.org INST_ADDR(VM_XOR_D)
	eor AL, DL
	eor AH, DH
	DISPATCH

	/* OR B C or D to A */
.org INST_ADDR(VM_OR_B)
	or AL, BL
	or AH, BH
	DISPATCH

.org INST_ADDR(VM_OR_C)
	or AL, CL
	or AH, CH
	DISPATCH

.org INST_ADDR(VM_OR_D)
	or AL, DL
	or AH, DH
	DISPATCH


.org INST_ADDR(VM_INV_A)
	COM AL
	COM AH
	DISPATCH

.org INST_ADDR(VM_INV_B)
	COM BL
	COM BH
	DISPATCH

.org INST_ADDR(VM_INV_C)
	COM CL
	COM CH
	DISPATCH
   
.org INST_ADDR(VM_INV_D)
	COM DL
	COM DH
	DISPATCH


/* Load *A into register */
.org INST_ADDR(VM_LDA_PA)
	mov ZH, AH
	mov ZL, AL
	ld AL, Z+
	ld AH, Z+
	SETZL
	DISPATCH

.org INST_ADDR(VM_LDB_PA)
	mov ZH, AH
	mov ZL, AL
	ld BL, Z+
	ld BH, Z+
	SETZL
	DISPATCH

.org INST_ADDR(VM_LDC_PA)
	mov ZH, AH
	mov ZL, AL
	ld CL, Z+
	ld CH, Z+
	SETZL
	DISPATCH

.org INST_ADDR(VM_LDD_PA)
	mov ZH, AH
	mov ZL, AL
	ld DL, Z+
	ld DH, Z+
	SETZL
	DISPATCH

/* Store A into *B, *C, *D */
.org INST_ADDR(VM_STA_PB)
	mov ZH, BH
	mov ZL, BL
	st  Z+, AL
	st  Z+, AH
	SETZL
	DISPATCH

.org INST_ADDR(VM_STA_PC)
	mov ZH, CH
	mov ZL, CL
	st  Z+, AL
	st  Z+, AH
	SETZL
	DISPATCH


.org INST_ADDR(VM_STA_PD)
	mov ZH, DH
	mov ZL, DL
	st  Z+, AL
	st  Z+, AH
	SETZL
	DISPATCH


	/*jump if A is zero*/
.org INST_ADDR(VM_JAZ)

	mov r0, AL
	or  r0, AH   /* or second value on top */
	brne jmpazdispatch   /* if result is not zero, go directly to dispatch (vm does not jump) */

	ld r0, IP+   //fetch low address
	ld r1, IP+   //fetch high address
	movw IPL, r0   //ip = r1:r0
	DISPATCH

jmpazdispatch:
	adiw   IPL, 2   //skip destination address
	DISPATCH



	/*jump if A not zero*/
.org INST_ADDR(VM_JANZ)

	mov r0, AL
	or  r0, AH   /* or second value on top */
	breq jmpanzdispatch   /* if result is zero, go directly to dispatch (vm does not jump) */

	ld r0, IP+   //fetch low address
	ld r1, IP+   //fetch high address
	movw IPL, r0   //ip = r1:r0
	DISPATCH

jmpanzdispatch:
	adiw   IPL, 2   //skip destination address
	DISPATCH

	
.org INST_ADDR(VM_SWAP_B)
	movw r0, AL
	movw AL, BL
	movw BL, r0
	DISPATCH


.org INST_ADDR(VM_SWAP_C)
	movw r0, AL
	movw AL, CL
	movw CL, r0
	DISPATCH


.org INST_ADDR(VM_SWAP_D)
	movw r0, AL
	movw AL, DL
	movw DL, r0
	DISPATCH

.org INST_ADDR(VM_MOVA_B)
	mov AL, BL
	mov AH, BH
	DISPATCH

.org INST_ADDR(VM_MOVA_C)
	mov AL, CL
	mov AH, CH
	DISPATCH
	
.org INST_ADDR(VM_MOVA_D)
	mov AL, DL
	mov AH, DH
	DISPATCH

.org INST_ADDR(VM_MOVB_A)
	mov BL, AL
	mov BH, AH
	DISPATCH

.org INST_ADDR(VM_MOVC_A)
	mov CL, AL
	mov CH, AH
	DISPATCH
		
.org INST_ADDR(VM_MOVD_A)
	mov DL, AL
	mov DH, AH
	DISPATCH

/*unconditional jump */

.org INST_ADDR(VM_JMP)

	ld r0, IP+   //fetch low address
	ld r1, IP+   //fetch high address
	movw IPL, r0   //ip = r1:r0
	DISPATCH


.org INST_ADDR(VM_SYSCALL)
	eor r1,r1   //clear r1
	ld r24, IP+   
	movw r22, AL
	movw r20, BL
	call dosyscall
	DISPATCH


interpreter:
   /* set up interpreter IP */

   lds IPL, ip
   lds IPH, ip+1


   /* set up interpreter VSP */
   lds VSPL, istack
   lds VSPH, istack+1
    
 
   SETZL
   DISPATCH
   
   


