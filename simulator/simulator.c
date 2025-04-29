/* LC-2K Instruction-level simulator */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000 
typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

void printState(stateType *);
int convertNum(int num);

int main(int argc, char *argv[])
{
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }

    /* read in the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL; state.numMemory++) {

        if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }

    int instructionCount = 0;

    while (1)
    {
        printState(&state);

        int instruction = state.mem[state.pc];
        int opcode = (instruction >> 22) & 0x7;
        int regA = (instruction >> 19) & 0x7;
        int regB = (instruction >> 16) & 0x7;
        int destReg, offset;

        state.pc++; // 기본적으로 PC는 1 증가 (beq, jalr은 다를 수 있음)

        switch (opcode)
        {
        case 0: // add
            destReg = instruction & 0x7;
            state.reg[destReg] = state.reg[regA] + state.reg[regB];
            break;
        case 1: // nor
            destReg = instruction & 0x7;
            state.reg[destReg] = ~(state.reg[regA] | state.reg[regB]);
            break;
        case 2: // lw
            offset = convertNum(instruction & 0xFFFF);
            state.reg[regB] = state.mem[state.reg[regA] + offset];
            break;
        case 3: // sw
            offset = convertNum(instruction & 0xFFFF);
            state.mem[state.reg[regA] + offset] = state.reg[regB];
            break;
        case 4: // beq
            offset = convertNum(instruction & 0xFFFF);
            if (state.reg[regA] == state.reg[regB])
            {
                state.pc += offset;
            }
            break;
        case 5: // jalr
        {
            int temp = state.pc;
            state.pc = state.reg[regA];
            state.reg[regB] = temp;
        }
        break;
        case 6: // halt
            printf("machine halted\n");
            printf("total of %d instructions executed\n", instructionCount + 1); // halt도 실행 1번 포함
            printf("final state of machine:\n");
            printState(&state);
            exit(0);
            break;
        case 7: // noop
            break;
        default:
            printf("error: illegal opcode %d\n", opcode);
            exit(1);
        }

        instructionCount++;
    }
        /* TODO: */
    if (filePtr)
    {
        fclose(filePtr);
    }
    return(0);
}

void printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i = 0; i < statePtr->numMemory; i++) {
        printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
    for (i = 0; i < NUMREGS; i++) {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("end state\n");
}

int convertNum(int num)
{
	/* convert a 16-bit number into a 32-bit Linux integer */
	if (num & (1 << 15)) {
		num -= (1 << 16);
	}
	return (num);
}
