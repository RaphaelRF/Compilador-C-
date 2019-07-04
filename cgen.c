#include "globals.h"
#include "symtab.h"
#include "cgen.h"

static int tmpOffset = 0;

static void cGen (TreeNode * tree);


QuadList head = NULL;

int location = 0;
int mainLocation;

int nlabel = 0;
int ntemp = 0;
int nparams = -1;

Address aux;
Address var;
Address offset;
Address empty;

const char * OpKindNames[] =  { "add", "sub", "mult", "div", "lt", "let", "gt", "get", "and", "or", "assign", "alloc", "immed", "load", "store",
                              "vec", "goto", "iff", "ret", "fun", "end", "param", "call", "arg", "lab", "hlt"  };


void quad_insert (OpKind op, Address addr1, Address addr2, Address addr3) {

  Quad quad;
  quad.op = op;
  quad.addr1 = addr1;
  quad.addr2 = addr2;
  quad.addr3 = addr3;
  QuadList new = (QuadList) malloc(sizeof(struct QuadListRec));
  new->location = location;
  new->quad = quad;
  new->next = NULL;
  if (head == NULL) {
    head = new;
  }
  else {
    QuadList q = head;
    while (q->next != NULL) q = q->next;
    q->next = new;
  }
  location ++;
}

int quad_update(int loc, Address addr1, Address addr2, Address addr3) {
  QuadList q = head;
  while (q != NULL) {
    if (q->location == loc) break;
    q = q->next;
  }
  if (q == NULL) 
    return 0;
  else {
    q->quad.addr1 = addr1;
    q->quad.addr2 = addr2;
    q->quad.addr3 = addr3;
    return 1;
  }
}

char * newLabel() {

  char * label = (char *) malloc((nlabel_size + 3) * sizeof(char));
  sprintf(label, "L%d", nlabel);
  nlabel++;
  return label;
}

char * newTemp() {

  char * temp = (char *) malloc((ntemp_size + 3) * sizeof(char));
  sprintf(temp, "$t%d", ntemp);
  ntemp = (ntemp + 1) % 16;
  return temp;
}

Address addr_createEmpty() {
  Address addr;
  addr.kind = Empty;
  addr.contents.var.name = NULL;
  addr.contents.var.scope = NULL;
  return addr;
}

Address addr_createIntConst(int val) {
  Address addr;
  addr.kind = IntConst;
  addr.contents.val = val;
  return addr;
}

Address addr_createString(char * name, char * scope) {
  Address addr;
  addr.kind = String;
  addr.contents.var.name = (char *) malloc(strlen(name) * sizeof(char));
  strcpy(addr.contents.var.name, name);
  addr.contents.var.scope = (char *) malloc(strlen(scope) * sizeof(char));
  strcpy(addr.contents.var.scope, scope);
  return addr;
}

