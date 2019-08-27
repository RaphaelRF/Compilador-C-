#include "globals.h"
#include "symtab.h"
#include "cgen.h"
#include "AssemblyGEN.h"

const char * opcodes[] =  { "nop", "halt", "add", "addi", "sub", "mult", "divi", "mod", "and", "or", "not", "xor", "slt", "sgt", "sle", "sge",
                            "shl", "shr", "move", "ldi", "beq", "bne", "jmp", "in", "out", "str", "load", "jr" };

const char * opcodeBins[] =  {  "011011", "011100", "000000", "000001", "000010", "001100", "100001", "000100", "010100", "010101", "100001", "010110", "010010",
                                "011110", "011111", "100000", "011001", "011010", "011101", "010000", "000101", "000110", "001101", "010111", "011000", "010001",
                                "001111", "001110" };

const char * regBins[] = {  "00000", "00001", "00010", "00011", "00100", "00101", "00110", "00111", "01000", "01001", "01010", "01011", "01100", "01101", "01110",
                            "01111", "10000", "10001", "10010", "10011", "10100", "10101", "10110", "10111", "11000", "11001", "11010", "11011", "11100", "11101",
                            "11110", "11111" };


char * getImediate (int im, int size) {
    int i = 0;
    char * bin = (char *) malloc(size + 2);
    size --;
    for (unsigned bit = 1u << size; bit != 0; bit >>= 1) {
        bin[i++] = (im & bit) ? '1' : '0';
    }
    bin[i] = '\0';
    return bin;
}

char * assembly2binary (Instruction i) {
    char * bin = (char *) malloc((32 + 4 + 2) * sizeof(char));

    if (i.format == format1) {
        sprintf(bin, "%s_%s_%s_%s_%s", opcodeBins[i.opcode], regBins[i.reg2], regBins[i.reg3], regBins[i.reg1], "00000000000");
    }
    else if (i.format == format2) {
      if(i.opcode == move){
          sprintf(bin, "%s_%s_%s_%s", opcodeBins[i.opcode], regBins[i.reg2], regBins[i.reg1], "0000000000000000");
      }

      else if(i.opcode == str || i.opcode == load || i.opcode == addi){
            sprintf(bin, "%s_%s_%s_%s", opcodeBins[i.opcode], regBins[i.reg2], regBins[i.reg1], getImediate(i.im, 16));
      }

      else{
        sprintf(bin, "%s_%s_%s_%s", opcodeBins[i.opcode], regBins[i.reg1], regBins[i.reg2], getImediate(i.im, 16));
      }
    }
    else if (i.format == format3) {

        if(i.opcode == ldi){
            sprintf(bin, "%s_%s_%s_%s", opcodeBins[i.opcode], "00000", regBins[i.reg1], getImediate(i.im, 16));
        }

        else if(i.opcode == in){
            sprintf(bin, "%s_%s_%s_%s", opcodeBins[i.opcode], regBins[i.reg1], "00000", "0000000000000000");
        }
        else{
            sprintf(bin, "%s_%s_%s_%s", opcodeBins[i.opcode], regBins[i.reg1], "00000", getImediate(i.im, 16));
        }
    }
    else {
        sprintf(bin, "%s_%s", opcodeBins[i.opcode], getImediate(i.im, 26));
    }

    return bin;
}

void generateBinary (AssemblyCode head, int size) {
    AssemblyCode a = head;
    FILE * c = code;
    char * bin;

    
    printf("\n\n----------CODIGO BINARIO----------\n\n");

    while (a != NULL) {
        if (a->kind == instr) {
            fprintf(c, "\tmem[%d]\t=\t32'b", a->lineno);
            printf("%d:\t", a->lineno);
            bin = assembly2binary(a->line.instruction);
            fprintf(c, "%s;\n", bin);
            printf("%s\t// %s\n", bin, opcodes[a->line.instruction.opcode]);
        }
        else {
            printf("// %s\n", a->line.label);
        }
        a = a->next;
    }


}
