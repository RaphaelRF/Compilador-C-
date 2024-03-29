#include "globals.h"
#include "symtab.h"
#include "cgen.h"
#include "AssemblyGEN.h"

AssemblyCode codehead = NULL;
FunList funlisthead = NULL;

int line = 0;
int nscopes = 0;
int curmemloc = 0;
int curparam = 0;
int curarg = 0;
int narg = 0;
int jmpmain = 0;

const char *InstrNames[] = {"nop", "halt", "add", "addi", "sub", "mult", "divi", "mod", "and", "or", "not", "xor", "slt", "sgt", "sle", "sge",
                            "shl", "shr", "move", "ldi", "beq", "bne", "jmp", "in", "out", "str", "load", "jr"};

const char *regNames[] = {"$zero", "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9", "$t10", "$t11", "$t12", "$t13", "$t14",
                          "$t15", "$a0", "$a1", "$a2", "$a3", "$a4", "$a5", "$a6", "$a7", "$a8", "$a9", "$sp", "$gp", "$ra", "$ret", "$jmp"};

void insertFun(char *id)
{
    FunList new = (FunList)malloc(sizeof(struct FunListRec));
    new->id = (char *)malloc(strlen(id) * sizeof(char));
    strcpy(new->id, id);
    new->size = 0;
    new->memloc = curmemloc;
    new->next = NULL;
    if (funlisthead == NULL)
    {
        funlisthead = new;
    }
    else
    {
        FunList f = funlisthead;
        while (f->next != NULL)
            f = f->next;
        f->next = new;

    }
    nscopes++;
}

void insertVar(char *scope, char *id, int size, VarKind kind)
{

    FunList f = funlisthead;

    while (f != NULL && strcmp(f->id, scope) != 0)
        f = f->next;

    if (f == NULL)
    {

        insertFun(scope);
        f = funlisthead;
        while (f != NULL && strcmp(f->id, scope) != 0)
            f = f->next;
    }
    VarList new = (VarList)malloc(sizeof(struct VarListRec));
    new->id = (char *)malloc(strlen(id) * sizeof(char));
    strcpy(new->id, id);
    new->size = size;
    new->memloc = f->size;
    curmemloc = curmemloc + size;
    new->kind = kind;
    new->next = NULL;
    if (f->vars == NULL)
    {
        f->vars = new;
    }
    else
    {

        VarList v = f->vars;
        while (v->next != NULL)
            v = v->next;
        v->next = new;
    }

    f->size = f->size + size;
}

void insertLabel(char *label)
{
    AssemblyCode new = (AssemblyCode)malloc(sizeof(struct AssemblyCodeRec));
    new->lineno = line;
    new->kind = lbl;
    new->line.label = (char *)malloc(strlen(label) * sizeof(char));
    strcpy(new->line.label, label);
    new->next = NULL;
    if (codehead == NULL)
    {
        codehead = new;
    }
    else
    {
        AssemblyCode a = codehead;
        while (a->next != NULL)
            a = a->next;
        a->next = new;
    }
}

void insertInstruction(InstrFormat format, InstrKind opcode, Reg reg1, Reg reg2, Reg reg3, int im, char *imlbl)
{
    Instruction i;
    i.format = format;
    i.opcode = opcode;
    i.reg1 = reg1;
    i.reg2 = reg2;
    i.reg3 = reg3;
    i.im = im;
    if (imlbl != NULL)
    {
        i.imlbl = (char *)malloc(strlen(imlbl) * sizeof(char));
        strcpy(i.imlbl, imlbl);
    }
    AssemblyCode new = (AssemblyCode)malloc(sizeof(struct AssemblyCodeRec));
    new->lineno = line;
    new->kind = instr;
    new->line.instruction = i;
    new->next = NULL;
    if (codehead == NULL)
    {
        codehead = new;
    }
    else
    {
        AssemblyCode a = codehead;
        while (a->next != NULL)
            a = a->next;
        a->next = new;
    }
    line++;
}

void instructionFormat1(InstrKind opcode, Reg reg1, Reg reg2, Reg reg3)
{
    insertInstruction(format1, opcode, reg1, reg2, reg3, 0, NULL);
}

void instructionFormat2(InstrKind opcode, Reg reg1, Reg reg2, int im, char *imlbl)
{
    insertInstruction(format2, opcode, reg1, reg2, $zero, im, imlbl);
}

void instructionFormat3(InstrKind opcode, Reg reg1, int im, char *imlbl)
{
    insertInstruction(format3, opcode, reg1, $zero, $zero, im, imlbl);
}