static void genStmt( TreeNode * tree)
{ TreeNode * p1, * p2, * p3;
  Address addr1, addr2, addr3;
  Address aux1, aux2;
  int loc1, loc2, loc3;
  char * label;
  char * temp;
  
  switch (tree->kind.stmt) {

    case IfK:
      p1 = tree->child[0] ;
      p2 = tree->child[1] ;
      p3 = tree->child[2] ;
   
      cGen(p1);
      addr1 = aux;
   
      loc1 = location;
      quad_insert(opIFF, addr1, empty, empty);
      
      cGen(p2);
     
      loc2 = location;
      quad_insert(opGOTO, empty, empty, empty);

      label = newLabel();
      quad_insert(opLAB, addr_createString(label, tree->scope), empty, empty);
   
      quad_update(loc1, addr1, addr_createString(label, tree->scope), empty);
      
      cGen(p3);
      if (p3 != NULL) {
      
        loc3 = location;
        quad_insert(opGOTO, empty, empty, empty);
      }
      label = newLabel();
    
      quad_insert(opLAB, addr_createString(label, tree->scope), empty, empty);
      quad_update(loc2, addr_createString(label, tree->scope), empty, empty);
      if (p3 != NULL)
        quad_update(loc3, addr_createString(label, tree->scope), empty, empty);
      break;

    case WhileK:
      p1 = tree->child[0] ;
      p2 = tree->child[1] ;
      // inicio do while
      label = newLabel();
      quad_insert(opLAB, addr_createString(label, tree->scope), empty, empty);
      // condicao while
      cGen(p1);
      addr1 = aux;
      // se condição falsa?
      loc1 = location;
      quad_insert(opIFF, addr1, empty, empty);
      // while
      cGen(p2);
      loc3 = location;
      quad_insert(opGOTO, addr_createString(label, tree->scope), empty, empty);
      // final
      label = newLabel();
      quad_insert(opLAB, addr_createString(label, tree->scope), empty, empty);
      // se condição falsa vem pra cá
      quad_update(loc1, addr1, addr_createString(label, tree->scope), empty);
      break;

    case AssignK:
    
      p1 = tree->child[0];
      p2 = tree->child[1];
      
      cGen(p1);
      addr1 = aux;
      aux1 = var;
      aux2 = offset;
      
      cGen(p2);
      addr2 = aux;
    
      quad_insert(opASSIGN, addr1, addr2, empty);
      quad_insert(opSTORE, aux1, aux2, addr1);
   
      break;

    case ReturnK:
     
      p1 = tree->child[0];
      cGen(p1);
  
      if (p1 != NULL)
        addr1 = aux;

      else
        addr1 = empty;
      quad_insert(opRET, addr1, empty, empty);
     
      break;

    default:
      break;
  }
} 


static void genExp( TreeNode * tree)
{ TreeNode * p1, * p2, * p3;
  Address addr1, addr2, addr3;
  int loc1, loc2, loc3;
  char * label;
  char * temp;
  char * s = "";

  switch (tree->kind.exp) {
    
    case ConstK:
     
      addr1 = addr_createIntConst(tree->attr.val);
      temp = newTemp();
      aux = addr_createString(temp, s);
      quad_insert(opIMMED, aux, addr1, empty);
     
      break;
    
    case IdK:
   
      aux = addr_createString(tree->attr.name, tree->scope);
      p1 = tree->child[0];
      if (p1 != NULL) {
        temp = newTemp();
        addr1 = addr_createString(temp, tree->scope);
        addr2 = aux;
        cGen(p1);
        quad_insert(opVEC, addr1, addr2, aux);
        var = addr2;
        offset = aux;
        aux = addr1;
      }
      else {
        temp = newTemp();
        addr1 = addr_createString(temp, tree->scope);
        quad_insert(opLOAD, addr1, aux, empty);
        var = aux;
        offset = empty;
        aux = addr1;
      }
   
      break;

    case TypeK:
      break;

    case FunK:
     
    
      if (strcmp(tree->attr.name, "main") == 0)
        mainLocation = location;
      if ((strcmp(tree->attr.name, "input") != 0) && (strcmp(tree->attr.name, "output") != 0)) {
        quad_insert(opFUN, addr_createString(tree->attr.name, tree->scope), empty, empty);
        
        p1 = tree->child[0];
        cGen(p1);
    
        p2 = tree->child[1];
        cGen(p2);
        quad_insert(opEND, addr_createString(tree->attr.name, tree->scope), empty, empty);
      }
 
      break;
    
    case CallK:
     
      nparams = tree->params;
      p1 = tree->child[0];
  
      while (p1 != NULL) {
        cGen(p1);
        quad_insert(opPARAM, aux, empty, empty);

        nparams --;
        p1 = p1->sibling;
      }
      nparams = -1;
     
      temp = newTemp();
      aux = addr_createString(temp, tree->scope);
      quad_insert(opCALL, aux, addr_createString(tree->attr.name, tree->scope), addr_createIntConst(tree->params));
      
      break;
    
    case ParamK:
     
      quad_insert(opARG, addr_createString(tree->attr.name, tree->scope), empty, addr_createString(tree->scope, tree->scope));

    
      break;
    
    case VarK:
  
      if (tree->vet != -1)  
        quad_insert(opALLOC, addr_createString(tree->attr.name, tree->scope), addr_createIntConst(tree->vet), addr_createString(tree->scope, tree->scope));
      else {
        quad_insert(opALLOC, addr_createString(tree->attr.name, tree->scope), addr_createIntConst(1), addr_createString(tree->scope, tree->scope));
      }
      
      break;

    case OpK:
      
      p1 = tree->child[0];
      p2 = tree->child[1];
         
      cGen(p1);
      addr1 = aux;
      cGen(p2);
      addr2 = aux;
      temp = newTemp();
      aux = addr_createString(temp, tree->scope);
      switch (tree->attr.op) {
        case PLUS:
          quad_insert(opADD, aux, addr1, addr2);
          break;
        case MINUS:
          quad_insert(opSUB, aux, addr1, addr2);
          break;
        case TIMES:
          quad_insert(opMULT, aux, addr1, addr2);
          break;
        case OVER:
          quad_insert(opDIV, aux, addr1, addr2);
          break;
        case LT:
          quad_insert(opLT, aux, addr1, addr2);
          break;
        case LET:
          quad_insert(opLET, aux, addr1, addr2);
          break;
        case GT:
          quad_insert(opGT, aux, addr1, addr2);
          break;
        case GET:
          quad_insert(opGET, aux, addr1, addr2);
          break;
        case EQ:
          quad_insert(opGET, aux, addr1, addr2);
          addr3 = aux;
          temp = newTemp();
          aux = addr_createString(temp, tree->scope);
          quad_insert(opLET, aux, addr1, addr2);
          addr1 = addr3;
          addr2 = aux;
          temp = newTemp();
          aux = addr_createString(temp, tree->scope);
          quad_insert(opAND, aux, addr1, addr2);
          break;
        case NEQ:
          quad_insert(opGT, aux, addr1, addr2);
          addr3 = aux;
          temp = newTemp();
          aux = addr_createString(temp, tree->scope);
          quad_insert(opLT, aux, addr1, addr2);
          addr1 = addr3;
          addr2 = aux;
          temp = newTemp();
          aux = addr_createString(temp, tree->scope);
          quad_insert(opOR, aux, addr1, addr2);
          break;
        default:
       
          break;
      }
 
      break;

    default:
      break;
  }
} 

