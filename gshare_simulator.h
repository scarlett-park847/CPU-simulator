#ifndef __GSHARE_SIMULATOR_H__
#define __GSHARE_SIMULATOR_H__

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>

using namespace std;

#define NUMMEMORY 65536 /* maximum number of data words in memory */
#define NUMREGS 8 /* number of machine registers */

#define PHT_size 16
#define BTB_size 16
vector <int> PHT; // pattern history table - 16 entries
int GHR; // global history register - 4bits
vector<pair<int, int> > BTB;

int CYCLES; //:   cycle time to complete program (cycle when halt reaches MEM stage)
int FETCHED; //:  # of instruction fetched (including instructions squashed because of branch misprediction)
int RETIRED; //:  # of instruction completed
int BRANCHES; //of branches executed (i.e., resolved)
int MISPRED; //:  # of branches incorrectly predicted


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
    int predictRes;
    int targetAddr;
} IDEXType;

typedef struct EXMEMStruct {
    int pcPlus1;
    int instr;
    int offset;
    int misPredict;
    int branchTarget;
    int contentRegA;
    int contentRegB;
    int aluResult;
    int readRegA;
    int readRegB;
    int predicted_res;
    int actual_res;
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


int field0(int instruction){
    return( (instruction>>19) & 0x7);
}

int field1(int instruction){
    return( (instruction>>16) & 0x7);
}

int field2(int instruction){
    return(instruction & 0xFFFF);
}

int opcode(int instruction){
    return(instruction>>22);
}

void printInstruction(int instr){
    char opcodeString[10];

    if (opcode(instr) == ADD) {
	strcpy(opcodeString, "add");
    } else if (opcode(instr) == NAND) {
	strcpy(opcodeString, "nand");
    } else if (opcode(instr) == MOVL) {
	strcpy(opcodeString, "movl");
    } else if (opcode(instr) == MOVS) {
	strcpy(opcodeString, "movs");
    } else if (opcode(instr) == BEQ) {
	strcpy(opcodeString, "je");
    } else if (opcode(instr) == CMP) {
	strcpy(opcodeString, "cmp");
    } else if (opcode(instr) == HALT) {
	strcpy(opcodeString, "halt");
    } else if (opcode(instr) == NOOP) {
	strcpy(opcodeString, "noop");
    } else {
	strcpy(opcodeString, "data");
    }

    printf("%s %d %d %d\n", opcodeString, field0(instr), field1(instr),
	field2(instr));
}
void printState(stateType *statePtr){
    int i;
    printf("\n@@@\nstate at cycle %d starts\n", statePtr->cycles);
    printf("\tpc %d\n", statePtr->pc);

    printf("\tdata memory:\n");
	for (i=0; i<statePtr->numMemory; i++) {
	    printf("\t\tdataMem[ %d ] %d\n", i, statePtr->dataMem[i]);
	}
    printf("\tregisters:\n");
	for (i=0; i<NUMREGS; i++) {
	    printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
	}

    printf("\tIFID:\n");
	printf("\t\tinstruction ");
	printInstruction(statePtr->IFID.instr);
	printf("\t\tpcPlus1 %d\n", statePtr->IFID.pcPlus1);
    printf("\tIDEX:\n");
	printf("\t\tinstruction ");
	printInstruction(statePtr->IDEX.instr);
	printf("\t\tpcPlus1 %d\n", statePtr->IDEX.pcPlus1);
	printf("\t\treadRegA %d\n", statePtr->IDEX.readRegA);
	printf("\t\treadRegB %d\n", statePtr->IDEX.readRegB);
    printf("\t\tcontentRegA %d\n", statePtr->IDEX.contentRegA);
	printf("\t\tcontentRegB %d\n", statePtr->IDEX.contentRegB);
	printf("\t\toffset %d\n", statePtr->IDEX.offset);
    printf("\t\tpredictRes %d\n", statePtr->IDEX.predictRes);
    printf("\tEXMEM:\n");
	printf("\t\tinstruction ");
	printInstruction(statePtr->EXMEM.instr);
    printf("\t\treadRegA %d\n", statePtr->EXMEM.readRegA);
	printf("\t\treadRegB %d\n", statePtr->EXMEM.readRegB);
    printf("\t\tcontentRegA %d\n", statePtr->EXMEM.contentRegA);
    printf("\t\tcontentRegB %d\n", statePtr->EXMEM.contentRegB);
	printf("\t\tbranchTarget %d\n", statePtr->EXMEM.branchTarget);
    printf("\t\toffset %d\n", statePtr->EXMEM.offset);
	printf("\t\taluResult %d\n", statePtr->EXMEM.aluResult);
    printf("\t\tpredicted_res %d\n", statePtr->EXMEM.predicted_res);
    printf("\t\tactual_res %d\n", statePtr->EXMEM.actual_res);
    printf("\tMEMWB:\n");
	printf("\t\tinstruction ");
	printInstruction(statePtr->MEMWB.instr);
    printf("\t\treadRegA %d\n", statePtr->MEMWB.readRegA);
	printf("\t\treadRegB %d\n", statePtr->MEMWB.readRegB);
    printf("\t\tcontentRegB %d\n", statePtr->MEMWB.contentRegB);
	printf("\t\twriteData %d\n", statePtr->MEMWB.writeData);
    printf("\t\taluResult %d\n", statePtr->MEMWB.aluResult);
    printf("\tWBEND:\n");
	printf("\t\tinstruction ");
	printInstruction(statePtr->WBEND.instr);
    printf("\t\treadRegA %d\n", statePtr->WBEND.readRegA);
    printf("\t\treadRegB %d\n", statePtr->WBEND.readRegB);
	printf("\t\twriteData %d\n", statePtr->WBEND.writeData);
}

void statePrep(stateType *statePtr){
    statePtr->pc = 0;
    statePtr->cycles = 0;
    statePtr->numMemory = 20;
    statePtr->EXMEM.misPredict = 0;
    
    for(int i = 0; i < NUMREGS; i++){
        statePtr->reg[i] = 0;
    }
    for(int i = 0; i < NUMMEMORY; i++){
        statePtr->instrMem[i] = 0;
        statePtr->dataMem[i] = 0;
    }

    statePtr->IFID.instr = 29360128;
    statePtr->IDEX.instr = 29360128;
    statePtr->EXMEM.instr = 29360128;
    statePtr->MEMWB.instr = 29360128;
    statePtr->WBEND.instr = 29360128;
}

int convertNum(int num){
	// convert a 16-bit number into a 32-bit signed integer
	if (num & (1<<15) ) {
		num -= (1<<16);
	}

	return(num);
}

void forward(stateType *statePtr){

	int regA = statePtr->EXMEM.readRegA;
	int regB = statePtr->EXMEM.readRegB;

	switch(opcode(statePtr->WBEND.instr)){
		case ADD:
            if(regA == field0(statePtr->WBEND.instr)){
				statePtr->EXMEM.contentRegA = statePtr->WBEND.writeData;
			}
			if(regB == field0(statePtr->WBEND.instr)){
				statePtr->EXMEM.contentRegB = statePtr->WBEND.writeData;
			}
            break;
		case NAND:
            if(regA == field0(statePtr->WBEND.instr)){
				statePtr->EXMEM.contentRegA = statePtr->WBEND.writeData;
			}
			if(regB == field0(statePtr->WBEND.instr)){
				statePtr->EXMEM.contentRegB = statePtr->WBEND.writeData;
			}
            break;
		case CMP:
		case MOVL: 
			if( regA == field1(statePtr->WBEND.instr) ) {
				statePtr->EXMEM.contentRegA = statePtr->WBEND.writeData;
			}
			if( regB == field1(statePtr->WBEND.instr) ) {
				statePtr->EXMEM.contentRegB = statePtr->WBEND.writeData;
			}
			break;
        case MOVS: 
		case BEQ:
		case NOOP:
		case HALT:
			break;
	}

	switch(opcode(statePtr->MEMWB.instr)){
		case ADD:
            if(regA == field0(statePtr->MEMWB.instr)){
				statePtr->EXMEM.contentRegA = statePtr->MEMWB.writeData;
			}
			if(regB == field0(statePtr->MEMWB.instr)){
				statePtr->EXMEM.contentRegB = statePtr->MEMWB.writeData;
			}
            break;
		case NAND:
            if(regA == field0(statePtr->MEMWB.instr)){
				statePtr->EXMEM.contentRegA = statePtr->MEMWB.writeData;
			}
			if(regB == field0(statePtr->MEMWB.instr)){
				statePtr->EXMEM.contentRegB = statePtr->MEMWB.writeData;
			}
            break;
		case MOVL: 
			if( regA == field1(statePtr->MEMWB.instr) ) {
				statePtr->EXMEM.contentRegA = statePtr->MEMWB.writeData;
			}
			if( regB == field1(statePtr->MEMWB.instr) ) {
				statePtr->EXMEM.contentRegB = statePtr->MEMWB.writeData;
			}
			break;
		case BEQ:
		case MOVS:
		case NOOP:
		case HALT:
			break;
	}
}

void predictorPrep();

int checkIfExist(int pc);

void updateBTB(stateType *statePtr);

int findBTB(int pc);

int getIndex(int pcAddr);

void updateGHR(int actual_res);

void updatePHT(stateType *statePtr, int actual_res);

int predict(stateType *statePtr);

void printPrediction();

void printGHR();

void printPHT();

void printBTB();







#endif