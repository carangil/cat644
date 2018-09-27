
/*
 * Assembly1.s
 *
 * Created: 5/19/2014 10:33:54 PM
 *  Author: mark
 */ 


.global interpreter

//#include "vminstructions.h"

/*Define register pair 'halfs' */
//X is caller-saved in AVR
#define XL r26
#define XH r27

//Y is callee-saved
#define YL r28
#define YH r29

//Z is caller-saved
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


//these ones are caller saved

//register A is set up to be the 1st 16-bit value passed to a c function, just based on its position.it is also the return value
#define AL  r24
#define AH	r25


//these ones are saved by the callee
#define BL	r2
#define BH	r3
#define CL	r4
#define CH	r5
#define DL	r6
#define DH	r7


#define OFFSETL r8
#define OFFSETH r9



#define SETZH     ldi ZH, hi8(pm(instr0))


#define DISPATCHLD	ld  ZL, IP+
#define DISPATCHJ	ijmp


.global interpreter
.global instr0


/* Load immediates to A, B, C, or D */

//first instruction

.section vmhandlers,"ax",@progbits


//.org 0x100    //skip forward a bit

#define global_label(name)  .global name $ name:  //$ is the line seperator


instr0:
// Load immediate value

global_label(vm_ldi_a)
	LD AL, IP+
	LD AH, IP+
global_label(vm_nop)
	DISPATCHLD
	DISPATCHJ

global_label(vm_ldi_b)
	LD BL, IP+
	LD BH, IP+
	DISPATCHLD
	DISPATCHJ

global_label(vm_ldi_c)
	LD CL, IP+
	LD CH, IP+
	DISPATCHLD
	DISPATCHJ
   
global_label(vm_ldi_d)
	LD DL, IP+
	LD DH, IP+
	DISPATCHLD
	DISPATCHJ


//Add register to A

global_label(vm_add_b)
	add AL, BL
	adc AH, BH
	DISPATCHLD
	DISPATCHJ

global_label(vm_add_c)
	add AL, CL
	adc AH, CH
	DISPATCHLD
	DISPATCHJ




global_label(vm_add_d)
	add AL, DL
	adc AH, DH
	DISPATCHLD
	DISPATCHJ



//Subtract register from A

global_label(vm_sub_b)
	sub AL, BL
	sbc AH, BH
	DISPATCHLD
	DISPATCHJ

global_label(vm_sub_c)
	sub AL, CL
	sbc AH, CH
	DISPATCHLD
	DISPATCHJ


global_label(vm_sub_d)
	sub AL, DL
	sbc AH, DH
	DISPATCHLD
	DISPATCHJ


//and
global_label(vm_and_b)
	and AL, BL
	and AH, BH
	DISPATCHLD
	DISPATCHJ

global_label(vm_and_c)
	and AL, CL
	and AH, CH
	DISPATCHLD
	DISPATCHJ


global_label(vm_and_d)
	and AL, DL
	and AH, DH
	DISPATCHLD
	DISPATCHJ

//or
global_label(vm_or_b)
	or AL, BL
	or AH, BH
	DISPATCHLD
	DISPATCHJ

global_label(vm_or_c)
	or AL, CL
	or AH, CH
	DISPATCHLD
	DISPATCHJ


global_label(vm_or_d)
	or AL, DL
	or AH, DH
	DISPATCHLD
	DISPATCHJ

//xor
global_label(vm_xor_b)
	eor AL, BL
	eor AH, BH
	DISPATCHLD
	DISPATCHJ

global_label(vm_xor_c)
	eor AL, CL
	eor AH, CH
	DISPATCHLD
	DISPATCHJ


global_label(vm_xor_d)
	eor AL, DL
	eor AH, DH
	DISPATCHLD
	DISPATCHJ
	

global_label(vm_clr)
	clr AL
	clr AH
	DISPATCHLD
	DISPATCHJ

	

global_label(vm_bswap)
	mov r0, AL
	mov AL, AH
	mov AH, r0
	DISPATCHLD
	DISPATCHJ