void instructionFormat4(InstrKind opcode, int im, char *imlbl)
{
    insertInstruction(format4, opcode, $zero, $zero, $zero, im, imlbl);
}

Reg getParamReg()
{
    return (Reg)1 + nregtemp + curparam;
}

Reg getArgReg()
{
    return (Reg)1 + nregtemp + curarg;
}

Reg getReg(char *regName)
{
    for (int i = 0; i < nregisters; i++)
    {
        if (strcmp(regName, regNames[i]) == 0)
            return (Reg)i;
    }
    return $zero;
}

int getLabelLine(char *label)
{
    AssemblyCode a = codehead;
    while (a->next != NULL)
    {
        if (a->kind == lbl && strcmp(a->line.label, label) == 0)
            return a->lineno;
        a = a->next;
    }
    return -1;
}

VarKind checkType(QuadList l)
{
    QuadList aux = l;
    Quad q = aux->quad;
    aux = aux->next;
    while (aux != NULL && aux->quad.op != opEND)
    {
        if (aux->quad.op == opVEC && strcmp(aux->quad.addr2.contents.var.name, q.addr1.contents.var.name) == 0)
            return address;
        aux = aux->next;
    }
    return simple;
}

int getVarMemLoc(char *id, char *scope)
{

    FunList f = funlisthead;
    while (f != NULL && strcmp(f->id, scope) != 0)
        f = f->next;
    if (f == NULL)
        return -1;
    VarList v = f->vars;

    while (v != NULL)
    {

        if (strcmp(v->id, id) == 0)
            return v->memloc;
        v = v->next;
    }
    return -1;
}

VarKind getVarKind(char *id, char *scope)
{
    FunList f = funlisthead;
    while (f != NULL && strcmp(f->id, scope) != 0)
        f = f->next;
    if (f == NULL)
    {
        return simple;
    }
    VarList v = f->vars;
    while (v != NULL)
    {
        if (strcmp(v->id, id) == 0)
            return v->kind;
        v = v->next;
    }
    return simple;
}

int getFunSize(char *id)
{
    FunList f = funlisthead;
    while (f != NULL && strcmp(f->id, id) != 0)
        f = f->next;
    if (f == NULL)
        return -1;
    return f->size;
}

void initCode(QuadList head)
{
    QuadList l = head;
    Quad q;
    instructionFormat3(ldi, $sp, sploc, NULL);
    instructionFormat3(ldi, $gp, gploc, NULL);
    instructionFormat3(ldi, $ra, raloc, NULL);
    insertFun("Global");
}

