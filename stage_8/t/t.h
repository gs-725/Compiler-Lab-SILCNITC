#ifndef __T_H__
#define __T_H__

#include <stdio.h>
#include <string.h>


typedef enum Nodetype
{
    VARIABLE,
    ARRAY,
    CONSTANT,
    READ,
    WRITE,
    CONNECTOR,
    OPERATOR,
    WHILE,
    IF,
    BREAK,
    CONTINUE,
    BREAKPOINT,
    FUNCTION,
    BODY,
    RET,
    FUNCTIONCALL,
    NULL__,
    FIELD,
    FIELDFUNCTIONCALL,
    INITIALIZE,
    ALLOC,
    FREE,
    NEW,
    DELETE,
} Nodetype;

extern int inClass;

//-------------------Parameter List-------------------
struct ParamNode
{
    char *name;             // name of the variable
    struct TypeTable *type; // type of the variable
    struct ParamNode *next; // pointer to next symbol table entry
};

struct ParamNode *ParamInstall(struct ParamNode *, char *, struct TypeTable *);
struct ParamNode *ParamDelete(struct ParamNode *);
struct ParamNode *ParamCopy(struct ParamNode *);
void ParamPrint(struct ParamNode *);
int ParamGetCount(struct ParamNode *);
int ParamCheck(struct ParamNode *, struct ParamNode *);

//-------------------Type Table-------------------
struct TypeTable
{
    char *name;               // name of the type
    int size;                 // size of the type
    struct FieldList *fields; // pointer to the field list of structures
    struct TypeTable *next;   // pointer to next type table entry
};

void TTCreate();
struct TypeTable *TTLookup(char *);
struct TypeTable *TTInstall(char *, int, struct FieldList *);
struct FieldListNode *TTLookupField(struct TypeTable *, char *);
void TTPrint();

//-------------------Field List-------------------
struct FieldListNode
{
    char *name;
    int fieldIndex;
    struct TypeTable *type;
    struct ClassTable *class;
    struct FieldListNode *next;
};

struct FieldList
{
    struct FieldListNode *head, *tail;
    int size;
};

struct FieldList *FLInit();
struct FieldListNode *FLInitNode(char *, int, struct TypeTable *, struct ClassTable *);
struct FieldListNode *FLLookup(struct FieldList *, char *);
struct FieldList *FLInstall(struct FieldList *, char *, struct TypeTable *, struct ClassTable *);
void FLPrint(struct FieldList *);

//-------------------Class Table-------------------
struct ClassTable
{
    char *name;                  // name of the class
    int classIndex;              // position of the class in the virtual function table
    int fieldCount, methodCount; // number of fields and methods
    struct FieldList *fields;    // pointer to FieldList
    struct MethodList *methods;  // pointer to Methodlist
    struct ClassTable *parent;   // pointer to the parent's class table
    struct ClassTable *next;     // pointer to next class table entry
};

struct ClassTable *CInstall(char *, char *);
struct ClassTable *CLookup(char *);
void CInstallField(struct ClassTable *, char *, struct TypeTable *);
void CInstallMethod(struct ClassTable *, char *, struct TypeTable *, struct ParamNode *);
struct FieldListNode *CLookupField(struct ClassTable *, char *);
struct MethodListNode *CLookupMethod(struct ClassTable *, char *);
int CInheritanceCheck(struct ClassTable *, struct ClassTable *);
void Ccopyfields(struct ClassTable *);
void Ccopymethods(struct ClassTable *);
void CPrint();

//-------------------Method List-------------------
struct MethodListNode
{
    char *name;                  // name of the member function in the class
    struct TypeTable *type;      // pointer to typetable
    struct ParamNode *Phead;     // pointer to the head of the formal parameter list
    struct LSTable *LST;         // pointer to the local symbol table
    int methodIndex;             // position of the function in the class table
    int mlabel;                  // A label for identifying the starting address of the function's code in the memory
    int overridden;              // 1 if the function is overridden, 0 otherwise
    int inherited;               // 1 if the function is inherited, 0 otherwise
    struct MethodListNode *next; // pointer to next MethodList entry
};

struct MethodList
{
    struct MethodListNode *head, *tail;
    int size;
};

struct MethodList *MLInit();
struct MethodListNode *MLInitNode(char *, int, int, int, int, struct TypeTable *, struct ParamNode *);
struct MethodList *MLInstall(struct MethodList *, char *, struct TypeTable *, struct ParamNode *);
void MLPrint(struct MethodList *);

//-------------------AST-------------------
struct AST_Node
{
    int val;                             // value of a number for NUM nodes.
    struct TypeTable *type;              // type of variable
    struct ClassTable *class;            // class of variable
    char *name;                          // name of a variable for ID nodes
    Nodetype nodetype;                   // type of node
    char *s;                             // string representation of the node
    struct GST_Node *GSTentry;           // Pointer to Global Symbol Table entry for variables
    struct AST_Node *next_arg;           // Pointer to next argument in the argument list
    struct AST_Node *arg_list;           // Pointer to the argument list
    struct AST_Node *left, *mid, *right; // left, middle and right branches
};