global_label(vm_jz)  //jump if last result was zero (from subtract, etc)
	breq vm_jmpr
	rjmp skipjump

global_label(vm_jnz)  //jump if last result was not zero
	brne vm_jmpr
	rjmp skipjump

global_label(vm_jneg)  //jump if negative
	brmi vm_jmpr
	rjmp skipjump

global_label(vm_jb)  //jump if below
	brlo vm_jmpr
	rjmp skipjump

global_label(vm_jae)  //jump if above or equal (unsigned)
	brsh vm_jmpr
	rjmp skipjump


global_label(vm_call_d) //call 'd' (not relatively)
	movw IPL, DL
	DISPATCHLD
	DISPATCHJ


global_label(vm_callr) //abuse the C stack as a control stack for the VM program
	push IPH  //when popping, need to add '2'
	push IPL
	//fall through to jump

global_label(vm_jmpr)
	LD r0, IP+     //amount to jump
	LD r1, IP+     //amount to jump
	add IPL,r0
	adc IPH,r1
	DISPATCHLD
	DISPATCHJ

global_label(vm_ret)
	pop IPL
	pop IPH
	adiw IPL,2
	DISPATCHLD
	DISPATCHJ


//swap Register with A

global_label(vm_swp_b)
	movw r0, BL
	movw BL, AL
	movw AL, r0
	DISPATCHLD
	DISPATCHJ

global_label(vm_swp_c)
	movw r0, CL
	movw CL, AL
	movw AL, r0
	DISPATCHLD
	DISPATCHJ

global_label(vm_swp_d)
	movw r0, DL
	movw DL, AL
	movw AL, r0
	DISPATCHLD
	DISPATCHJ
	

//copy 'a' to a register

global_label(vm_mov_a_b)
	movw AL, BL
	DISPATCHLD
	DISPATCHJ


global_label(vm_mov_a_c)
	movw AL, CL
	DISPATCHLD
	DISPATCHJ

global_label(vm_mov_a_d)
	movw AL, DL
	DISPATCHLD
	DISPATCHJ



//special operations on 'A' only:
global_label(vm_com)  //complement
	com AL
	com AH
	DISPATCHLD
	DISPATCHJ

global_label(vm_asr)  //signed shift right
	asr AH  //asr shift  right, low bit goes in carry
	ror AL	// shift right, top bit takes the carry
	DISPATCHLD
	DISPATCHJ

global_label(vm_shr)  //unsigned shift right
	asr AH  //asr shift  right, low bit goes in carry
	ror AL	// shift right, top bit takes the carry
	DISPATCHLD
	DISPATCHJ

global_label(vm_shl)  //unsigned shift left
	lsl AL
	rol AH
	DISPATCHLD
	DISPATCHJ



global_label(vm_push_a)
	st -VSP, AH
	st -VSP, AL
	DISPATCHLD
	DISPATCHJ

global_label(vm_push_b)
	st -VSP, BH
	st -VSP, BL
	DISPATCHLD
	DISPATCHJ


global_label(vm_push_c)
	st -VSP, CH
	st -VSP, CL
	DISPATCHLD
	DISPATCHJ


global_label(vm_push_d)
	st -VSP, DH
	st -VSP, DL
	DISPATCHLD
	DISPATCHJ


global_label(vm_pop_a) //untested
	ld AL, VSP+
	ld AH, VSP+
	DISPATCHLD
	DISPATCHJ

	
global_label(vm_pop_b) //untested
	ld AL, VSP+
	ld AH, VSP+
	DISPATCHLD
	DISPATCHJ

global_label(vm_pop_c) //untested
	ld AL, VSP+
	ld AH, VSP+
	DISPATCHLD
	DISPATCHJ

global_label(vm_pop_d) //untested
	ld AL, VSP+
	ld AH, VSP+
	DISPATCHLD
	DISPATCHJ


global_label(vm_pick_a)  
	ld r0, IP+  //get displacement  
	add VSPL, r0  //add up into stack
	ld AL, Y
	ldd AH, Y+1
	sub VSPL, r0
	DISPATCHLD
	DISPATCHJ