static void cGen( TreeNode * tree)
{ if (tree != NULL)
  { switch (tree->nodekind) {
      case StmtK:
        genStmt(tree);
        break;
      case ExpK:
        genExp(tree);
        break;
      default:
        break;
    }
    if (nparams == -1) {
      cGen(tree->sibling);
    }
    else {
      if(nparams == 0) {
        cGen(tree->sibling);
      }
    }
  }
}

void printCode() {
  QuadList q = head;
  Address a1, a2, a3;
  while (q != NULL) {
    a1 = q->quad.addr1;
    a2 = q->quad.addr2;
    a3 = q->quad.addr3;
    printf("%s, ", OpKindNames[q->quad.op]);
    switch (a1.kind) {
      case Empty:
        printf("-");
        break;
      case IntConst:
        printf("%d", a1.contents.val);
        break;
      case String:
       
        printf("%s", a1.contents.var.name);
        break;
      default:
      break;
    }
    printf(", ");
    switch (a2.kind) {
      case Empty:
        printf("-");
        break;
      case IntConst:
        printf("%d", a2.contents.val);
        break;
      case String:
      
        printf("%s", a2.contents.var.name);
        break;
      default:
        break;
    }
    printf(", ");
    switch (a3.kind) {
      case Empty:
        printf("-");
        break;
      case IntConst:
        printf("%d", a3.contents.val);
        break;
      case String:
      
        printf("%s", a3.contents.var.name);
        break;
      default:
        break;
    }
    printf("\n");
    q = q->next;
  }
}


void codeGen(TreeNode * syntaxTree, char * codefile)
{  char * s = malloc(strlen(codefile)+7);
  
   empty = addr_createEmpty();
   cGen(syntaxTree);
   quad_insert(opHLT, empty, empty, empty);
   printCode();
   printf("\n\n");
}

QuadList getIntermediate() {
    return head; 
}
  
