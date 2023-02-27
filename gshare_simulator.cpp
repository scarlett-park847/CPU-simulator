#include "gshare_simulator.h"
#include <stdio.h>
#include <fstream>

using namespace std;

// vector <int> PHT; // pattern history table - 4 entries
// int GHR; // global history register - 2bits
void predictorPrep(){

    //initialize PHT with weakly NOT-TAKEN.
    for(int i = 0; i < PHT_size; i++){
        PHT.push_back(1);
    }
    //initialize GHR
    GHR = -1;
    CYCLES = 0; 
    FETCHED = 0; 
    RETIRED = 0; 
    BRANCHES = 0; 
    MISPRED = 0;
}


int checkIfExist(int pc){
    int res = -1;
    for(int i = 0; i < BTB.size(); i++){
        if(pc == BTB[i].first){
            res = i;
            break;
        }
    }
    return res;
}

void updateBTB(stateType *statePtr){
    if(statePtr->EXMEM.actual_res == 1){
        int pc = statePtr->EXMEM.pcPlus1 -1;
        int target = statePtr->EXMEM.branchTarget;
        if(checkIfExist(pc) == -1){
            BTB.push_back(make_pair(pc,target));
        }
    }
    if(BTB.size() > BTB_size){
        BTB.erase(BTB.begin());
    }
}

int findBTB(int pc){
    int target = -1;

    if(BTB.empty()){
        return -1;
    }else{
        for(int i = 0; i < BTB.size(); i++){
            if(pc == BTB[i].first){
                target = BTB[i].second;
                break;
            }
        }
    }
    return target;
}
int getIndex(int pcAddr){
    
    pcAddr = pcAddr & 15;
    int globHistory = GHR & 15;

    return (pcAddr ^ globHistory);
}


void updateGHR(int actual_res){
    if(GHR == -1){
        GHR = actual_res;
        return;
    }else{
        GHR = GHR << 1;
		if(actual_res == 1)
			GHR |= 1U;
		else if(actual_res == 0)
			GHR &= ~(1U);
    }
}

void updatePHT(stateType *statePtr, int predicted_res, int actual_res){
    int pcAddr = statePtr->EXMEM.pcPlus1 -1;
    int index = getIndex(pcAddr);

    if(actual_res == 1){
		if(PHT.at(index) < 3)
			PHT.at(index) = PHT.at(index) + 1;
		if(predicted_res == 0)
			MISPRED++;
	}		
	else if(actual_res == 0){	
		if(PHT.at(index) > 0)
			PHT.at(index) = PHT.at(index) - 1;
		if(predicted_res == 1)
			MISPRED++;
	}
	
	updateGHR(actual_res);
}

int predict(stateType *statePtr){
    int prediction_res = 1;
    if(GHR == -1){
        return prediction_res;
    }else{
        int pcAddr = statePtr->IDEX.pcPlus1 -1;
        int index = getIndex(pcAddr);

        prediction_res = PHT.at(index);
    }
    
    return prediction_res;
}

void printPrediction(){

    printf("CYCLES:");
    printf("\t %d\n", CYCLES);
    printf("FETCHED:");
    printf("\t %d\n", FETCHED);
    printf("RETIRED:");
    printf("\t %d\n", RETIRED);
    printf("BRANCHES:");
    printf("\t %d\n", BRANCHES);
    printf("MISPRED:");
    printf("\t %d\n", MISPRED);

}

void printGHR(){
    int globHistory = GHR & 15;

    int binaryNum[32];
 
    int i = 0;
    while (globHistory > 0) {
 
        // storing remainder in binary array
        binaryNum[i] = globHistory % 2;
        globHistory = globHistory / 2;
        i++;
    }
 
    // printing binary array in reverse order
    cout << "Gloabal History Register: ";
    for (int j = i - 1; j >= 0; j--)
        cout << binaryNum[j];

    cout << endl;
}

void printPHT(){
    printf("\nPattern History Table\n");

    for(int i = 0; i < PHT.size(); i++){
        printf("\tIndex: %d\t", i);
        printf("Pattern: %d\n", PHT[i]);
    }
}

void printBTB(){
    printf("\n@@@\nBranch Target Buffer\n");

    for(int i = 0; i < BTB.size(); i++){
        printf("\tTag: %d\t", BTB[i].first);
        printf("BranchTarget: %d\n", BTB[i].second);
    }
}