void generateInstruction(QuadList l)
{
    Quad q;
    Address a1, a2, a3;
    int aux;
    VarKind v;

    while (l != NULL)
    {

        q = l->quad;
        a1 = q.addr1;
        a2 = q.addr2;
        a3 = q.addr3;

        switch (q.op)
        {

        case opADD:
            instructionFormat1(add, getReg(a1.contents.var.name), getReg(a2.contents.var.name), getReg(a3.contents.var.name));
            break;

        case opSUB:
            instructionFormat1(sub, getReg(a1.contents.var.name), getReg(a2.contents.var.name), getReg(a3.contents.var.name));
            break;

        case opMULT:
            instructionFormat1(mult, getReg(a1.contents.var.name), getReg(a2.contents.var.name), getReg(a3.contents.var.name));
            break;

        case opDIV:
            instructionFormat1(divi, getReg(a1.contents.var.name), getReg(a2.contents.var.name), getReg(a3.contents.var.name));
            break;

        case opLT:
            instructionFormat1(slt, getReg(a1.contents.var.name), getReg(a2.contents.var.name), getReg(a3.contents.var.name));
            break;

        case opLET:
            instructionFormat1(sle, getReg(a1.contents.var.name), getReg(a2.contents.var.name), getReg(a3.contents.var.name));
            break;

        case opGT:
            instructionFormat1(sgt, getReg(a1.contents.var.name), getReg(a2.contents.var.name), getReg(a3.contents.var.name));
            break;

        case opGET:
            instructionFormat1(sge, getReg(a1.contents.var.name), getReg(a2.contents.var.name), getReg(a3.contents.var.name));
            break;

        case opAND:

            instructionFormat1(and, getReg(a1.contents.var.name), getReg(a2.contents.var.name), getReg(a3.contents.var.name));
            break;

        case opOR:

            instructionFormat1(or, getReg(a1.contents.var.name), getReg(a2.contents.var.name), getReg(a3.contents.var.name));
            break;

        case opASSIGN:

            instructionFormat2(move, getReg(a1.contents.var.name), getReg(a2.contents.var.name), 0, NULL);
            break;

        case opALLOC:

            if (a2.contents.val == 1)

                insertVar(a3.contents.var.name, a1.contents.var.name, a2.contents.val, simple);
            else
                insertVar(a3.contents.var.name, a1.contents.var.name, a2.contents.val, vector);

            break;

        case opIMMED:

            instructionFormat3(ldi, getReg(a1.contents.var.name), a2.contents.val, NULL);

            break;

        case opLOAD:

            aux = getVarMemLoc(a2.contents.var.name, a2.contents.var.scope);

            if (aux == -1)
            {
                v = getVarKind(a2.contents.var.name, "Global");
                aux = getVarMemLoc(a2.contents.var.name, "Global");
                if (v == vector)
                {
                    instructionFormat2(addi, getReg(a1.contents.var.name), $gp, aux, NULL);
                }
                else
                {
                    instructionFormat2(load, getReg(a1.contents.var.name), $gp, aux, NULL);
                }
            }
            else
            {
                v = getVarKind(a2.contents.var.name, a2.contents.var.scope);
                if (v == vector)
                {
                    instructionFormat2(addi, getReg(a1.contents.var.name), $sp, aux, NULL);
                }
                else
                {
                    instructionFormat2(load, getReg(a1.contents.var.name), $sp, aux, NULL);
                }
            }
            break;

        case opSTORE:
            aux = getVarMemLoc(a1.contents.var.name, a1.contents.var.scope);
            if (aux == -1)
            {
                aux = getVarMemLoc(a1.contents.var.name, "Global");
                if (a2.kind == String)
                    instructionFormat2(str, getReg(a3.contents.var.name), getReg(a2.contents.var.name), aux, NULL);
                else
                    instructionFormat2(str, getReg(a3.contents.var.name), $gp, aux, NULL);
            }
            else
            {
                if (a2.kind == String)
                    instructionFormat2(str, getReg(a3.contents.var.name), getReg(a2.contents.var.name), aux, NULL);
                else
                    instructionFormat2(str, getReg(a3.contents.var.name), $sp, aux, NULL);
            }
            break;

        case opVEC:
            v = getVarKind(a2.contents.var.name, a2.contents.var.scope); //a2.contents.var.scope);
            if (v == simple)
                v = getVarKind(a2.contents.var.name, "Global");
            aux = getVarMemLoc(a2.contents.var.name, a2.contents.var.scope); // a2.contents.var.scope);
            if (v == vector)
            {
                if (aux == -1)
                {
                    aux = getVarMemLoc(a2.contents.var.name, "Global");
                    instructionFormat1(add, getReg(a3.contents.var.name), getReg(a3.contents.var.name), $gp);
                }
                else
                {
                    instructionFormat1(add, getReg(a3.contents.var.name), getReg(a3.contents.var.name), $sp);
                }
                instructionFormat2(load, getReg(a1.contents.var.name), getReg(a3.contents.var.name), aux, NULL);
            }
            else
            {
                instructionFormat2(load, getReg(a1.contents.var.name), $sp, aux, NULL);
                instructionFormat1(add, getReg(a3.contents.var.name), getReg(a3.contents.var.name), getReg(a1.contents.var.name));
                instructionFormat2(load, getReg(a1.contents.var.name), getReg(a3.contents.var.name), 0, NULL);
            }

            break;

        case opGOTO:

            instructionFormat4(jmp, -1, a1.contents.var.name);
            break;

        case opIFF:

            instructionFormat2(beq, getReg(a1.contents.var.name), $zero, -1, a2.contents.var.name);
            break;

        case opRET:

            if (a1.kind == String)
                instructionFormat2(move, $ret, getReg(a1.contents.var.name), 0, NULL);
            instructionFormat2(addi, $ra, $ra, -1, NULL);
            instructionFormat2(load, $jmp, $ra, 0, NULL);
            instructionFormat3(jr, $jmp, 0, NULL);
            break;

        case opFUN:
            if (jmpmain == 0)
            {

                instructionFormat4(jmp, -1, "main");
                jmpmain = 1;
            }
            insertLabel(a1.contents.var.name);
            insertFun(a1.contents.var.name);
            curarg = 0;
            break;

        case opEND:
            if (strcmp(a1.contents.var.name, "main") == 0)
            {
                instructionFormat4(jmp, -1, "end");
            }
            else
            {
                instructionFormat2(addi, $ra, $ra, -1, NULL);
                instructionFormat2(load, $jmp, $ra, 0, NULL);
                instructionFormat3(jr, $jmp, 0, NULL);
            }
            break;

        case opPARAM:
            instructionFormat2(move, getParamReg(), getReg(a1.contents.var.name), 0, NULL);
            curparam++;
            break;

        case opCALL:

            if (strcmp(a2.contents.var.name, "input") == 0)
            {
                instructionFormat3(in, getReg(a1.contents.var.name), 0, NULL);
            }
            else if (strcmp(a2.contents.var.name, "output") == 0)
            {
                instructionFormat2(move, getReg(a1.contents.var.name), getArgReg(), 0, NULL);
                instructionFormat3(out, getReg(a1.contents.var.name), 0, NULL);
                instructionFormat4(nop, 0, NULL);
            }
            else
            {
                aux = getFunSize(a1.contents.var.scope);
                instructionFormat2(addi, $sp, $sp, aux, NULL);
                instructionFormat3(ldi, $jmp, line + 4, NULL);
                instructionFormat2(str, $jmp, $ra, 0, NULL);
                instructionFormat2(addi, $ra, $ra, 1, NULL);
                instructionFormat4(jmp, -1, a2.contents.var.name);
                instructionFormat2(move, getReg(a1.contents.var.name), $ret, 0, NULL);
                instructionFormat2(addi, $sp, $sp, -aux, NULL);
            }
            narg = a3.contents.val;
            curparam = 0;
            break;

        case opARG:

            insertVar(a3.contents.var.name, a1.contents.var.name, 1, checkType(l));

            instructionFormat2(str, getArgReg(), $sp, getVarMemLoc(a1.contents.var.name, a3.contents.var.name), NULL);
            curarg++;
            break;

        case opLAB:

            insertLabel(a1.contents.var.name);
            break;

        case opHLT:
            insertLabel("end");
            instructionFormat4(halt, 0, NULL);
            break;

        default:
            instructionFormat4(nop, 0, NULL);
            break;
        }

        l = l->next;
    }
}

