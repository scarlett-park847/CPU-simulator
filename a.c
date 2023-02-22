/* Assembler for LC */

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MAXLINELENGTH 1000
#define MAXNUMLABELS 65536
#define MAXLABELLENGTH 7 /* includes the null character termination */

#define ADD 0
#define NAND 1
//#define LW 2
#define MOVL 2
#define MOVS 3
#define BEQ 4
#define CMP 5
#define HALT 6
#define NOOP 7

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int translateSymbol(char [MAXNUMLABELS][MAXLABELLENGTH], int labelAddress[], int, char *);
int isNumber(char *);
void testRegArg(char *);
void testAddrArg(char *);
int is_load_word(char *arg);
int translate_load_word();
int translate_save_word();

static char line[MAXLINELENGTH]; // Keep a buffer of the current assembly line for tricky mov.
static char labelArray[MAXNUMLABELS][MAXLABELLENGTH];
static int labelAddress[MAXNUMLABELS];
static int numLabels=0;

int
main(int argc, char *argv[])
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    int address;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
	arg1[MAXLINELENGTH], arg2[MAXLINELENGTH], argTmp[MAXLINELENGTH];
    int i;
    int num;
    int addressField;

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

    /* map symbols to addresses */

    /* assume address start at 0 */
    for (address=0; readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2);
	    address++) {
	/*
	printf("%d: label=%s, opcode=%s, arg0=%s, arg1=%s, arg2=%s\n",
	    address, label, opcode, arg0, arg1, arg2);
	*/

	/* check for illegal opcode */
	if (strcmp(opcode, "add") && strcmp(opcode, "nand") &&
	    strcmp(opcode, "mov") &&
		strcmp(opcode, "je") && strcmp(opcode, "cmp") &&
		strcmp(opcode, "halt") && strcmp(opcode, "noop") &&
		strcmp(opcode, "dd") ) {
	    printf("error: unrecognized opcode %s at address %d\n", opcode,
		    address);
	    exit(1);
	}

	/* check register fields */
	if (!strcmp(opcode, "add") || !strcmp(opcode, "nand") ||
		!strcmp(opcode, "cmp")){
	    testRegArg(arg0);
	    testRegArg(arg1);
	}

    if (!strcmp(opcode, "mov")) {
        if (is_load_word(arg0)) { /* Load Word */
            //testRegArg(arg0);
            //testRegArg(arg1);
            //testAddrArg(arg2);
        } else { /* Store Word */
            //testRegArg(arg0);
            //testAddrArg(arg1);
            //testRegArg(arg2);
        }
    }

	/* check addressField */
	if (!strcmp(opcode, "je")) {
	    testAddrArg(arg0);
	}
	if (!strcmp(opcode, "dd")) {
	    testAddrArg(arg0);
	}

	/* check for enough arguments */
	if (!strcmp(opcode, "je")  && arg0[0] == '\0' ){
	    printf("error at address %d: not enough arguments, for instr: '%s'\n", address, opcode);
	    exit(2);
    }
	if ((!strcmp(opcode, "add") || !strcmp(opcode, "nand") ||
		!strcmp(opcode, "cmp")) && arg1[0] == '\0' ){
	    printf("error at address %d: not enough arguments, for instr: '%s'\n", address, opcode);
	    exit(2);
    }

	if ((!strcmp(opcode, "mov")) && arg2[0] == '\0' ){
	    printf("error at address %d: not enough arguments, for instr: '%s'\n", address, opcode);
        exit(2);
    }

	if (label[0] != '\0') {
	    /* check for labels that are too long */
	    if (strlen(label) >= MAXLABELLENGTH) {
		printf("label too long\n");
		exit(2);
	    }

	    /* make sure label starts with letter */
	    if (! sscanf(label, "%[a-zA-Z]", argTmp) ) {
	        printf("label doesn't start with letter\n");
		exit(2);
	    }

	    /* make sure label consists of only letters and numbers */
	    sscanf(label, "%[a-zA-Z0-9]", argTmp);
	    if (strcmp(argTmp, label)) {
	        printf("label has character other than letters and numbers\n");
		exit(2);
	    }

	    /* look for duplicate label */
	    for (i=0; i<numLabels; i++) {
		if (!strcmp(label, labelArray[i])) {
		    printf("error: duplicate label %s at address %d\n",
			label, address);
		    exit(1);
		}
	    }
	    /* see if there are too many labels */
	    if (numLabels >= MAXNUMLABELS) {
		printf("error: too many labels (label=%s)\n", label);
		exit(2);
	    }

	    strcpy(labelArray[numLabels], label);
	    labelAddress[numLabels++] = address;
	}
    }

    for (i=0; i<numLabels; i++) {
	/* printf("%s = %d\n", labelArray[i], labelAddress[i]); */
    }

    /* now do second pass (print machine code, with symbols filled in as
	addresses) */
    rewind(inFilePtr);
    for (address=0; readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2);
	    address++) {
	if (!strcmp(opcode, "add")) {
	    num = (ADD << 22) | (atoi(arg0) << 19) | (atoi(arg1) << 16);
		    //| atoi(arg2);
	} else if (!strcmp(opcode, "nand")) {
	    num = (NAND << 22) | (atoi(arg0) << 19) | (atoi(arg1) << 16);
		    //| atoi(arg2);
	} else if (!strcmp(opcode, "cmp")) {
	    num = (CMP  << 22) | (atoi(arg0) << 19) | (atoi(arg1) << 16);
		    //| atoi(arg2);
	} else if (!strcmp(opcode, "halt")) {
	    num = (HALT << 22);
	} else if (!strcmp(opcode, "noop")) {
	    num = (NOOP << 22);
	} else if (!strcmp(opcode, "mov") || !strcmp(opcode, "mov")) {
        num = (is_load_word(arg0)) ? translate_load_word()
            : translate_save_word();
	} else if (!strcmp(opcode, "je")) {
        addressField = (isalpha(arg0[0]))
            ? translateSymbol(labelArray, labelAddress, numLabels, arg0)
            : atoi(arg0);

        if (isalpha(arg0[0])) {
		    addressField = addressField-address-1;
	        if (addressField < -32768 || addressField > 32767) {
		        printf("error: offset %d out of range\n", addressField);
		        exit(1);
	        }

	        /* truncate the offset field, in case it's negative */
	        addressField = addressField & 0xFFFF;
        }
		num = (BEQ << 22) |  addressField;
	} else if (!strcmp(opcode, "dd")) {
	    if (!isNumber(arg0)) {
		num = translateSymbol(labelArray, labelAddress, numLabels,
					arg0);
	    } else {
		num = atoi(arg0);
	    }
	}
	printf("(address %3d): %9d (hex 0x%08x)\n", address, num, num);
	fprintf(outFilePtr, "%d\n", num);
    }

    exit(0);
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
int
readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
    char *arg1, char *arg2)
{
    char *ptr = line;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
	/* reached end of file */
        return(0);
    }

    /* check for line too long */
    if (strlen(line) == MAXLINELENGTH-1) {
	printf("error: line too long\n");
	exit(1);
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label)) {
	/* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
        opcode, arg0, arg1, arg2);
    return(1);
}

int
translateSymbol(char labelArray[MAXNUMLABELS][MAXLABELLENGTH],
    int labelAddress[MAXNUMLABELS], int numLabels, char *symbol)
{
    int i;

    /* search through address label table */
    for (i=0; i<numLabels && strcmp(symbol, labelArray[i]); i++) {
    }

    if (i>=numLabels) {
	printf("error: missing label %s\n", symbol);
	exit(1);
    }

    return(labelAddress[i]);
}