global_label(vm_put_a)  
	ld r0, IP+  //get displacement  
	add VSPL, r0  //add up into stack
	st Y, AL
	std Y+1,  AH
	sub VSPL, r0
	DISPATCHLD
	DISPATCHJ



global_label(vm_pop) //remove n items from stack  (n is byte numbers, so to remove words, do 2x)
	rjmp h_pop




//load register from address computed in 'A'

global_label(vm_lda_a) //ld *a into a
	movw ZL, AL //copy a ptr to Z
	ld AL, Z
	ldd AH, Z+1
	SETZH  //need Z back for stuff
	DISPATCHLD
	DISPATCHJ
	
global_label(vm_ldb_a) //ld *a into b
	movw ZL, AL //copy a ptr to Z
	ld BL, Z
	ldd BH, Z+1
	SETZH  //need Z back for stuff
	DISPATCHLD
	DISPATCHJ

global_label(vm_ldc_a) //ld *a into c
	movw ZL, AL //copy a ptr to Z
	ld CL, Z
	ldd CH, Z+1
	SETZH  //need Z back for stuff
	DISPATCHLD
	DISPATCHJ
	
global_label(vm_ldd_a) //ld *a into d
	movw ZL, AL //copy a ptr to Z
	ld DL, Z
	ldd DH, Z+1
	SETZH  //need Z back for stuff
	DISPATCHLD
	DISPATCHJ

	
//store value computed in 'A' to address in register
global_label(vm_sta_b) 
	movw ZL, BL //copy ptr to Z
	st Z, AL
	std Z+1, AH
	SETZH  //need Z back for stuff
	DISPATCHLD
	DISPATCHJ

global_label(vm_sta_c) 
	movw ZL, CL //copy ptr to Z
	st Z, AL
	std Z+1, AH
	SETZH  //need Z back for stuff
	DISPATCHLD
	DISPATCHJ
	
global_label(vm_sta_d) 
	movw ZL, DL //copy ptr to Z
	st Z, AL
	std Z+1, AH
	SETZH  //need Z back for stuff
	DISPATCHLD
	DISPATCHJ




//placeholder for function table for slower instructions where the extra jump doesn't matter
	


global_label(vm_extension)
rjmp tablefunc

//last handler only has to begin in this page.. it can be outside
global_label(instrlast)
global_label(vm_syscall)

	
	push XH
	push XL
	
	
	clr r1 //gcc uses r1 as always 0

	//register AH:AL is already in the 1st 16-bit slot for a C function
	movw  22, BL  //set up B as the second argument

	call syscall	//return value is already in register 'A'

	pop XL
	pop XH

	SETZH
	DISPATCHLD
	DISPATCHJ



interpreter:
   

/* 
	//if we ever exit the interpreter, we need to save some registers
   
	push YL
	push YH
*/

	

   movw IPL, r24    //first arg in 24/25, and contains the start place for interpreter code
   movw VSPL, r22   //second arg in 22/33, and contains the 
   movw OFFSETL, r20 //3rd arg is the offset for prog memory access (program can add SEG to any address) (not used yet)

   SETZH
   DISPATCHLD	
   DISPATCHJ



   //placehholder:  this just demonstrates we can jump out of the handler page into a different table
   tablefunc:
   SETZH
   DISPATCHLD	
   DISPATCHJ

global_label(h_subr_b)
	sub BL, AL
	sbc BH, AH
	DISPATCHLD
	DISPATCHJ

global_label(h_subr_c)
	sub CL, AL
	sbc CH, AH
	DISPATCHLD
	DISPATCHJ


global_label(h_subr_d)
	sub DL, AL
	sbc DH, AH
	DISPATCHLD
	DISPATCHJ


	
global_label(h_pop) //remove n items from stack  (n is byte numbers, so to remove words, do 2x)
	ld r0, IP+  //get displacement  
	add VSPL, r0  //add up into stack
	DISPATCHLD
	DISPATCHJ
	




skipjump:
	adiw IPL, 2    //skip over the target address
	DISPATCHLD    //continue to next instruction
	DISPATCHJ
		
