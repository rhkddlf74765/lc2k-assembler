/* Assembler code fragment for LC-2K */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000
// enum LABEL_TABLE{}

typedef struct ParseResult{
	int opcode;
	int reg0;
	int reg1;
	int destreg;
	int immediate;
}ParseResult;

typedef struct SymbolTable{
	char Label[100];
	int address;
}SymbolTable;

typedef enum {R_TYPE, I_TYPE, J_TYPE, O_TYPE} INSTRUCTION_TYPE;

typedef enum { ADD, NOR, LW, SW, BEQ, JALR, HALT, NOOP, FILL} OPCODE_TABLE;

// Making instruction structure, calculate registers bit, and return it
int get_instruction_bit(ParseResult *instruction);
int type_bit(int instruction_type, ParseResult *instruction);
ParseResult *initialize_instruction(char *label, char *opcode, char *reg0, char *reg1, char *reg2);
int findAddress(char *targetLabel);
int *ParseRegister_Type(char *reg0, char *reg1, char *reg2, int Type);

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);
SymbolTable symbolTable[100];
int symbolTablecount = 0;
int PC = 0;

int main(int argc, char *argv[]) 
{
	int line = 0;
	
	char *inFileString, *outFileString;
	FILE *inFilePtr, *outFilePtr;
	char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH], 
			 arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];
	// fprintf(outFilePtr, "9999\n"); 

	if (argc != 3) {
		printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
				argv[0]);
		exit(1);
	}

	inFileString = argv[1];
	outFileString = argv[2];

	inFilePtr = fopen(inFileString, "r");
	if (inFilePtr == NULL) {
		printf("error in opening %s\n", inFileString);
		exit(1);
	}
	outFilePtr = fopen(outFileString, "w");
	if (outFilePtr == NULL) {
		printf("error in opening %s\n", outFileString);
		exit(1);
	}

	/* use readAndParse to read a label
		read file and make symbol table */
	while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
		if (label[0] != '\0') {
			strcpy(symbolTable[symbolTablecount].Label, label);
			symbolTable[symbolTablecount].address = line;
			symbolTablecount++;
		}
		line++;
	}

	/* TODO: Phase-1 label calculation */

	/* this is how to rewind the file ptr so that you start reading from the
		 beginning of the file */
	rewind(inFilePtr);

	/* TODO: Phase-2 generate machine codes to outfile */

	/* after doing a readAndParse, you may want to do the following to test the
		 opcode */
	while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)){
		ParseResult *instruction = initialize_instruction(label, opcode, arg0, arg1, arg2);
		int instruction_bit = get_instruction_bit(instruction);
		fprintf(outFilePtr, "%d\n", instruction_bit);
		PC++;
		free(instruction);
	}
	// if (!strcmp(opcode, "add")) {
	// 	/* do whatever you need to do for opcode "add" */
	// }
	if (inFilePtr) {
		fclose(inFilePtr);
	}
	if (outFilePtr) {
		fclose(outFilePtr);
	}
	return(0);
}

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
		char *arg1, char *arg2)
{
	char line[MAXLINELENGTH];
	char *ptr = line;

	/* delete prior values */
	label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

	/* read the line from the assembly-language file */
	if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
		/* reached end of file */
		return(0);
	}

	/* check for line too long (by looking for a \n) */
	if (strchr(line, '\n') == NULL) {
		/* line too long */
		printf("error: line too long\n");
		exit(1);
	}

	/* is there a label? */
	ptr = line;
	if (sscanf(ptr, "%[^\t\n\r ]", label)) {
		/* successfully read label; advance pointer over the label */
		ptr += strlen(label);
	}

	/*
	 * Parse the rest of the line.  Would be nice to have real regular
	 * expressions, but scanf will suffice.
	 */
	sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%"
			"[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]", opcode, arg0, arg1, arg2);
	return(1);
}

int isNumber(char *string)
{
	/* return 1 if string is a number */
	int i;
	return( (sscanf(string, "%d", &i)) == 1);
}

int get_instruction_bit(ParseResult *instruction){
	if (0 <= instruction->opcode && instruction->opcode <= 1 ) 	return type_bit(R_TYPE, instruction);
	else if (instruction->opcode <= 4) 							return type_bit(I_TYPE, instruction);
	else if (instruction->opcode <= 6) 							return type_bit(J_TYPE, instruction);
	else if (instruction->opcode == 7) 							return type_bit(O_TYPE, instruction);
	else 														return instruction->immediate;
}