struct AST_Node *makeVariableLeafNode(char *, char *);
struct AST_Node *makeConstantLeafNode(struct TypeTable *, int, char *);
struct AST_Node *makeArrayLeafNode(char *, struct AST_Node *, char *);
struct AST_Node *makeNode(Nodetype, struct TypeTable *, struct ClassTable *, struct AST_Node *, struct AST_Node *, struct AST_Node *, struct GST_Node *, char *);
void ASTPrint(struct AST_Node *);

//-------------------Global Symbol Table-------------------
struct GST_Node
{
    char *name;               // name of the variable
    struct TypeTable *type;   // type of the variable
    struct ClassTable *class; // class of the variable
    int size;                 // size of the type
    Nodetype typeofvar;       // variable, array or function
    int binding;              // stores the static memory address allocated to the variable
    struct ParamNode *Phead;  // stores the parameter list for functions
    struct LSTable *LST;      // stores the local symbol table for functions
    int flabel;               // a label for identifying the starting address of a function's code
    struct GST_Node *next;    // pointer to next symbol table entry
};

struct GST_Node *GSTLookup(char *);
struct GST_Node *GSTInstall(char *, struct TypeTable *, int, Nodetype, struct ParamNode *, struct ClassTable *);
void GSTChangeTypeClass(struct AST_Node *, struct TypeTable *, struct ClassTable *, int);
void GSTPrint();

//-------------------Local Symbol Table-------------------
struct LST_Node
{
    char *name;               // name of the variable
    struct TypeTable *type;   // type of the variable
    struct ClassTable *class; // class of the variable
    int binding;              // stores the static memory address allocated to the variable
    struct LST_Node *next;    // pointer to next symbol table entry
};

struct LSTable
{
    struct LST_Node *head;
    struct LST_Node *tail;
    int size;
};

struct LST_Node *LSTInitNode(char *, struct TypeTable *, struct ClassTable *, int);
struct LSTable *LSTInitTable();
struct LST_Node *LSTLookup(struct LSTable *, char *);
struct LSTable *LSTInstall(struct LSTable *, char *, struct TypeTable *, struct ClassTable *);
struct LSTable *LSTDelete(struct LSTable *);
struct LSTable *LSTCopy(struct LSTable *);
void LSTChangeType(struct LSTable *, struct AST_Node *, struct TypeTable *);
void LSTPrint(struct LSTable *);

//-------------------CodeGen Stack-------------------
struct Stack
{
    struct AST_Node *node;
    struct LSTable *LST;
    struct ClassTable *class;
    struct Stack *next;
};

void pushStack(struct AST_Node *, struct LSTable *, struct ClassTable *);
struct Stack *popStack();
struct Stack *topStack();
void printStack();

//-------------------Argument List-------------------
struct AST_Node *ASTArgAppend(struct AST_Node *, struct AST_Node *);
int checkASTParam(struct ParamNode *, struct AST_Node *);
struct AST_Node *ParamToArg(struct ParamNode *);
struct ParamNode *ArgToParam(struct AST_Node *);
void ArgDelete(struct AST_Node *);

//-------------------Code Generation-------------------
int codeGen(struct AST_Node *, struct LSTable *, struct ClassTable *, FILE *);

//-------------------Auxiliary Functions-------------------
int getAddr(struct AST_Node *, struct LSTable *, struct ClassTable *, FILE *);
int getArrayAddr(struct AST_Node *, struct LSTable *, struct ClassTable *, FILE *);
int getFieldAddr(struct AST_Node *, struct LSTable *, struct ClassTable *, FILE *);

int getVirtualFuncTablePointerAddr(struct AST_Node *, struct LSTable *, struct ClassTable *, FILE *);
int getVirtualFuncTablePointer(struct AST_Node *, struct LSTable *, struct ClassTable *, FILE *);
int getClassVirtualFuncTablePointer(struct ClassTable *, FILE *);

int pushArgs(struct AST_Node *, int, struct LSTable *, struct ClassTable *, FILE *);
struct LSTable *LSTParamInstall(struct LSTable *, struct ParamNode *);
void generateHeader(FILE *);

struct AST_Node *ASTChangeTypeClassGST(struct AST_Node *, struct TypeTable *, struct ClassTable *, int);
struct AST_Node *ASTChangeTypeLST(struct LSTable *, struct AST_Node *, struct TypeTable *);

//-------------------Type Checking-------------------
int typeCheckInt(struct AST_Node *);
int typeCheckBool(struct AST_Node *);
int typeCheckStr(struct AST_Node *);
int typeCheckVoid(struct AST_Node *);

//-------------------Static Storage Allocation-------------------
int allocate(int);
int getSP();

//-------------------Register Allocation-------------------
int getReg();
void freeReg();

//-------------------Label Allocation-------------------
int getLabel();
int getFlabel(char name);
int setFlabel();

#endif
