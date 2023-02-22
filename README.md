# MIPS-pipeline

## Purpose

This project is intended to understand in detail how a pipelined
implementation works. A cycle-accurate simulator for a pipelined implementation of the LC is completed with data forwarding and branch prediction. This project constructs a pipeline simulator,
written in C++.

## LC Instruction-Set Architecture

The LC is an 8-register, 32-bit computer.  All addresses are word-addresses.
The LC has 65536 words of memory. By assembly-language convention, register 0 will always
contain the value 0. IE: Never write to register 0!

There are 3 instruction formats (bit 0 is the least-significant bit).  Bits
31-25 are unused for all instructions, and should always be 0.

```sh
R-type instructions (add, nand, cmp):
    bits 24-22: opcode
    bits 21-19: reg A
    bits 18-16: reg B
    bits 15-0:  unused (should all be 0)

I-type instructions (movl, movs, je):
    bits 24-22: opcode
    bits 21-19: reg A
    bits 18-16: reg B
    bits 15-0:  offsetField (an 16-bit, 2's complement number with a range of
		    -32768 to 32767)

O-type instructions (halt, noop):
    bits 24-22: opcode
    bits 21-0:  unused (should all be 0)
```

```

-------------------------------------------------------------------------------
Table 1: Description of Machine Instructions
-------------------------------------------------------------------------------
Assembly language 	Opcode in binary		Action
name for instruction	(bits 24, 23, 22)
-------------------------------------------------------------------------------
add (R-type format)	    000 			add contents of regA with
						contents of regB, store
						results in regA.

nand (R-type format)	001			nand contents of regA with
						contents of regB, store
						results in regA.

movl (I-type format)	010			load regB from memory. Memory
						address is formed by adding
						offsetField with the contents of
						regA. regB = Memory[regA + offset]

movs (I-type format)	011			store regB into memory. Memory
						address is formed by adding
						offsetField with the contents of
						regA. Memory[regA + offset] = regB

je   (I-type format)	100			if the cmp flag is 1 then branch
						to the address PC+1+offsetField,
						where PC is the address of the
						je instruction.

cmp  (R-type format)	101 			set the cmp flag to 1 if the contents
                                                regA and regB are the same.
                                                Otherwise cmp flag is 0.

halt (O-type format)	110			increment the PC (as with all
						instructions), then halt the
						machine (let the simulator
						notice that the machine
						halted).

noop (O-type format)	111			do nothing.
-------------------------------------------------------------------------------
```

## LC Assembly Language and Assembler

a.c file is an assembler that translates LC assembly code into machine code.
The format for a line of assembly code is (<white> means a series of tabs and/or spaces):

label<white>instruction<white>field0<white>field1<white>field2<white>comments

The leftmost field on a line is the label field.  Valid labels contain a
maximum of 6 characters and can consist of letters and numbers (but must start
with a letter). The label is optional (the white space following the label
field is required).  Labels make it much easier to write assembly-language
programs, since otherwise you would need to modify all address fields each time
you added a line to your assembly-language program!

After the optional label is white space.  Then follows the instruction field,
where the instruction can be any of the assembly-language instruction names
listed in the above table.  After more white space comes a series of fields.
All fields are given as decimal numbers or labels.  The number of fields
depends on the instruction, and unused fields should be ignored (treat them
like comments).

    R-type instructions (add, nand) instructions require 2 fields: field0
    is regA and field1 is regB.

    I-type instructions (movl, movs) require 3 fields: field0 is regA, field1
    is regB, and field2 is either a numeric value for offsetField or a symbolic
    address.  Numeric offsetFields can be positive or negative; symbolic
    addresses are discussed below.

    R-type instructions (cmp, je) instructions require 1 fields: field0
    is the offset.

    O-type instructions (noop and halt) require no fields.

Symbolic addresses refer to labels.  For lw or sw instructions, the assembler
should compute offsetField to be equal to the address of the label.  This could
be used with a zero base register to refer to the label, or could be used with
a non-zero base register to index into an array starting at the label.  For je
instructions, the assembler should translate the label into the numeric
offsetField needed to branch to that label.

After the last used field comes more white space, then any comments.  The
comment field ends at the end of a line.  Comments are vital to creating
understandable assembly-language programs, because the instructions themselves
are rather cryptic.

In addition to LC instructions, an assembly-language program may contain
directions for the assembler. The only assembler directive we will use is 'dd'
(note the leading period). 'dd' tells the assembler to put a number into the
place where the instruction would normally be stored. 'dd' instructions use
one field, which can be either a numeric value or a symbolic address.  For
example, "dd 32" puts the value 32 where the instruction would normally be
stored.  'dd' with a symbolic address will store the address of the label.
In the example below, "dd start" will store the value 2, because the label
"start" is at address 2.

The assembler makes two passes over the assembly-language program. In the
first pass, it will calculate the address for every symbolic label, assuming
that the first instruction is at address 0.  In the second pass, it will
generate a machine-language instruction (in decimal) for each line of assembly
language.  For example, here is an assembly-language program (that counts down
from 5, stopping when it hits 0).

> Assembly Example

```sh
	mov	1	[0+five]	load reg1 with 5 (uses symbolic address) (movl)
	mov	2       [1+5]		load reg2 with -1 (uses literal offset) (movl)
start	add	1	2		decrement reg1
	cmp	0	1		compare contents of register 0 to register 1
	je	3	 		goto end of program when cmp flag is 1
	cmp	0	0		set cmp flag to 1
	je	start			go back to the beginning of the loop
	noop
done	halt				end of program
five	dd	5
neg1	dd	-1
stAddr	dd	start			will contain the address of start (2)
```

> Machine Code from a.c file Example

```sh

(address   0):   8454153 (hex 0x00810009)
(address   1):   9043973 (hex 0x008a0005)
(address   2):    655360 (hex 0x000a0000)
(address   3):  21037056 (hex 0x01410000)
(address   4):  16842751 (hex 0x0100ffff)
(address   5):  20971520 (hex 0x01400000)
(address   6):  16842747 (hex 0x0100fffb)
(address   7):  29360128 (hex 0x01c00000)
(address   8):  25165824 (hex 0x01800000)
(address   9):         5 (hex 0x00000005)
(address  10):        -1 (hex 0xffffffff)
(address  11):         2 (hex 0x00000002)
```

### Note!!
- a.c file takes je as an input. However, it does not accept beq instruction.
- beq intruction is also I-type instructions that requires field0, field1 and the offset.
- In the simulator, I used beq instead of cmp and je
- beq compares the values of field0 and field1 and if they are the same, pc moves to pc+1+offset