int type_bit(int instruction_type, ParseResult *instruction){
	if (instruction_type == R_TYPE){
		return (instruction->opcode << 22) | (instruction->reg0 << 19) | (instruction->reg1 << 16) | instruction->destreg;
	}
	else if (instruction_type == I_TYPE){
		return (instruction->opcode << 22) | (instruction->reg0 << 19) | (instruction->reg1 << 16) | instruction->immediate;
	}
	else if (instruction_type == J_TYPE){
		return (instruction->opcode << 22) | instruction->immediate; // need to modify
	}
	else if (instruction_type == O_TYPE){
		return  instruction->opcode << 22;
	}
}

ParseResult *initialize_instruction(char *label, char *opcode, char *reg0, char *reg1, char *reg2){
	ParseResult *instruction = (ParseResult *)malloc(sizeof(ParseResult));
	int *regarr = NULL;
    if (instruction == NULL) {
		printf("error: malloc failed\n");
		exit(1);
	}

	if 	(!strcmp(opcode, "add")){
        regarr = ParseRegister_Type(reg0, reg1, reg2, R_TYPE);
		*instruction = (ParseResult){.opcode = ADD, .reg0 = regarr[0], .destreg = regarr[1], .reg1 = regarr[2]};
    }
	else if (!strcmp(opcode, "nor")){
        regarr = ParseRegister_Type(reg0, reg1, reg2, R_TYPE);
		*instruction = (ParseResult){.opcode = ADD, .reg0 = regarr[0], .destreg = regarr[1], .reg1 = regarr[2]};
    }
	else if (!strcmp(opcode, "lw")){ 
        regarr = ParseRegister_Type(reg0, reg1, reg2, I_TYPE);
		*instruction = (ParseResult){.opcode = LW, .reg0 = regarr[0], .reg1 = regarr[1], .immediate = regarr[2]};
    }
	else if (!strcmp(opcode, "sw")){
        regarr = ParseRegister_Type(reg0, reg1, reg2, I_TYPE);
		*instruction = (ParseResult){.opcode = SW, .reg0 = regarr[0], .reg1 = regarr[1], .immediate = regarr[2]};
    }
	else if (!strcmp(opcode, "beq")) {
        regarr = ParseRegister_Type(reg0, reg1, reg2, I_TYPE);
        // Checking whether the immediate is a number or a label is necessary
        if (!isNumber(reg2)) {
            regarr[2] = findAddress(reg2) - (PC + 1);
            regarr[2] = regarr[2] & 0xFFFF; // Masking to 16 bits
        } 
        *instruction = (ParseResult){.opcode = BEQ, .reg0 = regarr[0], .reg1 = regarr[1], .immediate = regarr[2]};
    }
	else if (!strcmp(opcode, "jalr")){
        regarr = ParseRegister_Type(reg0, reg1, NULL, J_TYPE);
		*instruction = (ParseResult){.opcode = JALR, .reg0 = regarr[0], .reg1 = regarr[1]};
    }
	else if (!strcmp(opcode, "halt"))
		*instruction = (ParseResult){.opcode = HALT};
	else if (!strcmp(opcode, "noop"))
		*instruction = (ParseResult){.opcode = NOOP};
	else if (!strcmp(opcode, ".fill")) {
        instruction->opcode = FILL;
        if (isNumber(reg0)) instruction->immediate = atoi(reg0);
        else instruction->immediate = findAddress(reg0);
        printf("what the fuck is that %d\n",instruction->immediate);
	}
	else {
		printf("error: unknown opcode %s\n", opcode);
		exit(1);
	}
    if (regarr != NULL)
        free(regarr);
	return instruction;
}

int findAddress(char *targetLabel){
    for (int i = 0; i < symbolTablecount; i++){
        if (!strcmp(symbolTable[i].Label, targetLabel)){
            return symbolTable[i].address;
        }
    }
    printf("error: unknown label %s\n", targetLabel);
    exit(1);
}

int *ParseRegister_Type(char *reg0, char *reg1, char *reg2, int Type){
    char *regarr[3] = {reg0, reg1, reg2};
    int *regarr_int = (int *)malloc(3 * sizeof(int)); 

    if (regarr_int == NULL) {
        printf("malloc failed\n");
        exit(1);
    }

    if (Type == R_TYPE || Type == I_TYPE){       
        for (int i = 0 ; i < 3; i++){
            if (isNumber(regarr[i])) regarr_int[i] = atoi(regarr[i]);
            else regarr_int[i] = findAddress(regarr[i]);
        }
    }
    else if (Type == J_TYPE){
        for (int i = 0 ; i < 2 ; i++){
            if (isNumber(regarr[i])) regarr_int[i] = atoi(regarr[i]);
            else regarr_int[i] = findAddress(regarr[i]);
        }
    }
    return regarr_int;
}