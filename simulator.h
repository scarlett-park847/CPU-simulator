#ifndef __SIMULATOR_H__
#define __SIMULATOR_H__


#define NUMMEMORY 65536 /* maximum number of data words in memory */
#define NUMREGS 8 /* number of machine registers */

#define ADD 0
#define NAND 1
#define MOVL 2
#define MOVS 3
#define BEQ 4 //JE?
#define CMP 5
#define HALT 6
#define NOOP 7

#define NOOPINSTRUCTION 0x1c00000

typedef struct IFIDStruct {
    int instr;
    int pcPlus1;
} IFIDType;

typedef struct IDEXStruct {
    int instr;
    int pcPlus1;
    int readRegA;
    int readRegB;
    int contentRegA;
    int contentRegB;
    int offset;
} IDEXType;

typedef struct EXMEMStruct {
    int instr;
    int offset;
    int cmpFlag;
    int branchTarget;
    int contentRegA;
    int contentRegB;
    int aluResult;
    int readRegA;
    int readRegB;
} EXMEMType;

typedef struct MEMWBStruct {
    int instr;
    int aluResult;
    int readRegA;
    int readRegB;
    int contentRegB;
    int writeData;
} MEMWBType;

typedef struct WBENDStruct {
    int instr;
    int readRegA;
    int readRegB;
    int writeData;
} WBENDType;

typedef struct stateStruct {
    int pc;
    int instrMem[NUMMEMORY];
    int dataMem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
    IFIDType IFID;
    IDEXType IDEX;
    EXMEMType EXMEM;
    MEMWBType MEMWB;
    WBENDType WBEND;
    int cycles; /* number of cycles run so far */
} stateType;


int field0(int instruction);

int field1(int instruction);

int field2(int instruction);

int opcode(int instruction);

void printInstruction(int instr);

void printState(stateType *statePtr);

void statePrep(stateType *statePtr);

int convertNum(int num);

void forward(stateType *statePtr);



#endif