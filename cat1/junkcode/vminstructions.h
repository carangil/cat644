/*
 * vminstructions.h
 *
 * Created: 7/17/2014 9:00:16 PM
 *  Author: mark
 */ 


#ifndef VMINSTRUCTIONS_H_
#define VMINSTRUCTIONS_H_

#ifndef VM_OFFSET
#define VM_OFFSET 0
#endif

//load immediate
#define VM_LDI_A (0 + VM_OFFSET)
#define VM_LDI_B (1 + VM_OFFSET)
#define VM_LDI_C (2 + VM_OFFSET)
#define VM_LDI_D (3 + VM_OFFSET)

//add
#define VM_ADD_B (4 + VM_OFFSET)
#define VM_ADD_C (5 + VM_OFFSET)
#define VM_ADD_D (6 + VM_OFFSET)

//sub
#define VM_SUB_B (7 + VM_OFFSET)
#define VM_SUB_C (8 + VM_OFFSET)
#define VM_SUB_D (9 + VM_OFFSET)

//and
#define VM_AND_B (10 + VM_OFFSET)
#define VM_AND_C (11 + VM_OFFSET)
#define VM_AND_D (12 + VM_OFFSET)

//xor
#define VM_XOR_B (13 + VM_OFFSET)
#define VM_XOR_C (14 + VM_OFFSET)
#define VM_XOR_D (15 + VM_OFFSET)

//or
#define VM_OR_B (16 + VM_OFFSET)
#define VM_OR_C (17 + VM_OFFSET)
#define VM_OR_D (18 + VM_OFFSET)

//complement
#define VM_INV_A (19 + VM_OFFSET)
#define VM_INV_B (20 + VM_OFFSET)
#define VM_INV_C (21 + VM_OFFSET)
#define VM_INV_D (22 + VM_OFFSET)

//load a from *a
#define VM_LDA_PA  (23 + VM_OFFSET)

//load b from *a
#define VM_LDB_PA  (24 + VM_OFFSET)

//load c from *a
#define VM_LDC_PA (25 + VM_OFFSET)

//load d from *a
#define VM_LDD_PA (26 + VM_OFFSET)

//store a in *b, *c or *d
#define VM_STA_PB (27 + VM_OFFSET)

#define VM_STA_PC (28 + VM_OFFSET)

#define VM_STA_PD (29 + VM_OFFSET)

//jump if 'a' zero
#define VM_JAZ  (30 + VM_OFFSET)
//jump if 'a' not zero
#define VM_JANZ (31 + VM_OFFSET)

//jump to A
#define VM_GOTOA (32 + VM_OFFSET)

#define VM_SWAP_B (33+VM_OFFSET)
#define VM_SWAP_C (34+VM_OFFSET)
#define VM_SWAP_D (35+VM_OFFSET)

#define VM_MOVA_B (36+VM_OFFSET)
#define VM_MOVA_C (37+VM_OFFSET)
#define VM_MOVA_D (38+VM_OFFSET)

#define VM_MOVB_A (39+VM_OFFSET)
#define VM_MOVC_A (40+VM_OFFSET)
#define VM_MOVD_A (41+VM_OFFSET)

#define VM_JMP (42 + VM_OFFSET)

#define VM_SYSCALL (43 + VM_OFFSET)

#endif /* VMINSTRUCTIONS_H_ */