int main(){
    fstream newfile;
    stateType state;
    stateType newState;

    statePrep(&state);
    statePrep(&newState);
    predictorPrep();

    newfile.open("asm&mc/BranchExample.mc",ios::in);  

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
    int j = 0;
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
                    state.EXMEM.actual_res = 1;
                    if(state.EXMEM.predicted_res <= 1){
                        state.EXMEM.misPredict = 1;
                        cout << "actual Res: " << state.EXMEM.actual_res << " predicted_res: " << state.EXMEM.predicted_res << endl;
                        state.IDEX.instr = 29360128;
                        state.IFID.instr = 29360128;
                        newState.pc = state.EXMEM.branchTarget;
                    }else if(findBTB(state.EXMEM.pcPlus1-1) == -1){
                        cout << "predicted correctly, but there was no BTB record" << endl;
                        state.IDEX.instr = 29360128;
                        state.IFID.instr = 29360128;
                        newState.pc = state.EXMEM.branchTarget;
                    }
                }else{
                    state.EXMEM.actual_res = 0;
                    if(state.EXMEM.predicted_res >= 2){
                        state.EXMEM.misPredict = 1;
                        cout << "actual Res: " << state.EXMEM.actual_res << " predicted_res: " << state.EXMEM.predicted_res << endl;
                        state.IDEX.instr = 29360128;
                        state.IFID.instr = 29360128;
                        newState.pc = state.EXMEM.pcPlus1;
                    }
                }
                updatePHT(&state, state.EXMEM.predicted_res, state.EXMEM.actual_res);
                updateBTB(&state);
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
            
            if(opcode(state.IDEX.instr) == ADD || opcode(state.IDEX.instr) == NAND || opcode(state.IDEX.instr) == MOVL || opcode(state.IDEX.instr) == MOVS || opcode(state.IDEX.instr) == BEQ){
                state.IDEX.readRegA = field0(state.IDEX.instr); 
                state.IDEX.readRegB = field1(state.IDEX.instr);
                state.IDEX.offset = field2(state.IDEX.instr);
                state.IDEX.offset = convertNum(state.IDEX.offset);
                state.IDEX.contentRegA = state.reg[state.IDEX.readRegA];
                state.IDEX.contentRegB = state.reg[state.IDEX.readRegB];
            }
            
            if(opcode(state.IDEX.instr) == BEQ){
                state.IDEX.predictRes = predict(&state);
                if(state.IDEX.predictRes >= 2){
                    state.IDEX.targetAddr = findBTB(state.IDEX.pcPlus1 -1);
                    if(state.IDEX.targetAddr != -1){
                        state.pc = state.IDEX.targetAddr;
                    }
                }
            }
        }
        newState.EXMEM.instr = state.IDEX.instr;
        newState.EXMEM.pcPlus1 = state.IDEX.pcPlus1;
        newState.EXMEM.contentRegA = state.IDEX.contentRegA;
        newState.EXMEM.contentRegB = state.IDEX.contentRegB;
        newState.EXMEM.readRegB = state.IDEX.readRegB;
        newState.EXMEM.readRegA = state.IDEX.readRegA;
        newState.EXMEM.offset = state.IDEX.offset;
        newState.EXMEM.branchTarget = (state.IDEX.pcPlus1) + state.IDEX.offset;
        newState.EXMEM.predicted_res = state.IDEX.predictRes;

        /* --------------------- IF stage --------------------- */ 
        
        if(state.EXMEM.misPredict != 1){
            state.IFID.instr = state.instrMem[state.pc];
            newState.pc = state.pc + 1;
        }
        
        
        cout << "state.IFID.instr: " << state.IFID.instr << endl;
        state.IFID.pcPlus1 = (state.pc) + 1;

        newState.IDEX.instr = state.IFID.instr;
        newState.IDEX.pcPlus1 = state.IFID.pcPlus1;
        memcpy(newState.instrMem, state.instrMem, NUMMEMORY*sizeof(int));
        memcpy(newState.dataMem, state.dataMem, NUMMEMORY*sizeof(int));
        memcpy(newState.reg, state.reg, NUMREGS*sizeof(int));

        printGHR();
        printPHT();
        printBTB();
        printState(&state);

        if (opcode(state.MEMWB.instr) == HALT) {
            printf("machine halted\n");
            printf("total of %d cycles executed\n", state.cycles);
            exit(0);
	    }

        newState.cycles = state.cycles + 1;
        state = newState;
        
        state.EXMEM.misPredict = 0;
        // state.IDEX.predictRes = 0;
        // state.IDEX.targetAddr = -1;
        j++;

    }
   

}