int
isNumber(char *string)
{
    /* return 1 if string is a number */
    int i;
    return( (sscanf(string, "%d", &i)) == 1);
}

int is_load_word(char *arg) {
    int num;
    char c;

    return arg[0] != '[';
}

/*
 * Test register argument; make sure it's in range and has no bad characters.
 */
void
testRegArg(char *arg)
{
    int num;
    char c;

    if (atoi(arg) < 0 || atoi(arg) > 7) {
	printf("error: register out of range\n");
	exit(2);
    }
    if (sscanf(arg, "%d%c", &num, &c) != 1) {
	printf("bad character in register argument\n");
	exit(2);
    }
}

/*
 * Test addressField argument.
 */
void
testAddrArg(char *arg)
{
    int num;
    char c;

    /* test numeric addressField */
    if (isNumber(arg)) {
	if (sscanf(arg, "%d%c", &num, &c) != 1) {
	    printf("bad character in addressField\n");
	    exit(2);
	}
    }
}
int translate_load_word() {
    int dest_reg = -1;
    int base_reg = -1;
    int address_field = -1;
    char label[1024];
    char *cptr = line;
    
    while (*cptr != '\0' && !isdigit(*cptr)) { cptr++; } if (*cptr == '\0') { return 0; }
    dest_reg = atoi(cptr);

    while (*cptr != '\0' && *cptr != '[') { cptr++; } if (*cptr == '\0') { return 0; }
    while (*cptr != '\0' && !isdigit(*cptr)) { cptr++; } if (*cptr == '\0') { return 0; }
    base_reg = atoi(cptr);

    while (*cptr != '\0' && *cptr != '+') { cptr++; } if (*cptr == '\0') { return 0; }
    while (*cptr != '\0' && !isdigit(*cptr) && !isalpha(*cptr)) { cptr++; } if (*cptr == '\0') { return 0; }

    if (isalpha(*cptr)) {
        int pos = 0;
        while (*cptr != '\0' && isalpha(*cptr)) { 
            label[pos++] = *cptr;
            cptr++; 
        }
        label[pos] = '\0';
		address_field = translateSymbol(labelArray, labelAddress,
					    numLabels, label);
    } else {
        address_field = atoi(cptr);
    }
    return (MOVL << 22) | (base_reg << 19) |	(dest_reg << 16) | address_field;
}

int translate_save_word() {
    int dest_reg = -1;
    int base_reg = -1;
    int address_field = -1;
    char label[1024];
    char *cptr = line;

    while (*cptr != '\0' && *cptr != '[') { cptr++; } if (*cptr == '\0') { return 0; }
    while (*cptr != '\0' && !isdigit(*cptr)) { cptr++; } if (*cptr == '\0') { return 0; }
    base_reg = atoi(cptr);

    while (*cptr != '\0' && *cptr != '+') { cptr++; } if (*cptr == '\0') { return 0; }
    while (*cptr != '\0' && !isdigit(*cptr) && !isalpha(*cptr)) { cptr++; } if (*cptr == '\0') { return 0; }

    if (isalpha(*cptr)) {
        int pos = 0;
        while (*cptr != '\0' && isalpha(*cptr)) { 
            label[pos++] = *cptr;
            cptr++; 
        }
        label[pos] = '\0';
		address_field = translateSymbol(labelArray, labelAddress,
					    numLabels, label);
    } else {
        address_field = atoi(cptr);
    }
    while (*cptr != '\0' && (isdigit(*cptr) || isalpha(*cptr))) { cptr++; } if (*cptr == '\0') { return 0; }

    while (*cptr != '\0' && !isdigit(*cptr)) { cptr++; } if (*cptr == '\0') { return 0; }
    dest_reg = atoi(cptr);

    return (MOVS << 22) | (base_reg << 19) |	(dest_reg << 16) | address_field;
}
