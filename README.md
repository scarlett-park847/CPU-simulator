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