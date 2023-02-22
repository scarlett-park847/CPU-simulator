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