void generateInstructions(QuadList head)
{
    QuadList l = head;
    generateInstruction(l);
    AssemblyCode a = codehead;
    while (a != NULL)
    {
        if (a->kind == instr)
        {
            if (a->line.instruction.opcode == jmp || a->line.instruction.opcode == beq || a->line.instruction.opcode == beq)
                a->line.instruction.im = getLabelLine(a->line.instruction.imlbl);
        }
        a = a->next;
    }
}

void printAssembly()
{
    AssemblyCode a = codehead;
    printf("\n\n----------CODIGO ASSEMBLY----------\n\n");
    while (a != NULL)
    {
        if (a->kind == instr)
        {
            if (a->line.instruction.format == format1)
            {
                printf("%d:\t%s %s, %s, %s\n", a->lineno, InstrNames[a->line.instruction.opcode], regNames[a->line.instruction.reg1],
                       regNames[a->line.instruction.reg2], regNames[a->line.instruction.reg3]);
            }
            else if (a->line.instruction.format == format2)
            {
                if (a->line.instruction.opcode == move)
                    printf("%d:\t%s %s, %s\n", a->lineno, InstrNames[a->line.instruction.opcode], regNames[a->line.instruction.reg1],
                           regNames[a->line.instruction.reg2]);
                else
                    printf("%d:\t%s %s, %s, %d\n", a->lineno, InstrNames[a->line.instruction.opcode], regNames[a->line.instruction.reg1],
                           regNames[a->line.instruction.reg2], a->line.instruction.im);
            }
            else if (a->line.instruction.format == format3)
            {
                if (a->line.instruction.opcode == jr || a->line.instruction.opcode == in || a->line.instruction.opcode == out)
                    printf("%d:\t%s %s\n", a->lineno, InstrNames[a->line.instruction.opcode], regNames[a->line.instruction.reg1]);
                else
                    printf("%d:\t%s %s, %d\n", a->lineno, InstrNames[a->line.instruction.opcode], regNames[a->line.instruction.reg1],
                           a->line.instruction.im);
            }
            else
            {
                if (a->line.instruction.opcode == halt || a->line.instruction.opcode == nop)
                    printf("%d:\t%s\n", a->lineno, InstrNames[a->line.instruction.opcode]);
                else
                    printf("%d:\t%s %d\n", a->lineno, InstrNames[a->line.instruction.opcode], a->line.instruction.im);
            }
        }
        else
        {
            printf(".%s\n", a->line.label);
        }
        a = a->next;
    }
}

void generateAssembly(QuadList head)
{
    initCode(head);
    generateInstructions(head);
    printAssembly();
}

AssemblyCode getAssembly()
{
    return codehead;
}

int getSize()
{
    return line - 1;
}
