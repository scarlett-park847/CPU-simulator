#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

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
    printf("\t\tcmpFlag %d\n", statePtr->EXMEM.cmpFlag);
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
    statePtr->EXMEM.cmpFlag = 0;
    
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



int main(){
    fstream newfile;
    stateType state;
    stateType newState;

    statePrep(&state);
    statePrep(&newState);

    newfile.open("asm&mc/DataFowardExample.mc",ios::in);  

    if (!newfile.is_open()){ 
        cout << "file cannot be opend" << endl;
         exit(EXIT_FAILURE);
    }else{
        string curr;
        int inst;
        int i = 0;
        while(getline(newfile, curr)){ 
            cout << curr << endl;
            inst = stoi(curr);
            state.instrMem[i] = inst;
            state.dataMem[i] = inst;
            i++;
        }
        newfile.close();
    }

    while (1) {
        

        /* --------------------- WB stage --------------------- */
        if(!(opcode(state.WBEND.instr) == HALT || opcode(state.WBEND.instr) == NOOP)){
            if(opcode(state.WBEND.instr) == ADD || opcode(state.WBEND.instr) == NAND){
                state.reg[state.WBEND.readRegA] = state.WBEND.writeData;
            }else if(opcode(state.WBEND.instr) == MOVL){
                state.reg[state.WBEND.readRegB] = state.WBEND.writeData;
            }
        }
        
        /* --------------------- MEM stage --------------------- */
        if(!(opcode(state.MEMWB.instr) == HALT || opcode(state.MEMWB.instr) == NOOP)){
            if(opcode(state.MEMWB.instr) == ADD || opcode(state.MEMWB.instr) == NAND){
                state.MEMWB.writeData = state.MEMWB.aluResult;
            }else if(opcode(state.MEMWB.instr) == MOVL){
                state.MEMWB.writeData = state.dataMem[state.MEMWB.aluResult];
            }else if(opcode(state.MEMWB.instr) == MOVS){
                state.dataMem[state.MEMWB.aluResult] = state.MEMWB.contentRegB;
            }
        }
        newState.WBEND.instr = state.MEMWB.instr;
        newState.WBEND.readRegB = state.MEMWB.readRegB;
        newState.WBEND.readRegA = state.MEMWB.readRegA;
        newState.WBEND.writeData = state.MEMWB.writeData;
        
        /* --------------------- EX stage --------------------- */
        if(!(opcode(state.EXMEM.instr) == HALT || opcode(state.EXMEM.instr) == NOOP)){
            
            forward(&state);

            if(opcode(state.EXMEM.instr) == ADD){
                state.EXMEM.aluResult = state.EXMEM.contentRegA + state.EXMEM.contentRegB;
            }else if(opcode(state.EXMEM.instr) == NAND){
                state.EXMEM.aluResult = ~(state.EXMEM.contentRegB & state.EXMEM.contentRegB);
            }else if(opcode(state.EXMEM.instr) == BEQ){
                if(state.EXMEM.contentRegA == state.EXMEM.contentRegB){
                    newState.pc = state.EXMEM.branchTarget;
                    state.EXMEM.cmpFlag = 1;
                    state.IDEX.instr = 29360128;
                    state.IFID.instr = 29360128;
                }
            }else if(opcode(state.EXMEM.instr) == MOVL){
                state.EXMEM.aluResult = state.EXMEM.contentRegA + state.EXMEM.offset; // regB = Memory[regA + offset]
            }else if(opcode(state.EXMEM.instr) == MOVS){
                state.EXMEM.aluResult = state.EXMEM.contentRegA + state.EXMEM.offset; // Memory[regA + offset] = regB
            }
        }
        
        newState.MEMWB.instr = state.EXMEM.instr;
        newState.MEMWB.readRegB = state.EXMEM.readRegB;
        newState.MEMWB.readRegA = state.EXMEM.readRegA;
        newState.MEMWB.aluResult = state.EXMEM.aluResult;
        newState.MEMWB.contentRegB = state.EXMEM.contentRegB;

        /* --------------------- ID stage --------------------- */
        if(!(opcode(state.IDEX.instr) == HALT || opcode(state.IDEX.instr) == NOOP)){
            
            if(opcode(state.IDEX.instr) == ADD || opcode(state.IDEX.instr) == NAND || opcode(state.IDEX.instr) == BEQ || opcode(state.IDEX.instr) == MOVL || opcode(state.IDEX.instr) == MOVS){
                state.IDEX.readRegA = field0(state.IDEX.instr); 
                state.IDEX.readRegB = field1(state.IDEX.instr);
                state.IDEX.offset = field2(state.IDEX.instr);
                state.IDEX.offset = convertNum(state.IDEX.offset);
                state.IDEX.contentRegA = state.reg[state.IDEX.readRegA];
                state.IDEX.contentRegB = state.reg[state.IDEX.readRegB];
            }
        }
        newState.EXMEM.instr = state.IDEX.instr;
        newState.EXMEM.contentRegA = state.IDEX.contentRegA;
        newState.EXMEM.contentRegB = state.IDEX.contentRegB;
        newState.EXMEM.readRegB = state.IDEX.readRegB;
        newState.EXMEM.readRegA = state.IDEX.readRegA;
        newState.EXMEM.offset = state.IDEX.offset;
        newState.EXMEM.branchTarget = (state.IDEX.pcPlus1) + state.IDEX.offset;

        /* --------------------- IF stage --------------------- */ 
        if(state.EXMEM.cmpFlag  != 1){
            state.IFID.instr = state.instrMem[state.pc];
        }
        
        cout << "state.IFID.instr: " << state.IFID.instr << endl;
        state.IFID.pcPlus1 = (state.pc) + 1;

        if(state.EXMEM.cmpFlag != 1){
            newState.pc = state.pc + 1;
        }
        
        newState.IDEX.instr = state.IFID.instr;
        newState.IDEX.pcPlus1 = state.IFID.pcPlus1;
        // memUpdate(&newState, &state);
        memcpy(newState.instrMem, state.instrMem, NUMMEMORY*sizeof(int));
        memcpy(newState.dataMem, state.dataMem, NUMMEMORY*sizeof(int));
        memcpy(newState.reg, state.reg, NUMREGS*sizeof(int));

        printState(&state);

        if (opcode(state.MEMWB.instr) == HALT) {
            printf("machine halted\n");
            printf("total of %d cycles executed\n", state.cycles);
            exit(0);
	    }

        state = newState;
        state.EXMEM.cmpFlag = 0;
        state.cycles++; /* this is the last statement before end of the loop.
                    It marks the end of the cycle and updates the
                    current state with the values calculated in this
                    cycle */
        
    }
   

}